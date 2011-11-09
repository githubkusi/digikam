/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-11-07
 * Description : Directory watch interface
 *
 * Copyright (C) 2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "albumwatch.moc"

// Qt includes

#include <QDateTime>
#include <QDBusConnection>
#include <QDir>
#include <QFileInfo>

// KDE includes

#include <kdebug.h>
#include <kdirwatch.h>

// Local includes

#include "album.h"
#include "albummanager.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "databaseparameters.h"
#include "kinotify.h"
#include "scancontroller.h"

namespace Digikam
{

enum Mode
{
    InotifyMode,
    KDirWatchMode
};

class AlbumWatch::AlbumWatchPriv
{
public:

    AlbumWatchPriv(AlbumWatch* q)
        : inotify(0),
          dirWatch(0),
          connectedToKIO(false),
          q(q)
          
    {
    }

    void determineMode();
    bool isInotifyMode() const { return mode == InotifyMode; }

    Mode               mode;

    KInotify*          inotify;
    KDirWatch*         dirWatch;
    QStringList        dirWatchAddedDirs;
    bool               connectedToKIO;

    DatabaseParameters params;
    QStringList        fileNameBlackList;
    QList<QDateTime>   dbPathModificationDateList;

    AlbumWatch* const  q;

    bool inBlackList(const QString& path);
    QList<QDateTime> buildDirectoryModList(const QFileInfo& dbFile);
    bool inDirWatchParametersBlackList(const QFileInfo& info, const QString& path);
};

void AlbumWatch::AlbumWatchPriv::determineMode()
{
    if (KInotify::available())
    {
        mode = InotifyMode;
    }
    else
    {
        mode = KDirWatchMode;
    }
}

bool AlbumWatch::AlbumWatchPriv::inBlackList(const QString& path)
{
    // Filter out dirty signals triggered by changes on the database file
    foreach (const QString& bannedFile, fileNameBlackList)
    {
        if (path.endsWith(bannedFile))
        {
            return true;
        }
    }
    return false;
}

// --------- //

AlbumWatch::AlbumWatch(AlbumManager* parent)
    : QObject(parent),
      d(new AlbumWatchPriv(this))
{
    d->determineMode();

    if (d->isInotifyMode())
    {
        connectToKInotify();
    }
    else
    {
        connectToKDirWatch();
        connectToKIO();
    }

    connect(parent, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));
    connect(parent, SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));
}

AlbumWatch::~AlbumWatch()
{
    delete d;
}

void AlbumWatch::clear()
{
    if (d->dirWatch)
    {
        foreach (const QString& addedDirectory, d->dirWatchAddedDirs)
        {
            d->dirWatch->removeDir(addedDirectory);
        }
        d->dirWatchAddedDirs.clear();
    }

    if (d->connectedToKIO)
    {
        QDBusConnection::sessionBus().disconnect(QString(), QString(), "org.kde.KDirNotify", "FileMoved", 0, 0);
        QDBusConnection::sessionBus().disconnect(QString(), QString(), "org.kde.KDirNotify", "FilesAdded", 0, 0);
        QDBusConnection::sessionBus().disconnect(QString(), QString(), "org.kde.KDirNotify", "FilesRemoved", 0, 0);

        d->connectedToKIO = false;
    }

    if (d->inotify)
    {
        d->inotify->removeAllWatches();
    }
}

void AlbumWatch::setDatabaseParameters(const DatabaseParameters& params)
{
    d->params = params;

    d->fileNameBlackList.clear();
    // filter out notifications caused by database operations
    if (params.isSQLite())
    {
        d->fileNameBlackList << "thumbnails-digikam.db" << "thumbnails-digikam.db-journal";

        QFileInfo dbFile(params.SQLiteDatabaseFile());
        d->fileNameBlackList << dbFile.fileName() << dbFile.fileName() + "-journal";

        // ensure this is done after setting up the black list
        d->dbPathModificationDateList = d->buildDirectoryModList(dbFile);
    }
}

void AlbumWatch::slotAlbumAdded(Album* a)
{
    if (a->isRoot() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* album = static_cast<PAlbum*>(a);

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(album->albumRootId());
    if (!location.isAvailable())
    {
        return;
    }
    QString dir = album->folderPath();
    if (dir.isEmpty())
    {
        return;
    }

    if (d->isInotifyMode())
    {
        d->inotify->watchDirectory(dir);
    }
    else
    {
        if (!d->dirWatch->contains(dir))
        {
            d->dirWatchAddedDirs << dir;
            d->dirWatch->addDir(dir, KDirWatch::WatchFiles | KDirWatch::WatchDirOnly);
        }
    }
}

void AlbumWatch::slotAlbumAboutToBeDeleted(Album* a)
{
    if (a->isRoot() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* album = static_cast<PAlbum*>(a);
    QString dir = album->folderPath();
    if (dir.isEmpty())
    {
        return;
    }

    if (d->isInotifyMode())
    {
        d->inotify->removeDirectory(dir);
    }
    else
    {
        d->dirWatch->removeDir(album->folderPath());
    }
}

void AlbumWatch::rescanDirectory(const QString& dir)
{
    kDebug() << "Detected change, triggering rescan of directory" << dir;
    ScanController::instance()->scheduleCollectionScanRelaxed(dir);
}

/* ---------- KInotify ---------- */

void AlbumWatch::connectToKInotify()
{
    if (d->inotify)
    {
        return;
    }

    d->inotify = new KInotify(this);

    connect( d->inotify, SIGNAL( moved( QString, QString ) ),
             this, SLOT( slotFileMoved( QString, QString ) ) );
    connect( d->inotify, SIGNAL( deleted( QString, bool ) ),
             this, SLOT( slotFileDeleted( QString, bool ) ) );
    connect( d->inotify, SIGNAL( created( QString, bool ) ),
             this, SLOT( slotFileCreated( QString, bool ) ) );
    connect( d->inotify, SIGNAL( closedWrite( QString ) ),
             this, SLOT( slotFileClosedAfterWrite( QString ) ) );
    connect( d->inotify, SIGNAL( watchUserLimitReached() ),
             this, SLOT( slotInotifyWatchUserLimitReached() ) );
}

void AlbumWatch::slotFileMoved(const QString& from, const QString& to)
{
    // we could add a copyOrMoveHint here...but identical-file detection seems to work well
    rescanPath(from);
    rescanPath(to);
}

void AlbumWatch::slotFileDeleted(const QString& path, bool isDir)
{
    Q_UNUSED(isDir);
    rescanPath(path);
}

void AlbumWatch::slotFileCreated(const QString& path, bool isDir)
{
    if (isDir)
    {
        rescanPath(path);
    }
    // for files, rely on ClosedAfterWrite only,
    // which always comes after create if the operation has finished
}

void AlbumWatch::slotFileClosedAfterWrite(const QString& path)
{
    rescanPath(path);
}

void AlbumWatch::slotInotifyWatchUserLimitReached()
{
    kError() << "Reached inotify limit";
}

void AlbumWatch::rescanPath(const QString& path)
{
    if (d->inBlackList(path))
    {
        return;
    }
    KUrl url(path);
    rescanDirectory(url.directory());
}

/* ---------- KDirWatch ---------- */

QList<QDateTime> AlbumWatch::AlbumWatchPriv::buildDirectoryModList(const QFileInfo& dbFile)
{
    // retrieve modification dates
    QList<QDateTime> modList;
    QFileInfoList    fileInfoList = dbFile.dir().entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    // build list
    foreach (const QFileInfo& info, fileInfoList)
    {
        // ignore digikam4.db and journal and other temporary files
        if (!fileNameBlackList.contains(info.fileName()))
        {
            modList << info.lastModified();
        }
    }
    return modList;
}

bool AlbumWatch::AlbumWatchPriv::inDirWatchParametersBlackList(const QFileInfo& info, const QString& path)
{
    if (params.isSQLite())
    {
        QDir dir;

        if (info.isDir())
        {
            dir = QDir(path);
        }
        else
        {
            dir = info.dir();
        }

        QFileInfo dbFile(params.SQLiteDatabaseFile());

        // Workaround for broken KDirWatch in KDE 4.2.4
        if (path.startsWith(dbFile.filePath()))
        {
            return true;
        }

        // is the signal for the directory containing the database file?
        if (dbFile.dir() == dir)
        {
            // retrieve modification dates
            QList<QDateTime> modList = buildDirectoryModList(dbFile);

            // check for equality
            if (modList == dbPathModificationDateList)
            {
                //kDebug() << "Filtering out db-file-triggered dir watch signal";
                // we can skip the signal
                return true;
            }

            // set new list
            dbPathModificationDateList = modList;
        }
    }

    return false;
}

void AlbumWatch::slotDirWatchDirty(const QString& path)
{
    if (d->inBlackList(path))
    {
        return;
    }

    QFileInfo info(path);
    if (d->inDirWatchParametersBlackList(info, path))
    {
        return;
    }

    kDebug() << "KDirWatch detected change at" << path;

    if (info.isDir())
    {
        rescanDirectory(path);
    }
    else
    {
        rescanDirectory(info.path());
    }
}

void AlbumWatch::connectToKDirWatch()
{
    if (d->dirWatch)
    {
        return;
    }

    KDirWatch::Method m = d->dirWatch->internalMethod();
    QString           mName("FAM");

    if (m == KDirWatch::DNotify)
    {
        mName = QString("DNotify");
    }
    else if (m == KDirWatch::Stat)
    {
        mName = QString("Stat");
    }
    else if (m == KDirWatch::INotify)
    {
        mName = QString("INotify");
    }

    kDebug() << "KDirWatch method = " << mName;

    d->dirWatch = new KDirWatch(this);

    connect(d->dirWatch, SIGNAL(dirty(QString)),
            this, SLOT(slotDirWatchDirty(QString)));
}


/* ---------- KIO ---------- */

void AlbumWatch::connectToKIO()
{
    if (d->connectedToKIO)
    {
        return;
    }

    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KDirNotify", "FileMoved",
                                          this, SLOT(slotKioFileMoved(QString,QString)));
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KDirNotify", "FilesAdded",
                                          this, SLOT(slotKioFilesAdded(QString)));
    QDBusConnection::sessionBus().connect(QString(), QString(), "org.kde.KDirNotify", "FilesRemoved",
                                          this, SLOT(slotKioFilesDeleted(QStringList)));

    d->connectedToKIO = true;
}

void AlbumWatch::slotKioFileMoved(const QString& urlFrom, const QString& urlTo)
{
    kDebug() << urlFrom << urlTo;
    handleKioNotification(KUrl(urlFrom));
    handleKioNotification(KUrl(urlTo));
}

void AlbumWatch::slotKioFilesDeleted(const QStringList& urls)
{
    kDebug() << urls;
    foreach (const QString& url, urls)
    {
        handleKioNotification(KUrl(url));
    }
}

void AlbumWatch::slotKioFilesAdded(const QString& url)
{
    kDebug() << url;
    handleKioNotification(KUrl(url));
}

void AlbumWatch::handleKioNotification(const KUrl& url)
{
    if (url.isLocalFile())
    {
        QString path = url.directory();

        //kDebug() << path << !CollectionManager::instance()->albumRootPath(path).isEmpty();
        // check path is in our collection
        if (CollectionManager::instance()->albumRootPath(path).isNull())
        {
            return;
        }

        kDebug() << "KDirNotify detected file change at" << path;

        rescanDirectory(path);
    }
    else
    {
        DatabaseUrl dbUrl(url);

        if (dbUrl.isAlbumUrl())
        {
            QString path = dbUrl.fileUrl().directory();
            kDebug() << "KDirNotify detected file change at" << path;
            rescanDirectory(path);
        }
    }
}

}
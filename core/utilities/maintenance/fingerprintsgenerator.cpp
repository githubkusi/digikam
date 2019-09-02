/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : fingerprints generator
 *
 * Copyright (C)      2018 by Mario Frank    <mario dot frank at uni minus potsdam dot de>
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "fingerprintsgenerator.h"

// Qt includes

#include <QApplication>
#include <QString>
#include <QIcon>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "dimg.h"
#include "coredb.h"
#include "albummanager.h"
#include "coredbaccess.h"
#include "maintenancethread.h"

namespace Digikam
{

class Q_DECL_HIDDEN FingerPrintsGenerator::Private
{
public:

    explicit Private()
      : rebuildAll(true),
        thread(nullptr)
    {
    }

    bool               rebuildAll;

    AlbumList          albumList;

    MaintenanceThread* thread;
};

FingerPrintsGenerator::FingerPrintsGenerator(const bool rebuildAll, const AlbumList& list, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("FingerPrintsGenerator"), parent),
      d(new Private)
{
    setLabel(i18n("Finger-prints"));
    ProgressManager::addProgressItem(this);

    d->albumList  = list;
    d->rebuildAll = rebuildAll;
    d->thread     = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvance(QImage)),
            this, SLOT(slotAdvance(QImage)));
}

FingerPrintsGenerator::~FingerPrintsGenerator()
{
    delete d;
}

void FingerPrintsGenerator::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void FingerPrintsGenerator::slotCancel()
{
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

void FingerPrintsGenerator::slotStart()
{
    MaintenanceTool::slotStart();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (d->albumList.isEmpty())
    {
        d->albumList = AlbumManager::instance()->allPAlbums();
    }

    // Get all item IDs from albums.

    QList<qlonglong> itemIds;

    for (AlbumList::ConstIterator it = d->albumList.constBegin() ;
         !canceled() && (it != d->albumList.constEnd()) ; ++it)
    {
        if ((*it)->type() == Album::PHYSICAL)
        {
            itemIds << CoreDbAccess().db()->getItemIDsInAlbum((*it)->id());
        }
        else if ((*it)->type() == Album::TAG)
        {
            itemIds << CoreDbAccess().db()->getItemIDsInTag((*it)->id());
        }
    }

    QApplication::restoreOverrideCursor();

    if (itemIds.isEmpty())
    {
        slotDone();
        return;
    }

    setTotalItems(itemIds.count());

    d->thread->generateFingerprints(itemIds, d->rebuildAll);
    d->thread->start();
}

void FingerPrintsGenerator::slotAdvance(const QImage& img)
{
    setThumbnail(QIcon(QPixmap::fromImage(img)));
    advance(1);
}

void FingerPrintsGenerator::slotDone()
{
    // Switch on scanned for finger-prints flag on digiKam config file.
    KSharedConfig::openConfig()->group(QLatin1String("General Settings")).writeEntry(QLatin1String("Finger Prints Generator First Run"), true);

    MaintenanceTool::slotDone();
}

} // namespace Digikam

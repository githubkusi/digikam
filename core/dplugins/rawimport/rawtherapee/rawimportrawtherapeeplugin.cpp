/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : RawTherapee raw import plugin.
 *
 * Copyright (C) 2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rawimportrawtherapeeplugin.h"

// Qt includes

#include <QMessageBox>
#include <QApplication>
#include <QPointer>
#include <QDebug>
#include <QByteArray>
#include <QFileInfo>
#include <QTemporaryFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dimg.h"
#include "loadingdescription.h"

namespace DigikamRawImportRawTherapeePlugin
{

class Q_DECL_HIDDEN RawTherapeeRawImportPlugin::Private
{
public:

    explicit Private()
      : rawtherapee(nullptr),
        tempFile(nullptr)
    {
    }

    QProcess*          rawtherapee;
    DImg               decoded;
    LoadingDescription props;
    QFileInfo          fileInfo;
    QTemporaryFile*    tempFile;
};

RawTherapeeRawImportPlugin::RawTherapeeRawImportPlugin(QObject* const parent)
    : DPluginRawImport(parent),
      d(new Private)
{
}

RawTherapeeRawImportPlugin::~RawTherapeeRawImportPlugin()
{
    delete d;
}

QString RawTherapeeRawImportPlugin::name() const
{
    return QString::fromUtf8("Raw Import using RawTherapee");
}

QString RawTherapeeRawImportPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon RawTherapeeRawImportPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("image-x-adobe-dng"));
}

QString RawTherapeeRawImportPlugin::description() const
{
    return QString::fromUtf8("A RAW import plugin based on RawTherapee");
}

QString RawTherapeeRawImportPlugin::details() const
{
    return QString::fromUtf8("<p>This RAW Import plugin use RawTherapee tool to pre-process file in Image Editor.</p>"
                             "<p>It requires at least RawTherapee version 5.2.0 to work.</p>");
}

QList<DPluginAuthor> RawTherapeeRawImportPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2019"))
            ;
}

void RawTherapeeRawImportPlugin::setup(QObject* const /*parent*/)
{
    // Nothing to do
}

bool RawTherapeeRawImportPlugin::run(const QString& filePath, const DRawDecoding& /*def*/)
{
    d->fileInfo = QFileInfo(filePath);
    d->props    = LoadingDescription(d->fileInfo.filePath(), LoadingDescription::ConvertForEditor);
    d->decoded  = DImg();

    delete d->tempFile;

    d->tempFile = new QTemporaryFile();
    d->tempFile->open();

    d->rawtherapee = new QProcess(this);
    d->rawtherapee->setProcessChannelMode(QProcess::MergedChannels);
    d->rawtherapee->setWorkingDirectory(d->fileInfo.path());

    connect(d->rawtherapee, SIGNAL(errorOccurred(QProcess::ProcessError)),
            this, SLOT(slotErrorOccurred(QProcess::ProcessError)));

    connect(d->rawtherapee, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(slotProcessFinished(int,QProcess::ExitStatus)));

    connect(d->rawtherapee, SIGNAL(readyRead()),
            this, SLOT(slotProcessReadyRead()));

    // --------

    d->fileInfo = QFileInfo(filePath);

    d->rawtherapee->setProgram(QLatin1String("rawtherapee"));
    d->rawtherapee->setArguments(QStringList() << QLatin1String("-gimp")   // Special mode used initialy as Gimp plugin
                                              << filePath                 // Input file
                                              << d->tempFile->fileName()); // Output file

    qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee arguments:" << d->rawtherapee->arguments();

    d->rawtherapee->start();

    return true;
}

void RawTherapeeRawImportPlugin::slotErrorOccurred(QProcess::ProcessError error)
{
    switch (error)
    {
        case QProcess::FailedToStart:
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process has failed to start";
            break;
        case QProcess::Crashed:
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process has crashed";
            break;
        case QProcess::Timedout:
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process time-out";
            break;
        case QProcess::WriteError:
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process write error";
            break;
        case QProcess::ReadError:
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process read error";
            break;
        default:
            qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: Process error unknown";
            break;
    }
}

void RawTherapeeRawImportPlugin::slotProcessFinished(int code, QProcess::ExitStatus status)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee :: return code:" << code << ":: Exit status:" << status;

    d->decoded = DImg(d->tempFile->fileName());

    if (d->decoded.isNull())
    {
        QString message = i18n("Error to import RAW image with RawTherapee\nClose this dialog to load RAW image with native import tool");
        QMessageBox::information(0, qApp->applicationName(), message);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Decoded image is null! Load with Native tool...";
        qCDebug(DIGIKAM_GENERAL_LOG) << d->props.filePath;
        emit signalLoadRaw(d->props);
    }
    else
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Decoded image is not null...";
        qCDebug(DIGIKAM_GENERAL_LOG) << d->props.filePath;
        d->props = LoadingDescription(d->tempFile->fileName(), LoadingDescription::ConvertForEditor);
        emit signalDecodedImage(d->props, d->decoded);
    }

    delete d->tempFile;
    d->tempFile = nullptr;
}

void RawTherapeeRawImportPlugin::slotProcessReadyRead()
{
    QByteArray data   = d->rawtherapee->readAllStandardError();
    QStringList lines = QString::fromUtf8(data).split(QLatin1Char('\n'), QString::SkipEmptyParts);

    foreach (const QString& one, lines)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "RawTherapee ::" << one;
    }
}

} // namespace DigikamRawImportRawTherapeePlugin

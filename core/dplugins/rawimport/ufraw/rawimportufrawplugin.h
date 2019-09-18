/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-18
 * Description : UFRaw raw import plugin.
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

#ifndef DIGIKAM_UFRAW_RAW_IMPORT_PLUGIN_H
#define DIGIKAM_UFRAW_RAW_IMPORT_PLUGIN_H

// Qt includes

#include <QProcess>
#include <QFileInfo>
#include <QTemporaryFile>

// Local includes

#include "dpluginrawimport.h"
#include "dimg.h"
#include "loadingdescription.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.rawimport.UFRaw"

using namespace Digikam;

namespace DigikamRawImportUFRawPlugin
{

class UFRawRawImportPlugin : public DPluginRawImport
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginRawImport)

public:

    explicit UFRawRawImportPlugin(QObject* const parent = nullptr);
    ~UFRawRawImportPlugin();

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;

    void setup(QObject* const)           override;

    bool run(const QString& filePath, const DRawDecoding& def);

private Q_SLOTS:

    void slotErrorOccurred(QProcess::ProcessError);
    void slotProcessFinished(int, QProcess::ExitStatus);
    void slotProcessReadyRead();

private:

    QProcess*          m_ufraw;
    DImg               m_decoded;
    LoadingDescription m_props;
    QFileInfo          m_fileInfo;
    QTemporaryFile*    m_tempFile;
};

} // namespace DigikamRawImportUFRawPlugin

#endif // DIGIKAM_UFRAW_RAW_IMPORT_PLUGIN_H

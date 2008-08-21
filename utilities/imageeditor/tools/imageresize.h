/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2005-04-07
 * Description : a tool to resize an image
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGE_RESIZE_H
#define IMAGE_RESIZE_H

// Qt includes.

#include <QString>
#include <QCloseEvent>
#include <QCustomEvent>

// KDE includes.

#include <kdialog.h>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class ImageResizePriv;

class DIGIKAM_EXPORT ImageResize : public KDialog
{
    Q_OBJECT

public:

    ImageResize(QWidget* parent);
    ~ImageResize();

protected:

    void closeEvent(QCloseEvent *e);

private:

    void writeUserSettings();

private slots:

    void slotOk();
    void slotCancel();
    void slotDefault();
    void slotUser2();
    void slotUser3();
    void processCImgUrl(const QString&);
    void slotValuesChanged();
    void readUserSettings();
    void slotRestorationToggled(bool);

    virtual void slotButtonClicked(int);

    void slotFilterStarted();
    void slotFilterFinished(bool);
    void slotFilterProgress(int);

private:

    ImageResizePriv *d;
};

}  // NameSpace Digikam

#endif /* IMAGE_RESIZE_H */

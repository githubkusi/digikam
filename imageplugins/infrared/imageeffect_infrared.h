/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-22
 * Description : a digiKam image editor plugin for simulate 
 *               infrared film.
 * 
 * Copyright (C) 2005-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEEFFECT_INFRARED_H
#define IMAGEEFFECT_INFRARED_H

// Digikam includes.

#include "ctrlpaneldlg.h"

class QSlider;
class QLCDNumber;
class QCheckBox;

namespace DigikamInfraredImagesPlugin
{

class ImageEffect_Infrared : public Digikam::CtrlPanelDlg
{
    Q_OBJECT

public:

    ImageEffect_Infrared(QWidget* parent);
    ~ImageEffect_Infrared();

private slots:

    void slotSliderMoved(int);
    void readUserSettings();

private:

    void writeUserSettings();
    void resetValues();    
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void renderingFinished();

private:

    QCheckBox  *m_addFilmGrain; 

    QSlider    *m_sensibilitySlider;

    QLCDNumber *m_sensibilityLCDValue;
};

}  // NameSpace DigikamInfraredImagesPlugin

#endif /* IMAGEEFFECT_INFRARED_H */

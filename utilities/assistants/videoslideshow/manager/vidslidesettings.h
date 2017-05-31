/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef VIDSLIDE_SETTINGS_H
#define VIDSLIDE_SETTINGS_H

// Qt includes

#include <QString>
#include <QList>
#include <QUrl>
#include <QSize>
#include <QMap>

// Local includes

#include "dinfointerface.h"
#include "transitionmngr.h"
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT VidSlideSettings
{
public:

    enum Selection
    {
        IMAGES = 0,
        ALBUMS
    };

    // See https://en.wikipedia.org/wiki/List_of_common_resolutions#Digital_TV_standards
    enum VidType
    {
        RIM240 = 0,     // 240  x 136
        QVGA,           // 320  x 180
        VCD,            // 352  x 240
        HVGA,           // 480  x 270
        SVCD,           // 480  x 576
        VGA,            // 640  x 360
        DVD,            // 720  x 576
        WVGA,           // 800  x 450
        XVGA,           // 1024 x 576
        HDTV,           // 1280 x 720
        BLUERAY,        // 1920 x 1080
        UHD4K           // 3840 x 2160
    };

    // Video rate in bits per seconds.
    enum VidBitRate
    {
        VBR04 = 0,  // 400000
        VBR05,      // 500000,
        VBR10,      // 1000000,
        VBR12,      // 1200000,
        VBR15,      // 1500000
        VBR20,      // 2000000
        VBR25,      // 2500000
        VBR30,      // 3000000
        VBR40,      // 4000000
        VBR45,      // 4500000
        VBR50,      // 5000000
        VBR60,      // 6000000
        VBR80       // 8000000
    };

public:

    explicit VidSlideSettings();
    ~VidSlideSettings();

    void  readSettings(KConfigGroup& group);
    void  writeSettings(KConfigGroup& group);

    QSize videoTypeSize() const;
    int   videoBitRate()  const;

    static QMap<VidType,    QString> videoTypeNames();
    static QMap<VidBitRate, QString> videoBitRateNames();

public:

    Selection                 selMode;       // Items selection mode

    QList<QUrl>               inputImages;   // Selection::IMAGES
    DInfoInterface::DAlbumIDs inputAlbums;   // Selection::ALBUMS

    QList<QUrl>               inputAudio;    // Soundtracks streams.

    TransitionMngr::TransType transition;    // Transition type between images.

    int                       aframes;       // Amount of frames to encode by image in video stream.
                                             // ex: 125 frames = 5 s at 25 img/s.

    int                       abitRate;      // Encoded Audio stream bit rate in bit/s.
    VidBitRate                vbitRate;      // Encoded Video stream bit rate in bit/s.
    qreal                     frameRate;     // Encoded Video stream frame rate in img/s.
    VidType                   outputType;    // Encoded video type.
    QUrl                      outputVideo;   // Encoded video stream.

    bool                      openInPlayer;  // Open video stream in desktop player at end.
};

} // namespace Digikam

#endif // VIDSLIDE_SETTINGS_H
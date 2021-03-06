/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-04-19
 * Description : ImageMagick loader for DImg framework.
 *
 * Copyright (C) 2019 by Caulier Gilles <caulier dot gilles at gmail dot com>
 * Copyright (c) 2019 by Maik Qualmann <metzpinguin at gmail dot com>
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

#include "dimgimagemagickloader.h"

// Qt includes

#include <QMimeDatabase>

// ImageMagick includes

// Pragma directives to reduce warnings from ImageMagick header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wkeyword-macro"
#endif

#include <Magick++.h>

// Restore warnings

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "dimgloaderobserver.h"

using namespace Magick;

namespace DigikamImageMagickDImgPlugin
{

DImgImageMagickLoader::DImgImageMagickLoader(DImg* const image)
    : DImgLoader(image)
{
    m_hasAlpha   = false;
    m_sixteenBit = false;
}

bool DImgImageMagickLoader::load(const QString& filePath, DImgLoaderObserver* const observer)
{
    QStringList blackList;
    blackList << QLatin1String("image/x-xcf");

    QString mimeType(QMimeDatabase().mimeTypeForFile(filePath).name());

    // qCDebug(DIGIKAM_DIMG_LOG) << "Mime type name:" << mimeType;

    if (blackList.contains(mimeType)                 ||
        mimeType.startsWith(QLatin1String("video/")) ||
        mimeType.startsWith(QLatin1String("audio/")))
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "Blacklisted from ImageMagick loader:" << mimeType;
        loadingFailed();
        return false;
    }

    readMetadata(filePath, DImg::QIMAGE);

    // Loading is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    if (observer)
    {
        observer->progressInfo(m_image, 0.5F);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Try to load image with ImageMagick codecs";

    try
    {
        Image image;

        if (m_loadFlags & LoadImageData)
        {
            try
            {
                image.read(filePath.toUtf8().constData());
            }
            catch (Warning& warning)
            {
                qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick warning [" << filePath << "]" << warning.what();
            }

            if (observer)
            {
                observer->progressInfo(m_image, 0.8F);
            }

            qCDebug(DIGIKAM_DIMG_LOG) << "IM to DImg      :" << image.columns() << image.rows();
            qCDebug(DIGIKAM_DIMG_LOG) << "IM QuantumRange :" << QuantumRange;
            qCDebug(DIGIKAM_DIMG_LOG) << "IM Depth        :" << image.depth();
            qCDebug(DIGIKAM_DIMG_LOG) << "IM Format       :" << image.format().c_str();

            int depth             = image.depth();
            Blob* const pixelBlob = new Blob;
            image.write(pixelBlob, "BGRA", depth);
            qCDebug(DIGIKAM_DIMG_LOG) << "IM blob size    :" << pixelBlob->length();

            if (observer)
            {
                observer->progressInfo(m_image, 0.9F);
            }

            if (m_loadFlags & LoadICCData)
            {
                Blob iccBlob(image.iccColorProfile());
                QByteArray iccRawProfile((char*)iccBlob.data(), iccBlob.length());

                if (!iccRawProfile.isEmpty())
                {
                    imageSetIccProfile(IccProfile(iccRawProfile));
                }
            }

            if (observer)
            {
                observer->progressInfo(m_image, 1.0F);
            }

            imageWidth()  = image.columns();
            imageHeight() = image.rows();
            imageData()   = (uchar*)pixelBlob->data();

#if MagickLibVersion < 0x700
            m_hasAlpha    = image.matte();
#else
            m_hasAlpha    = image.alpha();
#endif

            m_sixteenBit  = (depth == 16);

            // We considering that PNG is the most representative format of an image loaded by ImageMagick
            imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
            imageSetAttribute(QLatin1String("originalColorModel"), DImg::RGB);
            imageSetAttribute(QLatin1String("originalBitDepth"),   depth);
            imageSetAttribute(QLatin1String("originalSize"),       QSize(image.columns(), image.rows()));
        }
        else
        {
            try
            {
                image.ping(filePath.toUtf8().constData());
            }
            catch (Warning& warning)
            {
                qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick warning [" << filePath << "]" << warning.what();
            }

            imageWidth()  = image.columns();
            imageHeight() = image.rows();

#if MagickLibVersion < 0x700
            m_hasAlpha    = image.matte();
#else
            m_hasAlpha    = image.alpha();
#endif

            m_sixteenBit  = (image.depth() == 16);

            imageSetAttribute(QLatin1String("format"),             QLatin1String("PNG"));
            imageSetAttribute(QLatin1String("originalColorModel"), DImg::RGB);
            imageSetAttribute(QLatin1String("originalBitDepth"),   m_sixteenBit ? 16 : 8);
            imageSetAttribute(QLatin1String("originalSize"),       QSize(image.columns(), image.rows()));
        }
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick exception [" << filePath << "]" << error.what();
        loadingFailed();
        return false;
    }

    return true;
}

bool DImgImageMagickLoader::save(const QString& filePath, DImgLoaderObserver* const observer)
{
    // Saving is opaque to us. No support for stopping from observer,
    // progress info are only pseudo values

    if (observer)
    {
        observer->progressInfo(m_image, 0.5F);
    }

    qCDebug(DIGIKAM_DIMG_LOG) << "Try to save image with ImageMagick codecs";

    try
    {
        QVariant formatAttr = imageGetAttribute(QLatin1String("format"));
        QByteArray format   = formatAttr.toByteArray();

        if (observer)
        {
            observer->progressInfo(m_image, 0.8F);
        }

        Blob pixelBlob(imageData(), imageNumBytes());

        Image image;
        image.size(Geometry(imageWidth(), imageHeight()));
        image.magick("BGRA");
        image.depth(imageBitsDepth());

#if MagickLibVersion < 0x700
        image.matte(imageHasAlpha());
#else
        image.alpha(imageHasAlpha());
#endif

        image.read(pixelBlob);
        image.magick(format.data());

        QByteArray iccRawProfile = m_image->getIccProfile().data();

        if (!iccRawProfile.isEmpty())
        {
            Blob iccBlob(iccRawProfile.data(), iccRawProfile.size());
            image.iccColorProfile(iccBlob);
        }

        image.write(filePath.toUtf8().constData());

        if (observer)
        {
            observer->progressInfo(m_image, 1.0F);
        }

        imageSetAttribute(QLatin1String("format"), format.toUpper());

        saveMetadata(filePath);
        return true;
    }
    catch (Exception& error)
    {
        qCWarning(DIGIKAM_DIMG_LOG) << "ImageMagick exception [" << filePath << "]" << error.what();
        return false;
    }

    return true;
}

bool DImgImageMagickLoader::hasAlpha() const
{
    return m_hasAlpha;
}

bool DImgImageMagickLoader::sixteenBit() const
{
    return m_sixteenBit;
}

bool DImgImageMagickLoader::isReadOnly() const
{
    return false;
}

} // namespace DigikamImageMagickDImgPlugin

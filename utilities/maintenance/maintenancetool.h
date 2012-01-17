/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-01-16
 * Description : Maintenance tool class
 *
 * Copyright (C) 2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MAINTENANCETOOL_H
#define MAINTENANCETOOL_H

// Qt includes

#include <QPixmap>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class DImg;
class LoadingDescription;
class ThumbnailLoadThread;
class PreviewLoadThread;

class MaintenanceTool : public ProgressItem
{
    Q_OBJECT

public:

    enum Mode
    {
        AllItems = 0,  /// Process all items from whole collections
        MissingItems,  /// Process missing items from whole collections
        AlbumItems     /// Process items from current album set by albumId
    };

public:

    MaintenanceTool(Mode mode=AllItems, int albumId=-1);
    virtual ~MaintenanceTool();

    void setTitle(const QString& title);

Q_SIGNALS:

    void signalProcessDone();

protected:

    /** Return all paths to process. Data container can be custumized
     */
    QStringList&         allPicturePath();

    /** Return mode set in contructor. see Mode enum for details
     */
    Mode                 mode();
    
    /** Return thumbs loader instance
     */
    ThumbnailLoadThread* thumbsLoadThread()  const;

    /** Return preview loader instance
     */
    PreviewLoadThread*   previewLoadThread() const;

    /** Call this method into processOne() to check if another item must be processed
     */
    bool                 checkToContinue() const;

    /** Re-implement this if you want to use thumb loader items processed
     */
    virtual void gotNewThumbnail(const LoadingDescription&, const QPixmap&) {};

    /** Re-implement this if you want to use preview loader items processed
     */
    virtual void gotNewPreview(const LoadingDescription&, const DImg&) {};

    /** In this method, you can filter items to manage, hosted by allPicturePath(). These paths will be 
     *  used calling processOne().
     */
    virtual void listItemstoProcess() = 0;
    
    /** In this method, you can use thumb load thread or preview load thread to get item images. 
     *  gotNewThumbnail() or gotNewPreview() will be called accordingly.
     */
    virtual void processOne() = 0;

private Q_SLOTS:

    /** This slot call listItemstoProcess() when tool is started
     */
    void slotRun();

    /** This slot is called when user cancel tool from gui
     */
    void slotCancel();
    
    /** This slot call gotNewThumbnail()
     */
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

    /** This slot call gotNewPreview()
     */
    void slotGotImagePreview(const LoadingDescription&, const DImg&);

private:

    /** Called when all is done. It fire signalProcessDone()
     */
    void complete();

private:

    class MaintenanceToolPriv;
    MaintenanceToolPriv* const d;
};

}  // namespace Digikam

#endif /* MAINTENANCETOOL_H */

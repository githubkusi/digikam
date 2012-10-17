/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-03-09
 * Description : Captions, Tags, and Rating properties editor
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEDESCEDITTAB_H
#define IMAGEDESCEDITTAB_H

// Qt includes

#include <QScrollArea>
#include <QPixmap>
#include <QEvent>

// KDE includes

#include <kvbox.h>

// Local includes

#include "digikam_export.h"
#include "searchtextbar.h"
#include "imageinfolist.h"
#include "albummanager.h"
#include "albummodel.h"
#include "metadatahub.h"

//kusi
#include "searchtextbar.h"
#include "addtagslineedit.h"


namespace Digikam
{
class ImageInfo;
class TaggingAction;

class ImageDescEditTab : public KVBox
{
    Q_OBJECT

public:

    ImageDescEditTab(QWidget* parent);
    ~ImageDescEditTab();

    void assignPickLabel(int pickId);
    void assignColorLabel(int colorId);
    void assignRating(int rating);
    void setItem(const ImageInfo& info = ImageInfo());
    void setItems(const ImageInfoList& infos);
    void populateTags();
    void setFocusToTagsView();
    void setFocusToNewTagEdit();
    AddTagsLineEdit* getNewTagEdit();
    
Q_SIGNALS:

    void progressEntered(const QString&);
    void progressValueChanged(float);
    void progressFinished();
    void signalTagFilterMatch(bool);
    void signalPrevItem();
    void signalNextItem();

protected:

    bool eventFilter(QObject* o, QEvent* e);

private:

    void initializeTags(QModelIndex& parent);
    void setTagState(TAlbum* tag, MetadataHub::TagStatus status);

    void setInfos(const ImageInfoList& infos);
    void setFocusToLastSelectedWidget();

    void updateTagsView();
    void updateComments();
    void updatePickLabel();
    void updateColorLabel();
    void updateRating();
    void updateDate();
    void updateTemplate();
    void updateRecentTags();

    bool singleSelection() const;
    void setMetadataWidgetStatus(int status, QWidget* widget);
    void metadataChange(qlonglong imageId);
    void resetMetadataChangeInfo();

private Q_SLOTS:

    void slotApplyAllChanges();
    void slotApplyChangesToAllVersions();
    void slotRevertAllChanges();
    void slotChangingItems();
    void slotTagsSearchChanged(const SearchTextSettings& settings);
    void slotTagStateChanged(Album* album, Qt::CheckState checkState);
    void slotCommentChanged();
    void slotTitleChanged();
    void slotDateTimeChanged(const QDateTime& dateTime);
    void slotPickLabelChanged(int pickId);
    void slotColorLabelChanged(int colorId);
    void slotRatingChanged(int rating);
    void slotTemplateSelected();
    void slotModified();
    void slotCreateNewTag();
    void slotTaggingActionActivated(const TaggingAction&);
    void slotReloadForMetadataChange();

    void slotImageTagsChanged(qlonglong imageId);
    void slotImagesChanged(int albumId);
    void slotImageRatingChanged(qlonglong imageId);
    void slotImageDateChanged(qlonglong imageId);
    void slotImageCaptionChanged(qlonglong imageId);

    void slotRecentTagsMenuActivated(int);
    void slotAssignedTagsToggled(bool);

    void slotMoreMenu();
    void slotReadFromFileMetadataToDatabase();
    void slotWriteToFileMetadataFromDatabase();

private:

    class ImageDescEditTabPriv;
    ImageDescEditTabPriv* const d;
};

}  // namespace Digikam

#endif  // IMAGEDESCEDITTAB_H

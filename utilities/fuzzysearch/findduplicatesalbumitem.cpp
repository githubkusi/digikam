/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-06-17
 * Description : Find Duplicates tree-view search album item.
 *
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <QPainter>
#include <QTreeWidget>

// KDE includes.

#include <klocale.h>

// Local includes.

#include "album.h"
#include "ddebug.h"
#include "searchxml.h"
#include "findduplicatesalbumitem.h"

namespace Digikam
{

FindDuplicatesAlbumItem::FindDuplicatesAlbumItem(QTreeWidget* parent, SAlbum* album)
                       : TreeFolderItem(parent, QString())
{
    m_album = album;
    if (m_album)
    {
        m_refImgInfo = ImageInfo(m_album->title().toLongLong());
        setText(0, m_refImgInfo.name());

        SearchXmlReader reader(m_album->query());
        reader.readToFirstField();
        QList<int> list;
        list << reader.valueToIntList();
        setText(1, QString::number(list.count()));
    }
}

FindDuplicatesAlbumItem::~FindDuplicatesAlbumItem()
{
}

void FindDuplicatesAlbumItem::setThumb(const QPixmap& pix)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize+2, iconSize+2);
    pixmap.fill(Qt::color0);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2)  - (pix.width()/2),
                 (pixmap.height()/2) - (pix.height()/2), pix);
    setIcon(0, QIcon(pixmap));
}

SAlbum* FindDuplicatesAlbumItem::album() const
{
    return m_album;
}

KUrl FindDuplicatesAlbumItem::refUrl() const
{
    return m_refImgInfo.fileUrl();
}

}  // NameSpace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-02-12
 * Description : Wrapper model for table view
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
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

#include "tableview_model.moc"

// local includes
#include "imagefiltermodel.h"
#include "tableview_columnfactory.h"
#include <valarray>
#include "modeltest/modeltest.h"
#include "databasewatch.h"
#include "databasechangesets.h"

/// @todo Clean up include list
#include "imageposition.h"
#include "databasechangesets.h"
#include "imagelister.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "imageinfo.h"
#include "thumbnailloadthread.h"
#include "thumbnaildatabaseaccess.h"
#include "thumbnaildb.h"
#include "databasefields.h"


namespace Digikam
{

class TableViewModel::Private
{
public:
    ImageFilterModel* sourceModel;
    TableViewColumnFactory* columnFactory;
    QList<TableViewColumn*> columnObjects;
};

TableViewModel::TableViewModel(TableViewColumnFactory* const tableViewColumnFactory, Digikam::ImageFilterModel* const sourceModel, QObject* parent)
  : QAbstractItemModel(parent),
    d(new Private())
{
    d->sourceModel = sourceModel;
    d->columnFactory = tableViewColumnFactory;

    connect(d->sourceModel, SIGNAL(modelAboutToBeReset()),
            this, SLOT(slotSourceModelAboutToBeReset()));
    connect(d->sourceModel, SIGNAL(modelReset()),
            this, SLOT(slotSourceModelReset()));
    connect(d->sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAboutToBeInserted(QModelIndex,int,int)));
    connect(d->sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsInserted(QModelIndex,int,int)));
    connect(d->sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(d->sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotSourceRowsRemoved(QModelIndex,int,int)));
    connect(d->sourceModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotSourceRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(d->sourceModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),
            this, SLOT(slotSourceRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(d->sourceModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(slotSourceLayoutAboutToBeChanged()));
    connect(d->sourceModel, SIGNAL(layoutChanged()),
            this, SLOT(slotSourceLayoutChanged()));
    connect(d->sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotSourceDataChanged(QModelIndex,QModelIndex)));

    connect(DatabaseAccess::databaseWatch(), SIGNAL(imageChange(ImageChangeset)),
            this, SLOT(slotDatabaseImageChanged(ImageChangeset)), Qt::QueuedConnection);

    d->columnObjects << d->columnFactory->getColumn(TableViewColumnConfiguration("thumbnail"));
    d->columnObjects << d->columnFactory->getColumn(TableViewColumnConfiguration("filename"));
    d->columnObjects << d->columnFactory->getColumn(TableViewColumnConfiguration("coordinates"));

    new ModelTest(this, this);
}

TableViewModel::~TableViewModel()
{
    /// @todo delete d->columnObjects, d->tableViewColumnDataSource
}

int TableViewModel::columnCount(const QModelIndex& i) const
{
    return d->columnObjects.count();
}

QModelIndex TableViewModel::toImageFilterModelIndex(const QModelIndex& i) const
{
    if (i.isValid())
    {
        Q_ASSERT(i.model()==this);
    }

    const int rowNumber = i.row();

    if ( (rowNumber<0) || (rowNumber>=d->sourceModel->rowCount()) )
    {
        return QModelIndex();
    }

    const QModelIndex sourceIndex = d->sourceModel->index(rowNumber, 0);
    if (!sourceIndex.isValid())
    {
        return QModelIndex();
    }

    return sourceIndex;
}

QVariant TableViewModel::data(const QModelIndex& i, int role) const
{
    const int columnNumber = i.column();

    QModelIndex sourceIndex = toImageFilterModelIndex(i);
    if (!sourceIndex.isValid())
    {
        return QVariant();
    }

    TableViewColumn* const myColumn = d->columnObjects.at(columnNumber);
    return myColumn->data(sourceIndex, role);
}

QModelIndex TableViewModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        // there are no sub-items
        return QModelIndex();
    }

    // test for valid row/column values
    if ( (row<0) || (column<0) ||
         (column>=d->columnObjects.count()) || (row>=d->sourceModel->rowCount())
       )
    {
        return QModelIndex();
    }

    /// @todo range-check on row, column

    return createIndex(row, column, 0);
}

QModelIndex TableViewModel::parent(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    // we only have top level items
    return QModelIndex();
}

int TableViewModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }

    return d->sourceModel->rowCount();
}

QVariant TableViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // test for valid input ranges
    if ( (section<0) || (section>=d->columnObjects.count()) )
    {
        return QVariant();
    }

    if ((orientation!=Qt::Horizontal)||(role!=Qt::DisplayRole))
    {
        return QVariant();
    }

    TableViewColumn* const myColumn = d->columnObjects.at(section);
    return myColumn->getTitle();
}

void TableViewModel::addColumnAt(const TableViewColumnDescription& description, const int targetColumn)
{
    /// @todo take additional configuration data of the column into account
    TableViewColumnConfiguration newConfiguration(description.columnId);

    addColumnAt(newConfiguration, targetColumn);
}

void TableViewModel::addColumnAt(const TableViewColumnConfiguration& configuration, const int targetColumn)
{
    TableViewColumn* const newColumn = d->columnFactory->getColumn(configuration);

    int newColumnIndex = targetColumn;
    if (targetColumn<0)
    {
        // a negative column index means "append after last column"
        newColumnIndex = d->columnObjects.count();
    }

    beginInsertColumns(QModelIndex(), newColumnIndex, newColumnIndex);
    if (newColumnIndex >= d->columnObjects.count())
    {
        d->columnObjects.append(newColumn);
    }
    else
    {
        d->columnObjects.insert(newColumnIndex, newColumn);
    }
    endInsertColumns();

    connect(newColumn, SIGNAL(signalDataChanged(QModelIndex)),
            this, SLOT(slotColumnDataChanged(QModelIndex)));
}

void TableViewModel::slotColumnDataChanged(const QModelIndex& sourceIndex)
{
    TableViewColumn* const senderColumn = qobject_cast<TableViewColumn*>(sender());

    /// @todo find a faster way to find the column number
    const int iColumn = d->columnObjects.indexOf(senderColumn);
    if (iColumn<0)
    {
        return;
    }

    QModelIndex changedIndex = index(sourceIndex.row(), iColumn, QModelIndex());
    emit(dataChanged(changedIndex, changedIndex));
}

void TableViewModel::removeColumnAt(const int columnIndex)
{
    beginRemoveColumns(QModelIndex(), columnIndex, columnIndex);
    TableViewColumn* removedColumn = d->columnObjects.takeAt(columnIndex);
    endRemoveColumns();

    delete removedColumn;
}

TableViewColumn* TableViewModel::getColumnObject(const int columnIndex)
{
    return d->columnObjects.at(columnIndex);
}

TableViewColumnProfile TableViewModel::getColumnProfile() const
{
    TableViewColumnProfile profile;

    for (int i=0; i<d->columnObjects.count(); ++i)
    {
        TableViewColumnConfiguration ic = d->columnObjects.at(i)->getConfiguration();
        profile.columnConfigurationList << ic;
    }

    return profile;
}

void TableViewModel::loadColumnProfile(const TableViewColumnProfile& columnProfile)
{
    while (!d->columnObjects.isEmpty())
    {
        removeColumnAt(0);
    }

    /// @todo disable updating of the model while this happens
    for (int i=0; i<columnProfile.columnConfigurationList.count(); ++i)
    {
        addColumnAt(columnProfile.columnConfigurationList.at(i), -1);
    }
}

void TableViewModel::slotSourceModelAboutToBeReset()
{
    // the source model is about to be reset. Propagate that change:
    beginResetModel();
}

void TableViewModel::slotSourceModelReset()
{
    // the source model is done resetting.
    endResetModel();
}

void TableViewModel::slotSourceRowsAboutToBeInserted(const QModelIndex& parent, int start, int end)
{
    beginInsertRows(parent, start, end);
}

void TableViewModel::slotSourceRowsInserted(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endInsertRows();
}

void TableViewModel::slotSourceRowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    beginRemoveRows(parent, start, end);
}

void TableViewModel::slotSourceRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)

    endRemoveRows();
}

void TableViewModel::slotSourceRowsAboutToBeMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                                  const QModelIndex& destinationParent, int destinationRow)
{
    beginMoveRows(sourceParent, sourceStart, sourceEnd, destinationParent, destinationRow);
}

void TableViewModel::slotSourceRowsMoved(const QModelIndex& sourceParent, int sourceStart, int sourceEnd,
                                         const QModelIndex& destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent)
    Q_UNUSED(sourceStart)
    Q_UNUSED(sourceEnd)
    Q_UNUSED(destinationParent)
    Q_UNUSED(destinationRow)

    endMoveRows();
}

void TableViewModel::slotSourceLayoutAboutToBeChanged()
{
    /// @todo Emitting layoutAboutToBeChanged and layoutChanged is tricky,
    ///       because we do not know what will change.
    ///       It looks like ImageFilterModel emits layoutAboutToBeChanged and layoutChanged
    ///       even when the resulting dataset will be empty, and ModelTest does not like that.
    ///       For now, the easiest workaround is resetting the model
//     emit(layoutAboutToBeChanged());
    beginResetModel();
}

void TableViewModel::slotSourceLayoutChanged()
{
    /// @todo See note in TableViewModel#slotSourceLayoutAboutToBeChanged
//     emit(layoutChanged());
    endResetModel();
}

void TableViewModel::slotSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    /// @todo Not sure when ImageFilterModel will emit this signal and whether
    ///       it will be enough to monitor DatabaseAccess::databaseWatch()::imageChange(ImageChangeset)

    const int topRow = topLeft.row();
    const int bottomRow = bottomRight.row();

    const int nColumns = d->columnObjects.count();

    const QModelIndex myTopLeft = index(topRow, 0, QModelIndex());
    const QModelIndex myBottomRight = index(bottomRow, nColumns-1, QModelIndex());

    emit(dataChanged(myTopLeft, myBottomRight));
}

void TableViewModel::slotDatabaseImageChanged(const ImageChangeset& imageChangeset)
{
//     const DatabaseFields::Set changes = imageChangeset.changes();

    /// @todo Decide which changes are relevant here or
    ///       let the TableViewColumn object decide which are relevant

    foreach(const qlonglong& id, imageChangeset.ids())
    {
        const QModelIndex& changedIndex = d->sourceModel->indexForImageId(id);
        if (changedIndex.isValid())
        {
            emit(dataChanged(changedIndex, changedIndex));
        }
    }
}

TableViewSortFilterProxyModel::TableViewSortFilterProxyModel(TableViewShared* const sPointer, QObject* parent)
  : QSortFilterProxyModel(parent),
    s(sPointer)
{
    setSourceModel(s->tableViewModel);
    setDynamicSortFilter(true);
}

TableViewSortFilterProxyModel::~TableViewSortFilterProxyModel()
{

}

bool TableViewSortFilterProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
{
    return QSortFilterProxyModel::lessThan(left, right);
}

} /* namespace Digikam */


/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Thumbnail DB schema update
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEFACESCHEMAUPDATER_H
#define DATABASEFACESCHEMAUPDATER_H

namespace FacesEngine
{

class DatabaseFaceAccess;
class DatabaseFaceInitObserver;

class DatabaseFaceSchemaUpdater
{
public:

    static int schemaVersion();

public:

    DatabaseFaceSchemaUpdater(DatabaseFaceAccess* const access);
    ~DatabaseFaceSchemaUpdater();

    bool update();
    void setObserver(DatabaseFaceInitObserver* const observer);

private:

    bool startUpdates();
    bool makeUpdates();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool updateV1ToV2();

private:

    class Private;
    Private* const d;
};

} // namespace FacesEngine

#endif // DATABASEFACESCHEMAUPDATER_H

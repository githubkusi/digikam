/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-02
 * Description : Database error handler interface
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DATABASEERRORHANDLER_H
#define DATABASEERRORHANDLER_H

// Local includes

#include "digikam_export.h"
#include "sqlquery.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseErrorAnswer
{
public:
    virtual ~DatabaseErrorAnswer() {};
    virtual void connectionErrorContinueQueries() = 0;
    virtual void connectionErrorAbortQueries() = 0;
};

class DIGIKAM_EXPORT DatabaseErrorHandler : public QObject
{
    Q_OBJECT

public Q_SLOTS:

    //Note: This is a slot, may be called by queued connection
    /** In the situation of a connection error,
     *  all threads will be waiting with their queries
     *  and this method is called.
     *  This method can display an error dialog and try to repair
     *  the connection.
     *  It must then call either connectionErrorContinueQueries()
     *  or connectionErrorAbortQueries().
     */
    virtual void databaseError(DatabaseErrorAnswer *answer, const SqlQuery &query) = 0;

};


} // namespace Digikam

#endif // DATABASEERRORHANDLER_H

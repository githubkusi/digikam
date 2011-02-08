/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATA_PAGE_H
#define METADATA_PAGE_H

// Local includes

#include "assistantdlgpage.h"

namespace Digikam
{

class MetadataPage : public AssistantDlgPage
{
    Q_OBJECT

public:

    MetadataPage(KAssistantDialog* dlg);
    ~MetadataPage();

    void saveSettings();

private:

    class MetadataPagePriv;
    MetadataPagePriv* const d;
};

}   // namespace Digikam

#endif /* METADATA_PAGE_H */

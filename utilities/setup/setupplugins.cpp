/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : setup Kipi plugins tab.
 *
 * Copyright (C) 2004-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011-2012 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#include "setupplugins.moc"

// Qt includes

#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QString>
#include <QGridLayout>
#include <QPushButton>

// KDE includes

#include <kdialog.h>
#include <klocale.h>

// Libkipi includes

#include <libkipi/pluginloader.h>
#include <libkipi/version.h>

namespace Digikam
{

class SetupPlugins::SetupPluginsPriv
{
public:

    SetupPluginsPriv() :
        pluginsNumber(0),
        pluginsNumberActivated(0),
        checkAllBtn(0),
        clearBtn(0),
        kipiConfig(0)
    {
    }

    QLabel*             pluginsNumber;
    QLabel*             pluginsNumberActivated;

    QPushButton*        checkAllBtn;
    QPushButton*        clearBtn;

    KIPI::ConfigWidget* kipiConfig;
};

SetupPlugins::SetupPlugins(QWidget* parent)
    : QScrollArea(parent), d(new SetupPluginsPriv)
{
    QWidget* panel = new QWidget(viewport());
    setWidget(panel);
    setWidgetResizable(true);

    QGridLayout* mainLayout   = new QGridLayout;
    d->pluginsNumber          = new QLabel;
    d->pluginsNumberActivated = new QLabel;

    d->checkAllBtn            = new QPushButton(i18n("Check all"));
    d->clearBtn               = new QPushButton(i18n("Clear"));


    if (KIPI::PluginLoader::instance())
    {
        d->kipiConfig = KIPI::PluginLoader::instance()->configWidget(panel);
        d->kipiConfig->setWhatsThis(i18n("A list of available Kipi plugins."));
    }

    mainLayout->addWidget(d->pluginsNumber,             0, 0, 1, 1);
    mainLayout->addWidget(d->pluginsNumberActivated,    0, 1, 1, 1);
    mainLayout->addWidget(d->kipiConfig,                1, 0, 1, -1);
    mainLayout->addWidget(d->checkAllBtn,               2, 3, 1, 1);
    mainLayout->addWidget(d->clearBtn,                  2, 4, 1, 1);
    mainLayout->setColumnStretch(2, 10);
    mainLayout->setMargin(KDialog::spacingHint());
    mainLayout->setSpacing(KDialog::spacingHint());

    panel->setLayout(mainLayout);

#if KIPI_VERSION < 0x010400
    d->checkAllBtn->setVisible(false);
    d->clearBtn->setVisible(false);
#endif

    initPlugins();

    // --------------------------------------------------------

    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    panel->setAutoFillBackground(false);

    // --------------------------------------------------------

    connect(d->checkAllBtn, SIGNAL(clicked()),
            this, SLOT(slotCheckAll()));

    connect(d->clearBtn, SIGNAL(clicked()),
            this, SLOT(slotClear()));
}

SetupPlugins::~SetupPlugins()
{
    delete d;
}

void SetupPlugins::initPlugins()
{
    if (KIPI::PluginLoader::instance())
    {
        KIPI::PluginLoader::PluginList list = KIPI::PluginLoader::instance()->pluginList();
        d->pluginsNumber->setText(i18np("1 Kipi plugin found",
                                        "%1 Kipi plugins found",
                                        list.count()));

        int activated = 0;
        KIPI::PluginLoader::PluginList::const_iterator it = list.constBegin();

        for (; it != list.constEnd(); ++it)
        {
            if ((*it)->shouldLoad())
            {
                ++activated;
            }
        }

        d->pluginsNumberActivated->setText(i18nc("%1: number of plugins activated",
                                                 "(%1 activated)", activated));
    }
}

void SetupPlugins::applyPlugins()
{
    if (KIPI::PluginLoader::instance())
    {
        d->kipiConfig->apply();
    }
}

void SetupPlugins::slotCheckAll()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
#if KIPI_VERSION >= 0x010400
    d->kipiConfig->slotCheckAll();
#endif
    QApplication::restoreOverrideCursor();
}

void SetupPlugins::slotClear()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
#if KIPI_VERSION >= 0x010400
    d->kipiConfig->slotClear();
#endif
    QApplication::restoreOverrideCursor();
}
}  // namespace Digikam

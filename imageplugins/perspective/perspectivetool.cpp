/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective .
 *
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "perspectivetool.h"
#include "perspectivetool.moc"

// Qt includes

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// KDE includes

#include <kaboutdata.h>
#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kglobal.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes

#include "daboutdata.h"
#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "perspectivewidget.h"
#include "version.h"

using namespace Digikam;

namespace DigikamPerspectiveImagesPlugin
{

class PerspectiveToolPriv
{
public:

    PerspectiveToolPriv()
    {
        newWidthLabel             = 0;
        newHeightLabel            = 0;
        topLeftAngleLabel         = 0;
        topRightAngleLabel        = 0;
        bottomLeftAngleLabel      = 0;
        bottomRightAngleLabel     = 0;
        drawWhileMovingCheckBox   = 0;
        drawGridCheckBox          = 0;
        inverseTransformation     = 0;
        previewWidget             = 0;
        gboxSettings              = 0;
    }

    QLabel*             newWidthLabel;
    QLabel*             newHeightLabel;
    QLabel*             topLeftAngleLabel;
    QLabel*             topRightAngleLabel;
    QLabel*             bottomLeftAngleLabel;
    QLabel*             bottomRightAngleLabel;

    QCheckBox*          drawWhileMovingCheckBox;
    QCheckBox*          drawGridCheckBox;
    QCheckBox*          inverseTransformation;

    PerspectiveWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

PerspectiveTool::PerspectiveTool(QObject* parent)
               : EditorTool(parent),
                 d(new PerspectiveToolPriv)
{
    setObjectName("perspective");
    setToolName(i18n("Perspective"));
    setToolIcon(SmallIcon("perspective"));

    // -------------------------------------------------------------

    QFrame *frame   = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l  = new QVBoxLayout(frame);
    d->previewWidget = new PerspectiveWidget(525, 350, frame);
    l->addWidget(d->previewWidget);
    d->previewWidget->setWhatsThis(i18n("This is the perspective transformation operation preview. "
                                        "You can use the mouse for dragging the corner to adjust the "
                                        "perspective transformation area."));
    setToolView(frame);

    // -------------------------------------------------------------

    QString temp;
    Digikam::ImageIface iface(0, 0);

    d->gboxSettings = new EditorToolSettings(EditorToolSettings::Default|
                                             EditorToolSettings::Ok|
                                             EditorToolSettings::Cancel,
                                             EditorToolSettings::ColorGuide);

    // -------------------------------------------------------------

    QLabel *label1   = new QLabel(i18n("New width:"));
    d->newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"));
    d->newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel *label2    = new QLabel(i18n("New height:"));
    d->newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"));
    d->newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    // -------------------------------------------------------------

    KSeparator *line         = new KSeparator (Qt::Horizontal);
    QLabel *angleLabel       = new QLabel(i18n("Angles (in degrees):"));
    QLabel *label3           = new QLabel(i18n("  Top left:"));
    d->topLeftAngleLabel     = new QLabel;
    QLabel *label4           = new QLabel(i18n("  Top right:"));
    d->topRightAngleLabel    = new QLabel;
    QLabel *label5           = new QLabel(i18n("  Bottom left:"));
    d->bottomLeftAngleLabel  = new QLabel;
    QLabel *label6           = new QLabel(i18n("  Bottom right:"));
    d->bottomRightAngleLabel = new QLabel;

    // -------------------------------------------------------------

    KSeparator *line2          = new KSeparator (Qt::Horizontal);
    d->drawWhileMovingCheckBox = new QCheckBox(i18n("Draw preview while moving"));
    d->drawGridCheckBox        = new QCheckBox(i18n("Draw grid"));
    d->inverseTransformation   = new QCheckBox(i18n("Inverse transformation"));

    // -------------------------------------------------------------

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,                       0, 0, 1, 1);
    mainLayout->addWidget(d->newWidthLabel,             0, 1, 1, 2);
    mainLayout->addWidget(label2,                       1, 0, 1, 1);
    mainLayout->addWidget(d->newHeightLabel,            1, 1, 1, 2);
    mainLayout->addWidget(line,                         2, 0, 1, 3);
    mainLayout->addWidget(angleLabel,                   3, 0, 1, 3);
    mainLayout->addWidget(label3,                       4, 0, 1, 1);
    mainLayout->addWidget(d->topLeftAngleLabel,         4, 1, 1, 2);
    mainLayout->addWidget(label4,                       5, 0, 1, 1);
    mainLayout->addWidget(d->topRightAngleLabel,        5, 1, 1, 2);
    mainLayout->addWidget(label5,                       6, 0, 1, 1);
    mainLayout->addWidget(d->bottomLeftAngleLabel,      6, 1, 1, 2);
    mainLayout->addWidget(label6,                       7, 0, 1, 1);
    mainLayout->addWidget(d->bottomRightAngleLabel,     7, 1, 1, 2);
    mainLayout->addWidget(line2,                        8, 0, 1, 3);
    mainLayout->addWidget(d->drawWhileMovingCheckBox,   9, 0, 1, 3);
    mainLayout->addWidget(d->drawGridCheckBox,         10, 0, 1, 3);
    mainLayout->addWidget(d->inverseTransformation,    11, 0, 1, 3);
    mainLayout->setColumnStretch(1, 10);
    mainLayout->setRowStretch(12, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalPerspectiveChanged(const QRect&, float, float, float, float)),
            this, SLOT(slotUpdateInfo(const QRect&, float, float, float, float)));

    connect(d->drawWhileMovingCheckBox, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleDrawWhileMoving(bool)));

    connect(d->drawGridCheckBox, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleDrawGrid(bool)));

    connect(d->inverseTransformation, SIGNAL(toggled(bool)),
            this, SLOT(slotInverseTransformationChanged(bool)));

    connect(d->gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));
}

PerspectiveTool::~PerspectiveTool()
{
    delete d;
}

void PerspectiveTool::slotColorGuideChanged()
{
    d->previewWidget->slotChangeGuideColor(d->gboxSettings->guideColor());
    d->previewWidget->slotChangeGuideSize(d->gboxSettings->guideSize());
}

void PerspectiveTool::readSettings()
{
    QColor defaultGuideColor(Qt::red);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("perspective Tool");
    d->drawWhileMovingCheckBox->setChecked(group.readEntry("Draw While Moving", true));
    d->drawGridCheckBox->setChecked(group.readEntry("Draw Grid", false));
    d->inverseTransformation->setChecked(group.readEntry("Inverse Transformation", false));
    d->previewWidget->slotToggleDrawWhileMoving(d->drawWhileMovingCheckBox->isChecked());
    d->previewWidget->slotToggleDrawGrid(d->drawGridCheckBox->isChecked());
}

void PerspectiveTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group("perspective Tool");
    group.writeEntry("Draw While Moving", d->drawWhileMovingCheckBox->isChecked());
    group.writeEntry("Draw Grid", d->drawGridCheckBox->isChecked());
    group.writeEntry("Inverse Transformation", d->inverseTransformation->isChecked());
    config->sync();
}

void PerspectiveTool::slotResetSettings()
{
    d->previewWidget->reset();
}

void PerspectiveTool::finalRendering()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    d->previewWidget->applyPerspectiveAdjustment();
    kapp->restoreOverrideCursor();
}

void PerspectiveTool::slotUpdateInfo(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                     float bottomLeftAngle, float bottomRightAngle)
{
    QString temp;
    d->newWidthLabel->setText(temp.setNum(newSize.width()) + i18n(" px"));
    d->newHeightLabel->setText(temp.setNum(newSize.height()) + i18n(" px"));

    d->topLeftAngleLabel->setText(temp.setNum(topLeftAngle, 'f', 1));
    d->topRightAngleLabel->setText(temp.setNum(topRightAngle, 'f', 1));
    d->bottomLeftAngleLabel->setText(temp.setNum(bottomLeftAngle, 'f', 1));
    d->bottomRightAngleLabel->setText(temp.setNum(bottomRightAngle, 'f', 1));
}

void PerspectiveTool::slotInverseTransformationChanged(bool b)
{
    d->drawWhileMovingCheckBox->setEnabled(!b);
    d->drawGridCheckBox->setEnabled(!b);
    d->previewWidget->slotInverseTransformationChanged(b);
}

}  // namespace DigikamPerspectiveImagesPlugin

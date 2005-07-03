/* ============================================================
 * File  : imageeffect_restoration.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-03-26
 * Description : a digiKam image editor plugin to restore 
 *               a photograph
 * 
 * Copyright 2005 by Gilles Caulier
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
 
// C++ include.

#include <cstdio>
#include <cmath>
#include <cstring>

// Qt includes.

#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qlayout.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qevent.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qimage.h>

// KDE includes.

#include <kcursor.h>
#include <kurllabel.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kdebug.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "cimgiface.h"
#include "imageeffect_restoration.h"

namespace DigikamRestorationImagesPlugin
{

ImageEffect_Restoration::ImageEffect_Restoration(QWidget* parent)
                       : KDialogBase(Plain, i18n("Restoration"),
                                     Help|User1|User2|User3|Ok|Cancel, Ok,
                                     parent, 0, true, true,
                                     i18n("&Reset Values"),
                                     i18n("&Load..."),
                                     i18n("&Save As...")),
                         m_parent(parent)
{
    QString whatsThis;
    setButtonWhatsThis ( User1, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User2, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User3, i18n("<p>Save all filter parameters to settings text file.") );
    resize(configDialogSize("Restoration Tool Dialog"));  
        
    m_currentRenderingMode = NoneRendering;
    m_timer                = 0L;
    m_cimgInterface        = 0L;

    // About data and help button.
    
    KAboutData* about = new KAboutData("digikamimageplugins",
                                       I18N_NOOP("Photograph Restoration"), 
                                       digikamimageplugins_version,
                                       I18N_NOOP("A digiKam image plugin to restore a photograph."),
                                       KAboutData::License_GPL,
                                       "(c) 2005, Gilles Caulier", 
                                       0,
                                       "http://extragear.kde.org/apps/digikamimageplugins");
    
    about->addAuthor("Gilles Caulier", I18N_NOOP("Author and maintainer"),
                     "caulier dot gilles at free.fr");
                     
    about->addAuthor("David Tschumperle", I18N_NOOP("CImg library"), 0,
                     "http://cimg.sourceforge.net");
                        
    about->addAuthor("Gerhard Kulzer", I18N_NOOP("Feedback and plugin polishing"), 
                     "gerhard at kulzer.net");
    
    m_helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Photograph Restoration Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    m_helpButton->setPopup( helpMenu->menu() );
    
    // -------------------------------------------------------------

    QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint());

    QFrame *headerFrame = new QFrame( plainPage() );
    headerFrame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QHBoxLayout* layout = new QHBoxLayout( headerFrame );
    layout->setMargin( 2 ); // to make sure the frame gets displayed
    layout->setSpacing( 0 );
    QLabel *pixmapLabelLeft = new QLabel( headerFrame, "pixmapLabelLeft" );
    pixmapLabelLeft->setScaledContents( false );
    layout->addWidget( pixmapLabelLeft );
    QLabel *labelTitle = new QLabel( i18n("Photograph Restoration"), headerFrame, "labelTitle" );
    layout->addWidget( labelTitle );
    layout->setStretchFactor( labelTitle, 1 );
    topLayout->addWidget(headerFrame);
    
    QString directory;
    KGlobal::dirs()->addResourceType("digikamimageplugins_banner_left", KGlobal::dirs()->kde_default("data") +
                                                                        "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("digikamimageplugins_banner_left",
                                                 "digikamimageplugins_banner_left.png");
    
    pixmapLabelLeft->setPaletteBackgroundColor( QColor(201, 208, 255) );
    pixmapLabelLeft->setPixmap( QPixmap( directory + "digikamimageplugins_banner_left.png" ) );
    labelTitle->setPaletteBackgroundColor( QColor(201, 208, 255) );
    
    // -------------------------------------------------------------

    m_imagePreviewWidget = new Digikam::ImagePannelWidget(240, 160, plainPage(), true);
    topLayout->addWidget(m_imagePreviewWidget);
    
    m_imagePreviewWidget->setProgress(0);
    m_imagePreviewWidget->setProgressWhatsThis(i18n("<p>This is the current percentage of the task completed."));
    
    // -------------------------------------------------------------
    
    m_mainTab = new QTabWidget( m_imagePreviewWidget );
    
    QWidget* firstPage = new QWidget( m_mainTab );
    QGridLayout* grid = new QGridLayout( firstPage, 2, 2, marginHint(), spacingHint());
    m_mainTab->addTab( firstPage, i18n("Preset") );

    KURLLabel *cimgLogoLabel = new KURLLabel(firstPage);
    cimgLogoLabel->setText(QString::null);
    cimgLogoLabel->setURL("http://cimg.sourceforge.net");
    KGlobal::dirs()->addResourceType("cimg-logo", KGlobal::dirs()->kde_default("data") + "digikamimageplugins/data");
    directory = KGlobal::dirs()->findResourceDir("cimg-logo", "cimg-logo.png");
    cimgLogoLabel->setPixmap( QPixmap( directory + "cimg-logo.png" ) );
    QToolTip::add(cimgLogoLabel, i18n("Visit CImg library website"));
    
    QLabel *typeLabel = new QLabel(i18n("Filtering type:"), firstPage);
    m_restorationTypeCB = new QComboBox( false, firstPage ); 
    m_restorationTypeCB->insertItem( i18n("None") );
    m_restorationTypeCB->insertItem( i18n("Reduce Uniform Noise") );
    m_restorationTypeCB->insertItem( i18n("Reduce JPEG Artefacts") );
    m_restorationTypeCB->insertItem( i18n("Reduce Texturing") );
    QWhatsThis::add( m_restorationTypeCB, i18n("<p>Select here the filter preset to use for photograph restoration:<p>"
                                               "<b>None</b>: Most common values. Puts settings to default.<p>"
                                               "<b>Reduce Uniform Noise</b>: reduce small image artefacts like sensor noise.<p>"
                                               "<b>Reduce JPEG Artefacts</b>: reduce large image artefacts like JPEG compression mosaic.<p>"
                                               "<b>Reduce Texturing</b>: reduce image artefacts like paper texture or Moire patterns "
                                               "of a scanned image.<p>"));

    grid->addMultiCellWidget(cimgLogoLabel, 0, 0, 1, 1);
    grid->addMultiCellWidget(typeLabel, 1, 1, 0, 0);
    grid->addMultiCellWidget(m_restorationTypeCB, 1, 1, 1, 1);
    grid->setRowStretch(1, 10);
    
    // -------------------------------------------------------------
    
    QWidget* secondPage = new QWidget( m_mainTab );
    QGridLayout* grid2 = new QGridLayout( secondPage, 5, 2, marginHint(), spacingHint());
    m_mainTab->addTab( secondPage, i18n("Smoothing") );
    
    m_detailLabel = new QLabel(i18n("Detail preservation:"), secondPage);
    m_detailInput = new KDoubleNumInput(secondPage);
    m_detailInput->setPrecision(2);
    m_detailInput->setRange(0.0, 100.0, 0.1, true);
    QWhatsThis::add( m_detailInput, i18n("<p>Preservation of details to set the sharpening level of the small features in the target image. "
                                         "Higher values leave details sharp."));
    grid2->addMultiCellWidget(m_detailLabel, 0, 0, 0, 0);
    grid2->addMultiCellWidget(m_detailInput, 0, 0, 1, 1);

    m_gradientLabel = new QLabel(i18n("Anisotropy:"), secondPage);
    m_gradientInput = new KDoubleNumInput(secondPage);
    m_gradientInput->setPrecision(2);
    m_gradientInput->setRange(0.0, 1.0, 0.1, true);
    QWhatsThis::add( m_gradientInput, i18n("<p>Anisotropic (directional) modifier of the details. Keep it small for Gaussian noise."));
    grid2->addMultiCellWidget(m_gradientLabel, 1, 1, 0, 0);
    grid2->addMultiCellWidget(m_gradientInput, 1, 1, 1, 1);

    m_timeStepLabel = new QLabel(i18n("Smoothing:"), secondPage);
    m_timeStepInput = new KDoubleNumInput(secondPage);
    m_timeStepInput->setPrecision(2);
    m_timeStepInput->setRange(0.0, 500.0, 0.1, true);
    QWhatsThis::add( m_timeStepInput, i18n("<p>Total smoothing power: if Detail Factor sets the relative smoothing and Gradient Factor the "
                                           "direction, Time Step sets the overall effect."));
    grid2->addMultiCellWidget(m_timeStepLabel, 2, 2, 0, 0);
    grid2->addMultiCellWidget(m_timeStepInput, 2, 2, 1, 1);

    m_blurLabel = new QLabel(i18n("Regularity:"), secondPage);
    m_blurInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(2);
    m_blurInput->setRange(0.0, 10.0, 0.1, true);
    QWhatsThis::add( m_blurInput, i18n("<p>This value controls the smoothing regularity of the target image. "
                                       "Do not use an high value here, or the "
                                       "target image will be completely blurred."));
    grid2->addMultiCellWidget(m_blurLabel, 3, 3, 0, 0);
    grid2->addMultiCellWidget(m_blurInput, 3, 3, 1, 1);
    
    m_blurItLabel = new QLabel(i18n("Iterations:"), secondPage);
    m_blurItInput = new KDoubleNumInput(secondPage);
    m_blurInput->setPrecision(1);
    m_blurItInput->setRange(1.0, 16.0, 1.0, true);
    QWhatsThis::add( m_blurItInput, i18n("<p>Sets the number of times the filter is applied on the target image."));
    grid2->addMultiCellWidget(m_blurItLabel, 4, 4, 0, 0);
    grid2->addMultiCellWidget(m_blurItInput, 4, 4, 1, 1);
    
    // -------------------------------------------------------------
    
    QWidget* thirdPage = new QWidget( m_mainTab );
    QGridLayout* grid3 = new QGridLayout( thirdPage, 4, 2, marginHint(), spacingHint());
    m_mainTab->addTab( thirdPage, i18n("Advanced Settings") );
    
    m_angularStepLabel = new QLabel(i18n("Angular step:"), thirdPage);
    m_angularStepInput = new KDoubleNumInput(thirdPage);
    m_angularStepInput->setPrecision(2);
    m_angularStepInput->setRange(5.0, 90.0, 5.0, true);
    QWhatsThis::add( m_angularStepInput, i18n("<p>Set here the angular integration step in degrees in analogy to anisotropy."));
    grid3->addMultiCellWidget(m_angularStepLabel, 0, 0, 0, 0);
    grid3->addMultiCellWidget(m_angularStepInput, 0, 0, 1, 1);

    m_integralStepLabel = new QLabel(i18n("Integral step:"), thirdPage);
    m_integralStepInput = new KDoubleNumInput(thirdPage);
    m_integralStepInput->setPrecision(2);
    m_integralStepInput->setRange(0.1, 10.0, 0.1, true);
    QWhatsThis::add( m_integralStepInput, i18n("<p>Set here the spatial integral step. Stay below 1."));
    grid3->addMultiCellWidget(m_integralStepLabel, 1, 1, 0, 0);
    grid3->addMultiCellWidget(m_integralStepInput, 1, 1, 1, 1);

    m_gaussianLabel = new QLabel(i18n("Gaussian:"), thirdPage);
    m_gaussianInput = new KDoubleNumInput(thirdPage);
    m_gaussianInput->setPrecision(2);
    m_gaussianInput->setRange(0.0, 500.0, 0.01, true);
    QWhatsThis::add( m_gaussianInput, i18n("<p>Set here the precision of the Gaussian function."));
    grid3->addMultiCellWidget(m_gaussianLabel, 2, 2, 0, 0);
    grid3->addMultiCellWidget(m_gaussianInput, 2, 2, 1, 1);
    
    m_linearInterpolationBox = new QCheckBox(i18n("Use linear interpolation"), thirdPage);
    QWhatsThis::add( m_linearInterpolationBox, i18n("<p>Enable this option to quench the last bit of quality (slow)."));
    grid3->addMultiCellWidget(m_linearInterpolationBox, 3, 3, 0, 1);
    
    m_normalizeBox = new QCheckBox(i18n("Normalize photograph"), thirdPage);
    QWhatsThis::add( m_normalizeBox, i18n("<p>Enable this option to process an output image normalization."));
    grid3->addMultiCellWidget(m_normalizeBox, 4, 4, 0, 1);
    
    m_imagePreviewWidget->setUserAreaWidget(m_mainTab);
    
    // -------------------------------------------------------------
    
    QTimer::singleShot(0, this, SLOT(slotUser1())); // Reset all parameters to the default values.
        
    // -------------------------------------------------------------
    
    connect(cimgLogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processCImgURL(const QString&)));
    
    connect(m_imagePreviewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotEffect()));

    connect(m_restorationTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotUser1()));
                        
    connect(m_detailInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_gradientInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_timeStepInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_blurInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_angularStepInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_integralStepInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 
                                                
    connect(m_gaussianInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));                 

    connect(m_blurItInput, SIGNAL(valueChanged (double)),
            this, SLOT(slotTimer()));  

    connect(m_linearInterpolationBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));                        

    connect(m_normalizeBox, SIGNAL(toggled (bool)),
            this, SLOT(slotEffect()));                        
}

ImageEffect_Restoration::~ImageEffect_Restoration()
{
    saveDialogSize("Restoration Tool Dialog");    
    
    // No need to delete m_previewData because it's driving by QImage.

    if (m_cimgInterface)
       delete m_cimgInterface;
       
    if (m_timer)
       delete m_timer;
}

void ImageEffect_Restoration::abortPreview()
{
    m_currentRenderingMode = NoneRendering;
    m_imagePreviewWidget->setPreviewImageWaitCursor(false);
    m_imagePreviewWidget->setProgress(0);
    setButtonText(User1, i18n("&Reset Values"));
    setButtonWhatsThis( User1, i18n("<p>Reset all parameters to the default values.") );
    enableButton(Ok, true);    
    enableButton(User2, true);
    enableButton(User3, true);                      
    m_imagePreviewWidget->setEnable(true);                 
    m_restorationTypeCB->setEnabled(true);
    m_detailInput->setEnabled(true);
    m_gradientInput->setEnabled(true);
    m_timeStepInput->setEnabled(true);
    m_blurInput->setEnabled(true);
    m_blurItInput->setEnabled(true);
    m_angularStepInput->setEnabled(true);
    m_integralStepInput->setEnabled(true);
    m_gaussianInput->setEnabled(true);
    m_linearInterpolationBox->setEnabled(true);
    m_normalizeBox->setEnabled(true);
}

void ImageEffect_Restoration::slotTimer()
{
    if (m_timer)
       {
       m_timer->stop();
       delete m_timer;
       }
    
    m_timer = new QTimer( this );
    connect( m_timer, SIGNAL(timeout()),
             this, SLOT(slotEffect()) );
    m_timer->start(500, true);
}

void ImageEffect_Restoration::slotUser1()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_cimgInterface->stopComputation();
       }
    else
       {
       m_detailInput->blockSignals(true);
       m_gradientInput->blockSignals(true);
       m_timeStepInput->blockSignals(true);
       m_blurInput->blockSignals(true);
       m_blurItInput->blockSignals(true);
       m_angularStepInput->blockSignals(true);
       m_integralStepInput->blockSignals(true);
       m_gaussianInput->blockSignals(true);
       m_linearInterpolationBox->blockSignals(true);
       m_normalizeBox->blockSignals(true);

       m_detailInput->setValue(0.1);
       m_gradientInput->setValue(0.9);
       m_timeStepInput->setValue(20.0);
       m_blurInput->setValue(1.4);
       m_blurItInput->setValue(1.0);
       m_angularStepInput->setValue(45.0);
       m_integralStepInput->setValue(0.8);
       m_gaussianInput->setValue(3.0);
       m_linearInterpolationBox->setChecked(false);
       m_normalizeBox->setChecked(false);
       
       switch(m_restorationTypeCB->currentItem())
          {
          case ReduceUniformNoise:
            {
            m_timeStepInput->setValue(40.0);
            break;
            }
          
          case ReduceJPEGArtefacts:
            {
            m_detailInput->setValue(0.3);
            m_blurInput->setValue(1.0);
            m_timeStepInput->setValue(100.0);
            m_blurItInput->setValue(2.0);
            break;
            }
          
          case ReduceTexturing:
            {
            m_detailInput->setValue(0.5);
            m_blurInput->setValue(1.5);
            m_timeStepInput->setValue(100.0);
            m_blurItInput->setValue(2.0);
            break;
            }
          }                       
                      
       m_detailInput->blockSignals(false);
       m_gradientInput->blockSignals(false);
       m_timeStepInput->blockSignals(false);
       m_blurInput->blockSignals(false);
       m_blurItInput->blockSignals(false);
       m_angularStepInput->blockSignals(false);
       m_integralStepInput->blockSignals(false);
       m_gaussianInput->blockSignals(false);
       m_linearInterpolationBox->blockSignals(false);
       m_normalizeBox->blockSignals(false);
        
       slotEffect();    
       }
} 

void ImageEffect_Restoration::slotCancel()
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_cimgInterface->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    done(Cancel);
}

void ImageEffect_Restoration::slotHelp()
{
    KApplication::kApplication()->invokeHelp("restoration", "digikamimageplugins");
}

void ImageEffect_Restoration::slotFocusChanged(void)
{
    if (m_currentRenderingMode == FinalRendering)
       {
       m_imagePreviewWidget->update();
       return;
       }
    else if (m_currentRenderingMode == PreviewRendering)
       {
       m_cimgInterface->stopComputation();
       }
       
    QTimer::singleShot(0, this, SLOT(slotEffect()));        
}

void ImageEffect_Restoration::processCImgURL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void ImageEffect_Restoration::closeEvent(QCloseEvent *e)
{
    if (m_currentRenderingMode != NoneRendering)
       {
       m_cimgInterface->stopComputation();
       kapp->restoreOverrideCursor();
       }
       
    e->accept();
}

void ImageEffect_Restoration::slotEffect()
{
     // Computation already in progress.
    if (m_currentRenderingMode != NoneRendering) return; 
    
    m_currentRenderingMode = PreviewRendering;
    
    m_imagePreviewWidget->setEnable(false);
    m_restorationTypeCB->setEnabled(false);
    m_detailInput->setEnabled(false);
    m_gradientInput->setEnabled(false);
    m_timeStepInput->setEnabled(false);
    m_blurInput->setEnabled(false);
    m_blurItInput->setEnabled(false);
    m_angularStepInput->setEnabled(false);
    m_integralStepInput->setEnabled(false);
    m_gaussianInput->setEnabled(false);
    m_linearInterpolationBox->setEnabled(false);
    m_normalizeBox->setEnabled(false);
    setButtonText(User1, i18n("&Abort"));
    setButtonWhatsThis( User1, i18n("<p>Abort the current image rendering.") );
    enableButton(Ok, false);
    enableButton(User2, false);
    enableButton(User3, false);    
    m_imagePreviewWidget->setPreviewImageWaitCursor(true);
    
    QImage previewImage = m_imagePreviewWidget->getOriginalClipImage();
    
    m_imagePreviewWidget->setProgress(0);
    
    if (m_cimgInterface)
       delete m_cimgInterface;
        
    m_cimgInterface = new DigikamImagePlugins::CimgIface(&previewImage, 
                                    (uint)m_blurItInput->value(),
                                    m_timeStepInput->value(),
                                    m_integralStepInput->value(),
                                    m_angularStepInput->value(),
                                    m_blurInput->value(),
                                    m_detailInput->value(),
                                    m_gradientInput->value(),
                                    m_gaussianInput->value(),   
                                    m_normalizeBox->isChecked(),
                                    m_linearInterpolationBox->isChecked(),
                                    true, false, false, NULL, 0, 0, 0, this);
}

void ImageEffect_Restoration::slotOk()
{
    m_currentRenderingMode = FinalRendering;
    m_imagePreviewWidget->setEnable(false);
    m_restorationTypeCB->setEnabled(false);
    m_detailInput->setEnabled(false);
    m_gradientInput->setEnabled(false);
    m_timeStepInput->setEnabled(false);
    m_blurInput->setEnabled(false);
    m_blurItInput->setEnabled(false);
    m_angularStepInput->setEnabled(false);
    m_integralStepInput->setEnabled(false);
    m_gaussianInput->setEnabled(false);
    m_linearInterpolationBox->setEnabled(false);
    m_normalizeBox->setEnabled(false);
    enableButton(Ok, false);
    enableButton(User1, false);
    enableButton(User2, false);
    enableButton(User3, false);
    kapp->setOverrideCursor( KCursor::waitCursor() );
    m_imagePreviewWidget->setProgress(0);
    m_mainTab->setCurrentPage(0);
    
    if (m_cimgInterface)
       delete m_cimgInterface;
       
    Digikam::ImageIface iface(0, 0);
    QImage originalImage(iface.originalWidth(), iface.originalHeight(), 32);
    uint *data = iface.getOriginalData();
    memcpy( originalImage.bits(), data, originalImage.numBytes() );
    
    m_cimgInterface = new DigikamImagePlugins::CimgIface(&originalImage, 
                                    (uint)m_blurItInput->value(),
                                    m_timeStepInput->value(),
                                    m_integralStepInput->value(),
                                    m_angularStepInput->value(),
                                    m_blurInput->value(),
                                    m_detailInput->value(),
                                    m_gradientInput->value(),
                                    m_gaussianInput->value(),   
                                    m_normalizeBox->isChecked(),
                                    m_linearInterpolationBox->isChecked(),
                                    true, false, false, NULL, 0, 0, 0, this);
    delete [] data;                                    
}

void ImageEffect_Restoration::customEvent(QCustomEvent *event)
{
    if (!event) return;

    DigikamImagePlugins::CimgIface::EventData *d = (DigikamImagePlugins::CimgIface::EventData*) event->data();

    if (!d) return;

    if (d->starting)           // Computation in progress !
        {
        m_imagePreviewWidget->setProgress(d->progress);
        }  
    else 
        {
        if (d->success)        // Computation Completed !
            {
            switch (m_currentRenderingMode)
              {
              case PreviewRendering:
                 {
                 kdDebug() << "Preview Restoration completed..." << endl;
                 
                 QImage imDest = m_cimgInterface->getTargetImage();
                 m_imagePreviewWidget->setPreviewImageData(imDest);
                 abortPreview();
                 break;
                 }
              
              case FinalRendering:
                 {
                 kdDebug() << "Final Restoration completed..." << endl;
                 Digikam::ImageIface iface(0, 0);
                 iface.putOriginalData(i18n("Restoration"), (uint*)m_cimgInterface->getTargetImage().bits());
                 kapp->restoreOverrideCursor();
                 accept();       
                 break;
                 }
              }
            }
        else                   // Computation Failed !
            {
            switch (m_currentRenderingMode)
                {
                case PreviewRendering:
                    {
                    kdDebug() << "Preview Restoration failed..." << endl;
                    // abortPreview() must be call here for set progress bar to 0 properly.
                    abortPreview();
                    break;
                    }
                
                case FinalRendering:
                    break;
                }
            }
        }
    
    delete d;        
}

void ImageEffect_Restoration::slotUser2()
{
    KURL loadRestorationFile = KFileDialog::getOpenURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Restoration Settings File to Load")) );
    if( loadRestorationFile.isEmpty() )
       return;

    QFile file(loadRestorationFile.path());
    
    if ( file.open(IO_ReadOnly) )   
        {
        QTextStream stream( &file );
        if ( stream.readLine() != "# Photograph Restoration Configuration File" )
           {
           KMessageBox::error(this, 
                        i18n("\"%1\" is not a Photograph Restoration settings text file.")
                        .arg(loadRestorationFile.fileName()));
           file.close();            
           return;
           }
        
        blockSignals(true);
        m_normalizeBox->setChecked( stream.readLine().toInt() );
        m_linearInterpolationBox->setChecked( stream.readLine().toInt() );
        
        m_detailInput->setValue( stream.readLine().toDouble() );
        m_gradientInput->setValue( stream.readLine().toDouble() );
        m_timeStepInput->setValue( stream.readLine().toDouble() );
        m_blurInput->setValue( stream.readLine().toDouble() );
        m_blurItInput->setValue( stream.readLine().toDouble() );
        m_angularStepInput->setValue( stream.readLine().toDouble() );
        m_integralStepInput->setValue( stream.readLine().toDouble() );
        m_gaussianInput->setValue( stream.readLine().toDouble() );
        blockSignals(false);
        slotEffect();  
        }
    else
        KMessageBox::error(this, i18n("Cannot load settings from the Photograph Restoration text file."));

    file.close();             
}

void ImageEffect_Restoration::slotUser3()
{
    KURL saveRestorationFile = KFileDialog::getSaveURL(KGlobalSettings::documentPath(),
                                            QString( "*" ), this,
                                            QString( i18n("Photograph Restoration Settings File to Save")) );
    if( saveRestorationFile.isEmpty() )
       return;

    QFile file(saveRestorationFile.path());
    
    if ( file.open(IO_WriteOnly) )   
        {
        QTextStream stream( &file );        
        stream << "# Photograph Restoration Configuration File\n";    
        stream << m_normalizeBox->isChecked() << "\n";    
        stream << m_linearInterpolationBox->isChecked() << "\n";    
        stream << m_detailInput->value() << "\n";    
        stream << m_gradientInput->value() << "\n";    
        stream << m_timeStepInput->value() << "\n";    
        stream << m_blurInput->value() << "\n";    
        stream << m_blurItInput->value() << "\n";    
        stream << m_angularStepInput->value() << "\n";    
        stream << m_integralStepInput->value() << "\n";    
        stream << m_gaussianInput->value() << "\n";    
        }
    else
        KMessageBox::error(this, i18n("Cannot save settings to the Photograph Restoration text file."));
    
    file.close();        
}

}  // NameSpace DigikamRestorationImagesPlugin

#include "imageeffect_restoration.moc"

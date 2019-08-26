#
# Copyright (c) 2010-2019 by Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# digiKam exported API

# -----------------------------------------------------------------------------------
# Headers to install

install(FILES

            ${CMAKE_CURRENT_BINARY_DIR}/app/utils/digikam_config.h
            ${CMAKE_CURRENT_BINARY_DIR}/app/utils/digikam_core_export.h
            ${CMAKE_CURRENT_BINARY_DIR}/app/utils/digikam_database_export.h
            ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/digikam_export.h
            ${CMAKE_CURRENT_SOURCE_DIR}/app/utils/digikam_globals.h

            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/core/dplugin.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/core/dpluginaction.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/core/dpluginauthor.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/core/dplugineditor.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/core/dplugingeneric.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/iface/dinfointerface.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/iface/dmetainfoiface.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/ditemslist.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/dplugindialog.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/dpreviewimage.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/dpreviewmanager.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/dsavesettingswidget.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/dwizarddlg.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dplugins/widgets/dwizardpage.h

            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/geoiface/items/gpsitemcontainer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/geoiface/correlator/gpsdatacontainer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/geoiface/core/geoifacetypes.h
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/geoiface/core/geocoordinates.h
            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/geolocation/geoiface/reversegeocoding/rginfo.h

            ${CMAKE_CURRENT_SOURCE_DIR}/utilities/imageeditor/editor/imageiface.h

            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dimg.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolor.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorpixelaccess.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorcomposer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dcolorblend.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/dshareddata.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/drawdecoding.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/history/historyimageid.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/history/filteraction.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/icc/iccprofile.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/bcg/bcgcontainer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/wb/wbcontainer.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/dimg/filters/curves/curvescontainer.h

            ${CMAKE_CURRENT_SOURCE_DIR}/libs/metadataengine/engine/metaengine.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/metadataengine/engine/metaengine_data.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/metadataengine/containers/photoinfocontainer.h

            ${CMAKE_CURRENT_SOURCE_DIR}/libs/rawengine/drawdecoder.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/rawengine/drawinfo.h
            ${CMAKE_CURRENT_SOURCE_DIR}/libs/rawengine/drawdecodersettings.h

        DESTINATION

            ${CMAKE_INSTALL_INCLUDEDIR}/digikam
)

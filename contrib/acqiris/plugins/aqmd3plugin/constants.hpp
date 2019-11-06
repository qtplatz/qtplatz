/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include <boost/uuid/uuid.hpp>

namespace aqmd3 {

    // constexpr boost::uuids::uuid aqmd3_observer = {{ 0x1c, 0x64, 0xd0, 0xc3, 0xd2, 0x2c, 0x4b, 0x79, 0xa5, 0x9c, 0x4c, 0x75, 0x2b, 0x90, 0x53, 0x3a }};
    // extern const boost::uuids::uuid aqmd3_observer;

    namespace Constants {

        const char ACTION_ID[] = "aqmd3.Action";
        const char MENU_ID[] = "aqmd3.Menu";

        const char AQMD3_MODE[] = "aqmd3.Mode";

        // common actions
        const char * const FILE_OPEN          = "aqmd3.FileOpen";
        ////////////////////////
        const char * const ACTION_CONNECT       = "aqmd3.Connect";
        const char * const ACTION_RUN           = "aqmd3.Run";
        const char * const ACTION_STOP          = "aqmd3.Stop";
        const char * const ACTION_REC           = "aqmd3.Rec";
        const char * const ACTION_SNAPSHOT      = "aqmd3.Snapshot";
        const char * const ACTION_SYNC          = "aqmd3.SyncTrig";
        const char * const ACTION_INJECT        = "aqmd3.Inject";
        const char * const PRINT_CURRENT_VIEW   = "aqmd3.print_current_view";
        const char * const SAVE_CURRENT_IMAGE   = "aqmd3.save_current_image";
        const char * const HIDE_DOCK            = "aqmd3.HideDock";

        const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
        const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";

        // icon
        const char * const ICON_CONNECT         = ":/aqmd3/images/control_power_blue.png";
        const char * const ICON_STOP            = ":/aqmd3/images/control_stop.png";
        const char * const ICON_RUN             = ":/aqmd3/images/control.png";
        const char * const ICON_REC_ON          = ":/aqmd3/images/control_record.png";
        const char * const ICON_REC_PAUSE       = ":/aqmd3/images/control-pause-record.png";
        const char * const ICON_SNAPSHOT        = ":/aqmd3/images/camera.png";
        const char * const ICON_SYNC            = ":/aqmd3/images/arrow-circle-double.png";
        const char * const ICON_INJECT          = ":/aqmd3/images/plus-circle.png";
        const char * const ICON_FOLDER_OPEN     = ":/aqmd3/images/folder-horizontal-open.png";
        const char * const ICON_DISK_PLUS       = ":/aqmd3/images/disk--plus.png";
        const char * const ICON_FILE_OPEN       = ":/aqmd3/images/folder-open-document.png";
        const char * const ICON_FILE_SAVE       = ":/aqmd3/images/disk.png";
        const char * const ICON_PDF             = ":/aqmd3/images/document-pdf.png";
        const char * const ICON_IMAGE           = ":/aqmd3/images/image.png";
        const char * const ICON_NEXT            = ":/aqmd3/images/blue-document-page-next.png";
        ///////////////////////////////////////////

        // settings
        const char * const GRP_DATA_FILES       = "DataFiles";
        const char * const GRP_METHOD_FILES     = "MethodFiles";
        const char * const KEY_FILES            = "Files";
        const char * const THIS_GROUP           = "aqmd3";

        // default method files
        const char * const LAST_METHOD          = "aqmd3.adfs";
        const char * const LAST_METHOD_AQMD3   = "aqmd3_aqmd3.cmth.xml";
        const char * const LAST_METHOD_INFITOF  = "aqmd3_infitof.cmth.xml";
        const char * const LAST_PROC_METHOD     = "aqmd3.pmth.xml";

    } // namespace Constants

}

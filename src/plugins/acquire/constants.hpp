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

namespace acquire {

    // constexpr boost::uuids::uuid acquire_observer = {{ 0x1c, 0x64, 0xd0, 0xc3, 0xd2, 0x2c, 0x4b, 0x79, 0xa5, 0x9c, 0x4c, 0x75, 0x2b, 0x90, 0x53, 0x3a }};
    // extern const boost::uuids::uuid acquire_observer;

    namespace Constants {

        const char ACTION_ID[] = "acquire.Action";
        const char MENU_ID[] = "acquire.Menu";

        const char ACQUIRE_MODE[] = "acquire.Mode";

        // common actions
        const char * const FILE_OPEN          = "acquire.FileOpen";
        ////////////////////////
        const char * const ACTION_CONNECT       = "acquire.Connect";
        const char * const ACTION_RUN           = "acquire.Run";
        const char * const ACTION_STOP          = "acquire.Stop";
        const char * const ACTION_REC           = "acquire.Rec";
        const char * const ACTION_SNAPSHOT      = "acquire.Snapshot";
        const char * const ACTION_SYNC          = "acquire.SyncTrig";
        const char * const ACTION_INJECT        = "acquire.Inject";
        const char * const PRINT_CURRENT_VIEW   = "acquire.print_current_view";
        const char * const SAVE_CURRENT_IMAGE   = "acquire.save_current_image";
        const char * const HIDE_DOCK            = "acquire.HideDock";

        const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
        const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";

        // icon
        const char * const ICON_CONNECT         = ":/acquire/images/control_power_blue.png";
        const char * const ICON_STOP            = ":/acquire/images/control_stop.png";
        const char * const ICON_RUN             = ":/acquire/images/control.png";
        const char * const ICON_REC_ON          = ":/acquire/images/control_record.png";
        const char * const ICON_REC_PAUSE       = ":/acquire/images/control-pause-record.png";
        const char * const ICON_SNAPSHOT        = ":/acquire/images/camera.png";
        const char * const ICON_SYNC            = ":/acquire/images/arrow-circle-double.png";
        const char * const ICON_INJECT          = ":/acquire/images/plus-circle.png";
        const char * const ICON_FOLDER_OPEN     = ":/acquire/images/folder-horizontal-open.png";
        const char * const ICON_DISK_PLUS       = ":/acquire/images/disk--plus.png";
        const char * const ICON_FILE_OPEN       = ":/acquire/images/folder-open-document.png";
        const char * const ICON_FILE_SAVE       = ":/acquire/images/disk.png";
        const char * const ICON_PDF             = ":/acquire/images/document-pdf.png";
        const char * const ICON_IMAGE           = ":/acquire/images/image.png";
        const char * const ICON_NEXT            = ":/acquire/images/blue-document-page-next.png";
        ///////////////////////////////////////////

        // settings
        const char * const GRP_DATA_FILES       = "DataFiles";
        const char * const GRP_METHOD_FILES     = "MethodFiles";
        const char * const KEY_FILES            = "Files";
        const char * const THIS_GROUP           = "acquire";

        // default method files
        const char * const LAST_METHOD          = "acquire.adfs";
        const char * const LAST_METHOD_ACQUIRE   = "acquire_acquire.cmth.xml";
        const char * const LAST_METHOD_INFITOF  = "acquire_infitof.cmth.xml";
        const char * const LAST_PROC_METHOD     = "acquire.pmth.xml";

    } // namespace Constants

}

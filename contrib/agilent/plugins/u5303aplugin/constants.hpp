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

namespace boost { namespace uuids { struct uuid; } }

namespace u5303a {

    extern const boost::uuids::uuid ap240_observer;
    extern const boost::uuids::uuid u5303a_observer;
    extern const boost::uuids::uuid histogram_observer;

    namespace Constants {

        const char ACTION_ID[] = "u5303a.Action";
        const char MENU_ID[] = "u5303a.Menu";
        
        const char U5303A_MODE[] = "u5303a.Mode";
        
        // common actions
        const char * const FILE_OPEN          = "u5303a.FileOpen";
        ////////////////////////
        const char * const ACTION_CONNECT       = "u5303a.Connect";    
        const char * const ACTION_RUN           = "u5303a.Run";
        const char * const ACTION_STOP          = "u5303a.Stop";
        const char * const ACTION_REC           = "u5303a.Rec";
        const char * const ACTION_SNAPSHOT      = "u5303a.Snapshot";
        const char * const ACTION_SYNC          = "u5303a.SyncTrig";
        const char * const ACTION_INJECT        = "u5303a.Inject";
        const char * const PRINT_CURRENT_VIEW   = "u5303a.print_current_view";
        const char * const SAVE_CURRENT_IMAGE   = "u5303a.save_current_image";
        const char * const HIDE_DOCK            = "u5303a.HideDock";

        const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
        const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";
        
        // icon
        const char * const ICON_CONNECT         = ":/u5303a/images/control_power_blue.png";
        const char * const ICON_STOP            = ":/u5303a/images/control_stop.png";
        const char * const ICON_RUN             = ":/u5303a/images/control.png";
        const char * const ICON_REC_ON          = ":/u5303a/images/control_record.png";
        const char * const ICON_REC_PAUSE       = ":/u5303a/images/control-pause-record.png";
        const char * const ICON_SNAPSHOT        = ":/u5303a/images/camera.png";
        const char * const ICON_SYNC            = ":/u5303a/images/arrow-circle-double.png";
        const char * const ICON_INJECT          = ":/u5303a/images/plus-circle.png";
        const char * const ICON_FOLDER_OPEN     = ":/u5303a/images/folder-horizontal-open.png";
        const char * const ICON_DISK_PLUS       = ":/u5303a/images/disk--plus.png";
        const char * const ICON_FILE_OPEN       = ":/u5303a/images/folder-open-document.png";
        const char * const ICON_FILE_SAVE       = ":/u5303a/images/disk.png";
        const char * const ICON_PDF             = ":/u5303a/images/document-pdf.png";
        const char * const ICON_IMAGE           = ":/u5303a/images/image.png";
        const char * const ICON_NEXT            = ":/u5303a/images/blue-document-page-next.png";
        ///////////////////////////////////////////

        // settings
        const char * const GRP_DATA_FILES       = "DataFiles";
        const char * const GRP_METHOD_FILES     = "MethodFiles";
        const char * const KEY_FILES            = "Files";
        const char * const THIS_GROUP           = "u5303a";

        // default method files
        const char * const LAST_METHOD          = "u5303a.adfs";
        const char * const LAST_METHOD_U5303A   = "u5303a_u5303a.cmth.xml";
        const char * const LAST_METHOD_INFITOF  = "u5303a_infitof.cmth.xml";
        const char * const LAST_PROC_METHOD     = "u5303a.pmth.xml";

    } // namespace Constants
    
}


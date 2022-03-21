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

namespace accutof { namespace acquire {

        // constexpr boost::uuids::uuid pkdavg_observer    = { 0x1F, 0xCB, 0xDD, 0x0E, 0x74, 0x45, 0x42, 0x5D, 0xBA, 0x6D, 0x4A, 0xE6, 0xB5, 0x57, 0xE5, 0xC9};
        // constexpr boost::uuids::uuid histogram_observer = { 0xD0, 0xE5, 0xC4, 0x7B, 0xDD, 0xE2, 0x49, 0x0B, 0x89, 0xA7, 0xA0, 0xFD, 0x86, 0xC4, 0x1D, 0x00};
        // share id with qtplatz-pkdavg
        constexpr boost::uuids::uuid trace_observer     = {{ 0x29, 0x7E, 0x26, 0x09, 0x78, 0xF4, 0x4B, 0xCC, 0xB2, 0xF3, 0x71, 0x0A, 0xCE, 0xB7, 0x8E, 0xE3 }};
        constexpr boost::uuids::uuid pkd_trace_observer = {{ 0xed, 0x91, 0x38, 0x17, 0x64, 0x8b, 0x4c, 0x07, 0xa9, 0x1b, 0xf5, 0x37, 0xe3, 0x71, 0x0a, 0x1c }};

        namespace Constants {

            const char ACTION_ID[] = "accutof.acquire.Action";
            const char MENU_ID[] = "accutof.acquire.Menu";

            const char PKDAVG_MODE[] = "accutof.acquire.Mode";

            // common actions
            const char * const FILE_OPEN            = "accutof.acquire.FileOpen";
            ////////////////////////
            const char * const ACTION_CONNECT       = "accutof.acquire.Connect";
            const char * const ACTION_RUN           = "accutof.acquire.Run";
            const char * const ACTION_STOP          = "accutof.acquire.Stop";
            const char * const ACTION_REC           = "accutof.acquire.Rec";
            const char * const ACTION_SNAPSHOT      = "accutof.acquire.Snapshot";
            const char * const ACTION_SYNC          = "accutof.acquire.SyncTrig";
            const char * const ACTION_INJECT        = "accutof.acquire.Inject";
            const char * const ACTION_DARK          = "accutof.acquire.Dark";
            const char * const ACTION_XIC_ZERO      = "accutof.acquire.XIC_ZERO";
            const char * const ACTION_XIC_CLEAR     = "accutof.acquire.XIC_CLEAR";
            const char * const ACTION_SEL_CALIBFILE = "accutof.acquire.sel_calibfile";
            const char * const PRINT_CURRENT_VIEW   = "accutof.acquire.print_current_view";
            const char * const SAVE_CURRENT_IMAGE   = "accutof.acquire.save_current_image";
            const char * const HIDE_DOCK            = "accutof.acquire.HideDock";

            const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
            const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";

            // icon
            const char * const ICON_CONNECT         = ":/accutof/images/control_power_blue.png";
            const char * const ICON_STOP            = ":/accutof/images/control_stop.png";
            const char * const ICON_RUN             = ":/accutof/images/control.png";
            const char * const ICON_REC_ON          = ":/accutof/images/control_record.png";
            const char * const ICON_REC_PAUSE       = ":/accutof/images/control-pause-record.png";
            const char * const ICON_SNAPSHOT        = ":/accutof/images/camera.png";
            const char * const ICON_SYNC            = ":/accutof/images/arrow-circle-double.png";
            const char * const ICON_INJECT          = ":/accutof/images/plus-circle.png";
            const char * const ICON_FOLDER_OPEN     = ":/accutof/images/folder-horizontal-open.png";
            const char * const ICON_DISK_PLUS       = ":/accutof/images/disk--plus.png";
            const char * const ICON_FILE_OPEN       = ":/accutof/images/folder-open-document.png";
            const char * const ICON_FILE_SAVE       = ":/accutof/images/disk.png";
            const char * const ICON_PDF             = ":/accutof/images/document-pdf.png";
            const char * const ICON_IMAGE           = ":/accutof/images/image.png";
            const char * const ICON_NEXT            = ":/accutof/images/blue-document-page-next.png";
            const char * const ICON_OWL             = ":/accutof/images/owl-icon32x32.png";
            const char * const ICON_BALANCE         = ":/accutof/images/balance.png";
            const char * const ICON_RECYCLE         = ":/accutof/images/recycle-symbol.png";
            const char * const ICON_CALIBRATE       = ":/accutof/images/calibrate.png";
            ///////////////////////////////////////////

            // settings
            constexpr const char * const GRP_DATA_FILES       = "DataFiles";
            constexpr const char * const GRP_METHOD_FILES     = "MethodFiles";
            constexpr const char * const KEY_FILES            = "Files";
            constexpr const char * const THIS_GROUP           = "pkdavg";
            constexpr const char * const GRP_MSCALIB_FILES    = "MSCalibFiles";

            // default method files
            const char * const LAST_METHOD          = "accutof-acquire.adfs";
            const char * const LAST_METHOD_PKDAVG   = "accutof-acquire.cmth.xml";
            const char * const LAST_PROC_METHOD     = "accutof-acquire.pmth.xml";

            // default calibfile
            const char * const DEFAULT_CALIB_FILE   = "default.msclb";

        } // namespace Constants

    }
}

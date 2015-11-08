// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

namespace acquire {

    extern const boost::uuids::uuid deprecated_observer;
    
    namespace Constants {

        const char ACTION_ID[] = "Acquire.Action";
        const char MENU_ID[] = "Acquire.Menu";
        
        const char * const C_MODE_ACQUIRE = "Acquire.Mode";
        const int P_MODE_ACQUIRE        = 99;
    
        // common actions
        const char * const ACTION_SNAPSHOT      = "Acquire.Snapshot";
        const char * const ACTION_CONNECT       = "Acquire.Connect";
        const char * const ACTION_RESET         = "Acquire.Reset";
        const char * const ACTION_INITIALRUN    = "Acquire.InitialRun";
        const char * const ACTION_RUN           = "Acquire.Run";
        const char * const ACTION_STOP          = "Acquire.Stop";
        //const char * const ACTION_ACQUISITION   = "Acquire.Acquisition";
        const char * const ACTION_INJECT        = "Acquire.Inject";
        const char * const METHODOPEN           = "Acquire.MethodOpen";
        const char * const METHODSAVE           = "Acquire.MethodSave";
        const char * const PRINT_CURRENT_VIEW   = "Acquire.print_current_view";
        const char * const SAVE_CURRENT_IMAGE   = "Acquire.save_current_image";
        const char * const HIDE_DOCK            = "Acquire.HideDock";

        // icon
        const char * const ICON_CONNECT         = ":/acquire/images/Button Refresh.png";
        const char * const ICON_START           = ":/acquire/images/debugger_start.png";
        const char * const ICON_INITRUN         = ":/acquire/images/Button Last.png";
        const char * const ICON_RUN             = ":/acquire/images/Button Play.png";
        const char * const ICON_INJECT          = ":/acquire/images/Button Add.png";
        const char * const ICON_STOP            = ":/acquire/images/Button Stop.png";
        //
        const char * const ICON_SNAPSHOT        = ":/acquire/images/snapshot_small.png";
        const char * const ICON_FILE_OPEN       = ":/acquire/images/fileopen.png";
        const char * const ICON_FILE_SAVE       = ":/acquire/images/filesave.png";
        const char * const ICON_PDF             = ":/acquire/images/document-pdf.png";
        const char * const ICON_IMAGE           = ":/acquire/images/image.png";
        const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
        const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";

        // settings
        const char * const GRP_DATA_FILES       = "DataFiles";
        const char * const GRP_METHOD_FILES     = "MethodFiles";
        const char * const GRP_SEQUENCE_FILES   = "SequenceFiles";
        const char * const KEY_FILES            = "Files";
    }

}



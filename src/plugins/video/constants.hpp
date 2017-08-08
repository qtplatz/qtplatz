/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

namespace video {
    namespace Constants {

        const char ACTION_ID[]                  = "video.action";
        const char MENU_ID[]                    = "video.menu";
        const char M_FILE_RECENTFILES[]         = "video.RecentFiles";

        const char * const C_VIDEO_MODE         = "video.Mode";
        const char * const VIDEO_FILE_OPEN      = "video.FileOpen";
        const char * const VIDEO_METHOD_OPEN    = "video.MethodOpen";
        const char * const VIDEO_METHOD_SAVE    = "video.MethodSave";
        const char * const VIDEO_SEQUENCE_OPEN  = "video.SequenceOpen";
        const char * const VIDEO_SEQUENCE_SAVE  = "video.SequenceSave";
        const char * const VIDEO_RECENTFILES    = "video.RecentFiles";
        const char * const VIDEO_PRINT_PDF      = "video.PrintPDF";

        const char * const PUBLISHER_FILE_MENU  = "video.Publisher.FileMenu";
        const char * const PUBLISHER_EDIT_MENU  = "video.Publisher.EditMenu";
        const char * const PUBLISHER_TEXT_MENU  = "video.Publisher.TextMenu";

        const int ICON_SIZE( 64 );
        const int ABOVE_HEADING_MARGIN( 10 );
        const int ABOVE_CONTENTS_MARGIN( 4 );
        const int BELOW_CONTENTS_MARGIN( 16 );
        const int PANEL_LEFT_MARGIN = 70;

        // settings
        const char * const GRP_DATA_FILES   = "DataFiles";
        const char * const GRP_METHOD_FILES = "MethodFiles";
        const char * const KEY_FILES        = "Files";
        const char * const KEY_REFERENCE    = "ReferenceFiles";
        const char * const GRP_DIRECTORIES  = "Directories";
        
    } // Constants
    
}



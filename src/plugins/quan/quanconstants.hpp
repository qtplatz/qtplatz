/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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
#ifndef QUANCONSTANTS_HPP
#define QUANCONSTANTS_HPP

namespace quan {
namespace Constants {

    const char ACTION_ID[]                 = "Quan.Action";
    const char MENU_ID[]                   = "Quan.Menu";
    const char M_FILE_RECENTFILES []       = "Quan.Menu.RecentFiles";
    const char * const C_QUAN_MODE         = "Quan.Mode";
    const char * const FILE_OPEN           = "Quan.FileOpen";
    const char * const QUAN_METHOD_OPEN    = "Quan.MethodOpen";
    const char * const QUAN_METHOD_SAVE    = "Quan.MethodSave";
    const char * const QUAN_SEQUENCE_OPEN  = "Quan.SequenceOpen";
    const char * const QUAN_SEQUENCE_SAVE  = "Quan.SequenceSave";

    const char * const PUBLISHER_FILE_MENU = "Quan.Publisher.FileMenu";
    const char * const PUBLISHER_EDIT_MENU = "Quan.Publisher.EditMenu";
    const char * const PUBLISHER_TEXT_MENU = "Quan.Publisher.TextMenu";

    // actions
    const char * const QUAN_SEQUENCE_RUN   = "Quan.Run";
    const char * const QUAN_SEQUENCE_STOP  = "Quan.Stop";

    // task
    const char * const QUAN_TASK_OPEN      = "Quan.Task.OpenConn";
    const char * const QUAN_TASK_PROC      = "Quan.Task.SampleProc";

    const int ICON_SIZE(64);
    
    const int ABOVE_HEADING_MARGIN(10);
    const int ABOVE_CONTENTS_MARGIN(4);
    const int BELOW_CONTENTS_MARGIN(16);
    const int PANEL_LEFT_MARGIN = 70;

    // object names
    const char * const editQuanMethodName     = "editQuanMethodName";
    const char * const editOutfile            = "editOutfile";
    const char * const editQuanFilename       = "editQuanFilename"; // result filename

    // Combo box
    const char * const cmbAvgAll            = "Average all";
    const char * const cmbTake1st           = "Take 1st spc.";
    const char * const cmbTake2nd           = "Take 2nd spc.";
    const char * const cmbTakeLast          = "Take last spc.";
    const char * const cmbProcEach          = "Proc. ea. spc.";

    // settings
    const char * const GRP_DATA_FILES       = "QuanDataFiles";
    const char * const GRP_SEQUENCE_FILES   = "SequenceFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";
    const char * const KEY_REFERENCE        = "ReferenceFiles";
    const char * const GRP_DIRECTORIES      = "Directories";

} // namespace quan
} // namespace Constants

#endif // QUANCONSTANTS_HPP


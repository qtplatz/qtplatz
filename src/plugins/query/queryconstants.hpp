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
#ifndef QUERYCONSTANTS_HPP
#define QUERYCONSTANTS_HPP

namespace query {
namespace Constants {

    const char ACTION_ID[]                 = "Query.Action";
    const char MENU_ID[]                   = "Query.Menu";
    const char M_FILE_RECENTFILES []       = "Query.Menu.RecentFiles";
    const char * const C_QUERY = "Query";
    const char * const C_QUERY_MODE         = "Query.Mode";
    const char * const FILE_OPEN           = "Query.FileOpen";
    const char * const QUERY_METHOD_OPEN    = "Query.MethodOpen";
    const char * const QUERY_METHOD_SAVE    = "Query.MethodSave";
    const char * const QUERY_SEQUENCE_OPEN  = "Query.SequenceOpen";
    const char * const QUERY_SEQUENCE_SAVE  = "Query.SequenceSave";
    const char * const QUERY_RECENTFILES    = "Query.RecentFiles";

    const char * const PUBLISHER_FILE_MENU = "Query.Publisher.FileMenu";
    const char * const PUBLISHER_EDIT_MENU = "Query.Publisher.EditMenu";
    const char * const PUBLISHER_TEXT_MENU = "Query.Publisher.TextMenu";

    // actions
    const char * const QUERY_SEQUENCE_RUN   = "Query.Run";
    const char * const QUERY_SEQUENCE_STOP  = "Query.Stop";

    // task
    const char * const QUERY_TASK_OPEN      = "Query.Task.OpenConn";
    const char * const QUERY_TASK_PROC      = "Query.Task.SampleProc";

    const int ICON_SIZE(64);
    
    const int ABOVE_HEADING_MARGIN(10);
    const int ABOVE_CONTENTS_MARGIN(4);
    const int BELOW_CONTENTS_MARGIN(16);
    const int PANEL_LEFT_MARGIN = 70;

    // object names
    const char * const editQueryMethodName     = "editQueryMethodName";
    const char * const editOutfile            = "editOutfile";
    const char * const editQueryFilename       = "editQueryFilename"; // result filename

    // Combo box
    const char * const cmbAvgAll            = "Average all";
    const char * const cmbTake1st           = "Take 1st spc.";
    const char * const cmbTake2nd           = "Take 2nd spc.";
    const char * const cmbTakeLast          = "Take last spc.";
    const char * const cmbProcEach          = "Proc. ea. spc.";

    // settings
    const char * const GRP_DATA_FILES       = "QueryDataFiles";
    const char * const GRP_SEQUENCE_FILES   = "SequenceFiles";
    const char * const GRP_METHOD_FILES     = "MethodFiles";
    const char * const KEY_FILES            = "Files";
    const char * const KEY_REFERENCE        = "ReferenceFiles";
    const char * const GRP_DIRECTORIES      = "Directories";

} // namespace query
} // namespace Constants

#endif // QUERYCONSTANTS_HPP


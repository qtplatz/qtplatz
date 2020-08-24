/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

namespace cluster {
    namespace Constants {

        const char ACTION_ID[]                  = "cluster.proc.action";
        const char MENU_ID[]                    = "cluster.proc.menu";
        const char M_FILE_RECENTFILES[]         = "cluster.proc.RecentFiles";
        // const char * const C_CLUSTER           = "Cluster.proc";
        const char * const C_CLUSTER_MODE        = "cluster.Mode";
        const char * const CLUSTER_FILE_OPEN     = "cluster.FileOpen";
        const char * const CLUSTER_METHOD_OPEN   = "cluster.MethodOpen";
        const char * const CLUSTER_METHOD_SAVE   = "cluster.MethodSave";
        const char * const CLUSTER_SEQUENCE_OPEN = "cluster.SequenceOpen";
        const char * const CLUSTER_SEQUENCE_SAVE = "cluster.SequenceSave";
        const char * const CLUSTER_RECENTFILES   = "cluster.RecentFiles";
        const char * const CLUSTER_PRINT_PDF     = "cluster.PrintPDF";

        const char * const PUBLISHER_FILE_MENU  = "cluster.Publisher.FileMenu";
        const char * const PUBLISHER_EDIT_MENU  = "cluster.Publisher.EditMenu";
        const char * const PUBLISHER_TEXT_MENU  = "cluster.Publisher.TextMenu";

        // // actions
        // const char * const CLUSTER_SEQUENCE_RUN = "cluster.Run";
        // const char * const CLUSTER_SEQUENCE_STOP = "cluster.Stop";

        // database task
        const char * const CLUSTER_TASK_OPEN = "cluster.Task.OpenConn";
        const char * const CLUSTER_TASK_PROC = "cluster.Task.SampleProc";
        const char * const HIDE_DOCK        = "cluster.HideDock";

        const char * const ICON_DOCKHIDE        = ":/dataproc/image/button_close.png";
        const char * const ICON_DOCKSHOW        = ":/dataproc/image/control-090-small.png";

        const int ICON_SIZE( 64 );

        const int ABOVE_HEADING_MARGIN( 10 );
        const int ABOVE_CONTENTS_MARGIN( 4 );
        const int BELOW_CONTENTS_MARGIN( 16 );
        const int PANEL_LEFT_MARGIN = 70;

        // object names
        const char * const editClusterMethodName = "editClusterMethodName";
        const char * const editOutfile = "editOutfile";
        const char * const editClusterFilename = "editClusterFilename"; // result filename

        // Combo box
        const char * const cmbAvgAll = "Average all";
        const char * const cmbTake1st = "Take 1st spc.";
        const char * const cmbTake2nd = "Take 2nd spc.";
        const char * const cmbTakeLast = "Take last spc.";
        const char * const cmbProcEach = "Proc. ea. spc.";

        // settings
        const char * const GRP_DATA_FILES = "ClusterDataFiles";
        const char * const GRP_SEQUENCE_FILES = "SequenceFiles";
        const char * const GRP_METHOD_FILES = "MethodFiles";
        const char * const KEY_FILES = "Files";
        const char * const KEY_REFERENCE = "ReferenceFiles";
        const char * const GRP_DIRECTORIES = "Directories";

    } // Constants

}

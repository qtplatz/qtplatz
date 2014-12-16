/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

namespace malpix {
    namespace mpx4 {
        namespace Constants {
            
            const char ACTION_ID[]                 = "Mpx4.Action";
            const char MENU_ID[]                   = "Mpx4.Menu";
            const char M_FILE_RECENTFILES []       = "Mpx4.Menu.RecentFiles";
            // actions
            const char * const QUAN_SEQUENCE_RUN   = "Mpx4.Run";
            const char * const QUAN_SEQUENCE_STOP  = "Mpx4.Stop";
            
            const char * const EDIT_PROCMETHOD     = "Mpx4.Edit.ProcessMethod";
            
            // settings
            const char * const GRP_DATA_FILES       = "DataFiles";
            const char * const GRP_SEQUENCE_FILES   = "SequenceFiles";
            const char * const GRP_METHOD_FILES     = "MethodFiles";
            const char * const GRP_VTHCOMP_FILES    = "VthCompFiles";
            const char * const KEY_FILES            = "Files";
            const char * const KEY_REFERENCE        = "ReferenceFiles";
            const char * const GRP_DIRECTORIES      = "Directories";
            
        } // namespace Constants
    } // namespace mpx4
} // malpix
    
#endif // QUANCONSTANTS_HPP


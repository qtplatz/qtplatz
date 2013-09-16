// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace dataproc {

    namespace Constants {
        const char * const C_DATAPROCESSOR      = "Dataprocessor";
        const char * const C_DATAPROC_MODE      = "Dataproc.Mode";
        const char * const C_DATA_TEXT_MIMETYPE = "application/txt";
        const char * const C_DATA_NATIVE_MIMETYPE = "application/adfs";

        // common actions
        const char * const METHOD_OPEN          = "dataproc.MethodOpen";
        const char * const METHOD_SAVE          = "dataproc.MethodSave";
        const char * const METHOD_APPLY         = "dataproc.MethodApply";
        const char * const PRINT_CURRENT_VIEW   = "dataproc.PrintCurrentView";

        // icon
        const char * const ICON_METHOD_SAVE     = ":/dataproc/image/filesave.png";
        const char * const ICON_METHOD_OPEN     = ":/dataproc/image/fileopen.png";
        const char * const ICON_METHOD_APPLY    = ":/dataproc/image/apply_small.png";
		const char * const ICON_PDF             = ":/dataproc/image/file_pdf.png"; // http://findicons.com/icon/74870/file_pdf?id=355001
                                                                                   // freeware license, Designed by Andy Gongea
    }

    enum ProcessType {
        CentroidProcess
        , IsotopeProcess
        , CalibrationProcess
        , PeakFindProcess 
    };

}


#endif // CONSTANTS_H

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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////


#ifndef DATAPUBLISHER_H
#define DATAPUBLISHER_H

#include "adcontrols_global.h"

namespace adcontrols {

    class dataSubscriber;
    class Chromatogram;
    class MassSpectrum;
    class LCMSDataset;

    class PDAData {
    public:
        size_t getChromatogramCount() const;  // number of chromatograms
        bool getChromatogram( int fcn, adcontrols::Chromatogram& ) const;
    };

    class ADCONTROLSSHARED_EXPORT dataPublisher { // visitor
    public:
        virtual ~dataPublisher() {}
        dataPublisher() {}

        virtual void visit( LCMSDataset& ) { /**/ }
        virtual void visit( PDAData& ) { /**/ }
    };

    ///////////

}

#endif // DATAPUBLISHER_H

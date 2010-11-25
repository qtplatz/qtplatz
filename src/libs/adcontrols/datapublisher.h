// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
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

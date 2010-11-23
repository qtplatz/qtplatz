// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef PROCESSEDDATASET_H
#define PROCESSEDDATASET_H

#include "adcontrols_global.h"
#include <string>

namespace adcontrols {

    namespace internal {
        class ProcessedDatasetImpl;
    }

    class ADCONTROLSSHARED_EXPORT ProcessedDataset {
    public:
        ProcessedDataset();

        void xml( const std::wstring& );
        const std::wstring& xml() const;

    private:
        internal::ProcessedDatasetImpl* impl_;
    };

}

#endif // PROCESSEDDATASET_H

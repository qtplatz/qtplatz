//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "processeddataset.h"

namespace adcontrols {
    namespace internal {

        class ProcessedDatasetImpl {
        public:
            ~ProcessedDatasetImpl() {}
            ProcessedDatasetImpl() {}
        public:
            std::wstring xml_;
        };
    }
}

using namespace adcontrols;
using namespace adcontrols::internal;

ProcessedDataset::ProcessedDataset() : impl_( new ProcessedDatasetImpl )
{
}

void
ProcessedDataset::xml( const std::wstring& xml )
{
    impl_->xml_ = xml;
}

const std::wstring&
ProcessedDataset::xml() const
{
    return impl_->xml_;
}
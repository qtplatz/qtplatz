// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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

#include "samplesequence.hpp"
#include "sampleprocessor.hpp"
#include <deque>
#include <mutex>
#include <thread>

namespace adacquire {

    class SampleSequence::impl {
    public:
        impl() {
        }
        
        ~impl() {
        }
        
        static std::unique_ptr< SampleSequence > instance_;
        std::mutex mutex_;
        std::deque< SampleSequence::value_type > que_; // SampleProcessors
    };

    std::unique_ptr< SampleSequence > SampleSequence::impl::instance_;    
}

using namespace adacquire;

SampleSequence::~SampleSequence()
{
}

SampleSequence::SampleSequence() : impl_( new impl() )
{
}

SampleSequence::iterator
SampleSequence::begin()
{
    return impl_->que_.begin();
}

SampleSequence::iterator
SampleSequence::end()
{
    return impl_->que_.end();    
}

SampleSequence::const_iterator
SampleSequence::begin() const
{
    return impl_->que_.begin();
}

SampleSequence::const_iterator
SampleSequence::end() const
{
    return impl_->que_.end();        
}

void
SampleSequence::clear()
{
    impl_->que_.clear();
}

size_t
SampleSequence::size() const
{
    return impl_->que_.size();
}

std::shared_ptr< const SampleProcessor >
SampleSequence::at( size_t idx ) const
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    if ( idx < impl_->que_.size() )
        return impl_->que_ [ idx ];
    return nullptr;
}

std::shared_ptr< const SampleProcessor >
SampleSequence::operator []( size_t idx ) const
{
    return at( idx );
}

void
SampleSequence::operator << ( value_type& sp )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );

    impl_->que_.push_back( sp );
}

SampleSequence::value_type
SampleSequence::deque()
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    if ( !impl_->que_.empty() ) {
        auto sp = impl_->que_.front();
        impl_->que_.pop_front();
        return sp;
    }
    return 0;
}

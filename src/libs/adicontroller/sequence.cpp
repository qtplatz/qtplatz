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

#include "sequence.hpp"
#include "sampleprocessor.hpp"
#include <deque>
#include <mutex>
#include <thread>

namespace adicontroller {

    class sequence::impl {
    public:
        impl() {
        }
        
        ~impl() {
        }
        
        static std::unique_ptr< sequence > instance_;
        std::mutex mutex_;
        std::deque< sequence::value_type > que_; // SampleProcessors
    };

    std::unique_ptr< sequence > sequence::impl::instance_;    
}

using namespace adicontroller;

sequence::~sequence()
{
}

sequence::sequence() : impl_( new impl() )
{
}

sequence *
sequence::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [](){ sequence::impl::instance_.reset( new sequence() ); } );

    return sequence::impl::instance_.get();
}

sequence::iterator
sequence::begin()
{
    return impl_->que_.begin();
}

sequence::iterator
sequence::end()
{
    return impl_->que_.end();    
}

sequence::const_iterator
sequence::begin() const
{
    return impl_->que_.begin();
}

sequence::const_iterator
sequence::end() const
{
    return impl_->que_.end();        
}

size_t
sequence::size() const
{
    return impl_->que_.size();
}

void
sequence::post( value_type sp )
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    impl_->que_.push_back( sp );
}

sequence::value_type
sequence::deque()
{
    std::lock_guard< std::mutex > lock( impl_->mutex_ );
    
    if ( !impl_->que_.empty() ) {
        auto sp = impl_->que_.front();
        impl_->que_.pop_front();
        return sp;
    }
    return 0;
}

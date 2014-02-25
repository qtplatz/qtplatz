/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "task.hpp"

using namespace batchproc;

task * task::instance_ = 0;
std::mutex task::mutex_;

task::task() : work_( io_service_ )
{
}

task*
task::instance()
{
    if ( instance_ == 0 ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        if ( instance_ == 0 )
            instance_ = new task;
        instance_->open();
    }
    return instance_;
}

bool
task::shutdown()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( instance_ ) {
        instance_->io_service_.stop();
        for ( auto& t: instance_->threads_ )
            t.join();
        return true;
    }
    return false;  // task not instanciated
}

void
task::open()
{
	threads_.push_back( std::thread( [&](){ io_service_.run(); } ) );
	threads_.push_back( std::thread( [&](){ io_service_.run(); } ) );
	threads_.push_back( std::thread( [&](){ io_service_.run(); } ) );
}

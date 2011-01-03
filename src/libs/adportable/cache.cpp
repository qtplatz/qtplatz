// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "cache.h"
#include <acewrapper/mutex.hpp>
#include <ace/Recursive_Thread_Mutex.h>

using namespace adportable;

Cache::~Cache()
{
    delete lock_;
}

Cache::Cache(size_t itemsize, size_t nitems) : item_size_( itemsize )
											 , nitems_(nitems)
											 , pool_(0)
											 , wr_count_(0)
											 , rd_count_(0)
                                             , lock_( new ACE_Recursive_Thread_Mutex() ) 
{
    pool_ = new unsigned long * [ nitems_ ];
    pool_[0] = new unsigned long [ nitems_ * item_size_ ];
    memset( pool_[0], 0, sizeof(long) * nitems_ * item_size_ );
    for ( size_t i = 1; i < nitems_; ++i )
        pool_[i] = pool_[i - 1] + item_size_;
}

void
Cache::dispose()
{
    acewrapper::scoped_mutex_t<> lock(*lock_);
	delete [] pool_[0];
	delete [] pool_;
	pool_ = 0;
}

void
Cache::reset()
{
    acewrapper::scoped_mutex_t<> lock(*lock_);	
	wr_count_ = rd_count_ = 0;
}

unsigned long *
Cache::rd_ptr( unsigned long& npos )
{
	unsigned long count = wr_count_;

	if ( count == 0 )
		return 0;

	if ( npos == (-1) )
		npos = count - 1;

	if ( npos >= count ) // too early
		return 0;

	if ( ( count - npos ) >= nitems_ ) // too late
		return 0;
	
	unsigned char * ptop = reinterpret_cast<unsigned char *>(pool_[ npos % nitems_ ]);
	ctrl_block * pctrl = reinterpret_cast<ctrl_block *>(ptop);

#if defined DEBUG && 0
	//if ( item_size_ < 512 ) 
	std::cerr << "Cache::rd_ptr(" << npos << "), wr_count_ = " << count
			  << " flag = " << pctrl->flag_ << std::endl;
#endif

	if ( ( pctrl->flag_ & (eWriteBusy | eReadBusy) ) == 0 ) {

        acewrapper::scoped_mutex_t<> lock(*lock_);

		if ( ( pctrl->flag_ & (eWriteBusy | eReadBusy) ) == 0 ) { // double check pattern
			pctrl->flag_ = eReadBusy;
			return reinterpret_cast<unsigned long *>(ptop + sizeof(ctrl_block));
		}
	}

	return 0;
}

unsigned long *
Cache::rd_ptr()
{
    if ( wr_count_ == (rd_count_ + 1) )
        return 0;
	
	unsigned char * ptop = reinterpret_cast<unsigned char *>(pool_[ rd_count_ % nitems_ ]);
	ctrl_block * pctrl = reinterpret_cast<ctrl_block *>(ptop);
	
	if ( ( pctrl->flag_ & eWriteBusy ) == 0 ) {
		
        acewrapper::scoped_mutex_t<> lock(*lock_);
		
		if ( ( pctrl->flag_ & eWriteBusy ) == 0 ) {   // double check pattern
			pctrl->flag_ = eReadBusy;
			rd_count_++;
			return reinterpret_cast<unsigned long *>(ptop + sizeof(ctrl_block));
		}
	}
	return 0;
}

unsigned long *
Cache::wr_ptr()
{
    acewrapper::scoped_mutex_t<> lock(*lock_);
	unsigned char * ptop = reinterpret_cast<unsigned char *>(pool_[ wr_count_ % nitems_ ]);
	ctrl_block * pctrl = reinterpret_cast<ctrl_block *>(ptop);
	wr_count_ ++;
	pctrl->flag_ = eWriteBusy;
	return reinterpret_cast<unsigned long *>(ptop + sizeof(ctrl_block));
}

void
Cache::unlock( unsigned long * p )
{
	ctrl_block * pctrl = 
		reinterpret_cast<ctrl_block *>(
			reinterpret_cast<unsigned char *>(p) - sizeof(ctrl_block) );

    acewrapper::scoped_mutex_t<> lock(*lock_);
	if ( pctrl->flag_ == eWriteBusy )
		pctrl->flag_ = eWriteComplete;
	else if ( pctrl->flag_ == eReadBusy )
		pctrl->flag_ = eFree;
}

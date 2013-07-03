/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef DMA_TYPE_HPP
#define DMA_TYPE_HPP

#include <boost/interprocess/sync/interprocess_semaphore.hpp>
#include <boost/array.hpp>
#include "tofdef.hpp" // dma_data_t

namespace tofinterface {

    template< std::size_t N > struct dma {
        boost::interprocess::interprocess_semaphore sema_;
        const uint32_t nsize_;
        uint32_t numwords_;
        boost::array< dma_data_t, N > data_;
        dma() : sema_( 0 ), nsize_( N ) {}
    };

};

#endif // DMA_TYPE_HPP

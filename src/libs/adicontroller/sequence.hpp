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

#pragma once

#include "adicontroller_global.hpp"
#include <deque>
#include <memory>

namespace adicontroller {

    class SampleProcessor;

    namespace internal {  struct receiver_data;  }

    class ADICONTROLLERSHARED_EXPORT sequence {

        sequence( const sequence& ) = delete;
        sequence& operator = ( const sequence& ) = delete;
        
        ~sequence();
        sequence();

    public:
        static sequence * instance();

        typedef std::shared_ptr< SampleProcessor > value_type;
        typedef std::shared_ptr< const SampleProcessor > const_value_type;

        typedef std::deque< value_type >::iterator iterator;
        typedef std::deque< value_type >::const_iterator const_iterator;

        iterator begin();
        iterator end();
        const_iterator begin() const;
        const_iterator end() const;

        size_t size() const;

        void post( value_type ); // <-- preparing_for_run

        value_type deque();      // <-- stop run | reset

    private:
        friend std::unique_ptr< sequence >::deleter_type;
        class impl;
        std::unique_ptr< impl > impl_;
    };

} // namespace adicontroller


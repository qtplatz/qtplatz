// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <vector>
#include <string>

namespace adplot {

    class Trace;
    class plot;

    class Traces {
    public:
        typedef std::vector< Trace > vector_type;

        Traces( plot&, vector_type& );
        Traces( const Traces& );
        size_t size() const;

        Trace add( const std::wstring& title = L"" );
        inline Trace& operator [] ( size_t idx ) { return vec_[ idx ]; }
        void clear();
        inline vector_type::iterator begin() { return vec_.begin(); }
        inline vector_type::iterator end() { return vec_.end(); }
        inline vector_type::const_iterator begin() const { return vec_.begin(); }
        inline vector_type::const_iterator end() const { return vec_.end(); }
    private:
        plot& plot_;
        vector_type& vec_;
    };

}


// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include "dgprotocol.hpp"
#include <vector>

namespace adportable {
    namespace dg {

        class protocols {
        public:
            protocols();
            protocols( const protocols& t );

            bool read_json( std::istream& );

            bool write_json( std::ostream& ) const;
            
            double interval() const;

            void setInterval( double interval );

            const protocol& operator [] ( int idx ) const;
            protocol& operator [] ( int idx );

            const size_t size() const;

            void resize( size_t );

            std::vector< protocol >::const_iterator begin() const { return protocols_.begin(); }
            std::vector< protocol >::const_iterator end() const { return protocols_.end(); }

        private:
            double interval_;
            std::vector< protocol > protocols_;
        };
    }
}

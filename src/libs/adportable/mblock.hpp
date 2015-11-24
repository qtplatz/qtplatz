/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <memory>


namespace adportable {

    template< typename value_type >
    class mblock : public std::enable_shared_from_this< mblock< value_type > > {

        std::unique_ptr< value_type [] > pData_;
        size_t size_;

        mblock( const mblock& ) = delete;

    public:

        mblock( size_t size = 8192 ) : pData_( new value_type[ size ] )
                                     , size_( size ) {
        }
        
        value_type * data() { return pData_.get(); }

        const value_type * data() const { return pData_.get(); }

        static size_t dataType() { return sizeof( value_type ); }
            
    };

}


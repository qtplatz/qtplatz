/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "adcontrols_global.h"
#include <memory>
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

#if defined _MSC_VER
//template class __declspec(dllexport) std::allocator<int>;
template class __declspec( dllexport ) std::basic_string < char > ;
template class __declspec( dllexport ) std::basic_string < wchar_t > ;
#endif

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT moltable {
    public:    
        struct ADCONTROLSSHARED_EXPORT value_type {
            bool enable;
            double mass;
            double abundance;
            std::string formula;
            std::string adducts;
            std::string synonym;
            std::string smiles;
            std::wstring description;
            
            value_type() : enable( true ), mass( 0 ), abundance( 1.0 ) {}
            
            value_type( const value_type& t ) : enable( t.enable )
                , mass( t.mass )
                , abundance( t.abundance )
                , formula( t.formula )
                , adducts( t.adducts )
                , synonym( t.synonym )
                , smiles( t.smiles )
                , description( t.description ) {
            }
        };

        ~moltable();        
        moltable();
        moltable( const moltable& );
        moltable& operator = ( const moltable& );
        
        const std::vector< value_type >& data() const;
        std::vector< value_type >& data();

        moltable& operator << ( const value_type& );
        size_t size() const;
        bool empty() const;
        
    private:
        class delegate;        
        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };
    
}

#if defined _MSC_VER
template class ADCONTROLSSHARED_EXPORT std::vector < adcontrols::moltable::value_type > ;
#endif


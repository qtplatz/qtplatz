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
#include <boost/variant.hpp>
#include <memory>
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT moltable {
    public:
        enum molflags { isMSRef = 0x80000000 };

        typedef boost::variant< bool, uint32_t, double, std::string > custom_type;
        
        class ADCONTROLSSHARED_EXPORT value_type {
            bool enable_;
            uint32_t flags_;
            double mass_;
            double abundance_;
#if defined _MSC_VER
#pragma warning(push)
#pragma warning(disable:4251)
#endif
            std::string formula_;
            std::string adducts_;
            std::string synonym_;
            std::string smiles_;
            std::wstring description_;
            std::vector < std::pair< std::string, custom_type > > customValues_;

#if defined _MSC_VER
#pragma warning(pop)
#endif
        public:
            bool& enable() { return enable_; }
            uint32_t& flags() { return flags_; }
            double& mass() { return mass_; }
            double& abundance() { return abundance_; }
            std::string& formula() { return formula_; }
            std::string& adducts() { return adducts_; }
            std::string& synonym() { return synonym_; }
            std::string& smiles() { return smiles_; }
            std::wstring& description() { return description_; }
            bool enable() const { return enable_; }
            uint32_t flags() const { return flags_; }
            double mass() const { return mass_; }
            double abundance() const { return abundance_; }
            const char * formula() const { return formula_.c_str(); }
            const char * adducts() const { return adducts_.c_str(); }
            const char * synonym() const { return synonym_.c_str(); }
            const char * smiles() const { return smiles_.c_str(); }
            const wchar_t * description() const { return description_.c_str(); }

            std::vector< std::pair< std::string, custom_type > >& customValues();
            const std::vector< std::pair< std::string, custom_type > >& customValues() const;
            
            value_type() : enable_( true ), flags_(0), mass_( 0 ), abundance_( 1.0 ) {}
            
            value_type( const value_type& t ) : enable_( t.enable_ )
                , flags_( t.flags_ )
                , mass_( t.mass_ )
                , abundance_( t.abundance_ )
                , formula_( t.formula_ )
                , adducts_( t.adducts_ )
                , synonym_( t.synonym_ )
                , smiles_( t.smiles_ )
                , description_( t.description_ )
                , customValues_( t.customValues_ ) {
            }
            bool isMSRef() const;
            void setIsMSRef( bool on );
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

        static bool xml_archive( std::wostream&, const moltable& );
        static bool xml_restore( std::wistream&, moltable& );
        
    private:
        class delegate;        
        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

#if defined _MSC_VER
    ADCONTROLSSHARED_TEMPLATE_EXPORT template class ADCONTROLSSHARED_EXPORT std::vector < adcontrols::moltable::value_type > ;
#endif    

}




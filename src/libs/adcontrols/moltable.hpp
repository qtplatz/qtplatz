/**************************************************************************
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "constants_fwd.hpp"
#include <boost/serialization/version.hpp>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

namespace boost { namespace serialization { class access; } }

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT moltable;

    class moltable {
    public:
        enum molflags { isMSRef = 0x80000000 };

        class ADCONTROLSSHARED_EXPORT value_type;

        class value_type {
        public:
            bool enable_;
            uint32_t flags_;
            double mass_;
            double abundance_;
            std::string formula_;
            std::tuple<std::string, std::string>  adducts_;
            std::string synonym_;
            std::string smiles_;
            std::wstring description_;
            boost::optional< int32_t > protocol_; // data source for mass chromatogram generation
            boost::optional< double > tR_; // data source for mass chromatogram generation
            boost::optional< boost::uuids::uuid > molid_; // used in quan
            uint32_t resv_;
        public:
            bool& enable()                     { return enable_; }
            uint32_t& flags()                  { return flags_; }
            double& mass()                     { return mass_; }
            double& abundance()                { return abundance_; }
            std::string& formula()             { return formula_; }
            std::tuple< std::string, std::string >& adducts()               { return adducts_; }
            const std::tuple< std::string, std::string >& adducts() const   { return adducts_; }

            template< ion_polarity pol > std::string& adducts()             { return std::get< pol >( adducts_ ); };
            template< ion_polarity pol > const std::string& adducts() const { return std::get< pol >( adducts_ ); };
            std::string& adducts( ion_polarity );
            const std::string& adducts( ion_polarity ) const;
            std::string& synonym()             { return synonym_; }
            std::string& smiles()              { return smiles_; }
            std::wstring& description()        { return description_; }
            bool enable() const                { return enable_; }
            uint32_t flags() const             { return flags_; }
            double mass() const                { return mass_; }
            double abundance() const           { return abundance_; }
            const std::string& formula() const { return formula_; }

            const std::string& synonym() const { return synonym_; }
            const std::string& smiles() const  { return smiles_; }
            const std::wstring& description() const { return description_; }

            bool isMSRef() const;
            void setIsMSRef( bool on );

            bool operator == ( const value_type& ) const;

            boost::optional< int32_t > protocol() const;
            void setProtocol( boost::optional< int32_t >&& proto );

            boost::optional< double > tR() const;
            void set_tR( boost::optional< double >&& );

            boost::optional< boost::uuids::uuid > molid() const;
            void setMolid( boost::optional< boost::uuids::uuid >&& );

            value_type();
            value_type( const value_type& t );
        };

        ~moltable();
        moltable();
        moltable( const moltable& );
        moltable& operator = ( const moltable& );
        moltable& operator += ( const moltable& );

        const std::vector< value_type >& data() const;
        std::vector< value_type >& data();

        const ion_polarity& polarity() const;
        ion_polarity& polarity();
        void setPolarity( ion_polarity );

        moltable& operator << ( const value_type& );
        size_t size() const;
        bool empty() const;

        static bool xml_archive( std::wostream&, const moltable& );
        static bool xml_restore( std::wistream&, moltable& );

    private:
        class impl;
        impl * impl_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const moltable::value_type& );

    ADCONTROLSSHARED_EXPORT
    moltable::value_type tag_invoke( boost::json::value_to_tag< moltable::value_type >&, const boost::json::value& jv );

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const moltable& );

    ADCONTROLSSHARED_EXPORT
    moltable tag_invoke( boost::json::value_to_tag< moltable >&, const boost::json::value& jv );

#if defined _MSC_VER
    ADCONTROLSSHARED_TEMPLATE_EXPORT template class ADCONTROLSSHARED_EXPORT std::vector < adcontrols::moltable::value_type > ;
#endif

}

BOOST_CLASS_VERSION( adcontrols::moltable::value_type, 4 )

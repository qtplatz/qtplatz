/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#ifndef QUANMETHODCOMPLEX_HPP
#define QUANMETHODCOMPLEX_HPP

#include <adcontrols/idaudit.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/variant.hpp>
#include <cstdint>
#include <memory>

namespace adcontrols { class QuanMethod; class QuanCompounds; class ProcessMethod; }
namespace adpublisher { class document; }

namespace quan {

    class QuanMethodComplex  {
    public:
        ~QuanMethodComplex();
        QuanMethodComplex();
        QuanMethodComplex( const QuanMethodComplex& );

        static const wchar_t * dataClass() { return L"QuanMethodComplex"; }

        std::shared_ptr< adcontrols::QuanMethod > quanMethod();
        std::shared_ptr< adcontrols::ProcessMethod > procMethod();
        std::shared_ptr< adcontrols::QuanCompounds > quanCompounds();
        std::shared_ptr< adpublisher::document > docTemplate();

        void operator = ( std::shared_ptr< adcontrols::QuanMethod >& );
        void operator = ( std::shared_ptr< adcontrols::ProcessMethod >& );
        void operator = ( std::shared_ptr< adcontrols::QuanCompounds > & );
        void operator = ( std::shared_ptr< adpublisher::document > & );

        const adcontrols::idAudit& ident() const { return ident_; }
        const wchar_t * filename() const;
        void setFilename( const wchar_t * );

        static bool restore( std::istream&, QuanMethodComplex& );
        static bool archive( std::ostream&, const QuanMethodComplex& );

    private:
        adcontrols::idAudit ident_;
        std::shared_ptr< adcontrols::QuanMethod > quanMethod_;
        std::shared_ptr< adcontrols::QuanCompounds > quanCompounds_;
        std::shared_ptr< adcontrols::ProcessMethod > procMethod_;
        std::shared_ptr< adpublisher::document > docTemplate_;

        friend class boost::serialization::access;
        template<class Archive>
        void serialize( Archive& ar, const unsigned int ) {
            using namespace boost::serialization;

            std::string docTemplate;
            if ( Archive::is_saving::value )
                docTemplate_->save( docTemplate );

            ar & BOOST_SERIALIZATION_NVP( ident_ );
            ar & boost::serialization::make_nvp( "quanMethod", *quanMethod_ );
            ar & boost::serialization::make_nvp( "quanCompounds", *quanCompounds_ );
            ar & boost::serialization::make_nvp( "procMethod", *procMethod_ );
            ar & BOOST_SERIALIZATION_NVP( docTemplate );

            if ( Archive::is_loading::value )
                docTemplate_->load( docTemplate.c_str() );
        }
    };

}

#endif // QUANMETHODCOMPLEX_HPP

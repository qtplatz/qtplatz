/**************************************************************************

** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <boost/optional.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_from.hpp>
#include <boost/json/value_to.hpp>
#include <string>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT annotation;

    class annotation {
    public:
        enum DataFormat {
            dataText
            , dataSvg
            , dataFormula
            , dataSmiles
            , dataMOL
            , dataJSON
        };

        enum DataFlag {
            flag_auto                = 0
            , flag_centroid          = 0x00000001
            , flag_targeting         = 0x00000002
            , flag_manually_assigned = 0x80000000
        };

        struct peak {
            int mode;
            double mass;
            peak() : mode(0), mass(0) {}
        };

        ~annotation();
        annotation();
        annotation( const annotation& );
        annotation( const std::wstring&, double x = 0, double y = 0, int id = (-1), int priority = 0, DataFormat fmt = dataText, DataFlag flag = flag_auto );
        annotation( const std::string&, double x = 0, double y = 0, int id = (-1), int priority = 0, DataFormat fmt = dataText, DataFlag flag = flag_auto );
        annotation( boost::json::object&&, double x = 0, double y = 0, int id = (-1), int priority = 0, DataFlag flag = flag_auto );

        const std::string& text() const;
        void text( const std::wstring& text, DataFormat f = dataText );
        void text( const std::string& text, DataFormat f = dataText );
        // void setJson( std::string&& );
        boost::optional< std::string > json() const;

        int index() const;
        void index( int );

        DataFormat dataFormat() const;
        void dataFormat( DataFormat );

        int priority() const;
        void priority( int );

        uint32_t flags() const;
        void setFlags( uint32_t );

        double x() const;
        double y() const;
        double width() const;
        double height() const;
        void rect( double x, double y, double width = 0, double height = 0 );
        void x( double );
        void y( double );

    private:
        DataFormat format_;
        int index_;
        int priority_;
#if defined _MSC_VER
# pragma warning( disable: 4251 )
#endif
        std::string text_;

        double x_, y_;
        double w_, h_;

        uint32_t flags_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize( Archive& ar, const unsigned int version ) {
            ar & BOOST_SERIALIZATION_NVP( format_ );
            ar & BOOST_SERIALIZATION_NVP( index_ );
            ar & BOOST_SERIALIZATION_NVP( priority_ );
            if ( version < 2 ) {
                std::wstring tmp;
                ar & BOOST_SERIALIZATION_NVP( tmp );
                text( tmp );
            } else {
                ar & BOOST_SERIALIZATION_NVP( text_ );
            }
            ar & BOOST_SERIALIZATION_NVP( x_ );
            ar & BOOST_SERIALIZATION_NVP( y_ );
            ar & BOOST_SERIALIZATION_NVP( w_ );
            ar & BOOST_SERIALIZATION_NVP( h_ );
            if ( version >= 3 ) {
                ar & BOOST_SERIALIZATION_NVP( flags_ );
            }
        }
    };

    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const annotation::peak& );

    ADCONTROLSSHARED_EXPORT
    annotation::peak tag_invoke( boost::json::value_to_tag< annotation::peak >&, const boost::json::value& jv );
}

BOOST_CLASS_VERSION( adcontrols::annotation, 3 )

// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include "../adcontrols_global.h"
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <string>
#include <map>
#include <vector>

namespace boost {
    namespace serialization { class access; }
    namespace uuids { struct uuid; }
}

namespace adcontrols {

    namespace constants {
        enum ion_polarity : unsigned int;
    }

    namespace xic {
        enum eIntensityAlgorishm : uint32_t;

        struct ADCONTROLSSHARED_EXPORT xic_method {
            std::tuple< bool, std::string, std::string, std::string > mol_; // enable, synonym, formula, smiles
            std::tuple< std::string, std::string >  adduct_; // pos, neg
            std::pair< double, double >  mass_window_;
            std::pair< double, double >  time_window_;
            eIntensityAlgorishm algo_;
            int protocol_;

            xic_method();
            xic_method( const xic_method& );

            bool enable() const                     { return std::get< 0 >( mol_ ); }
            const std::string& synonym() const      { return std::get< 1 >( mol_ ); }
            const std::string& formula() const      { return std::get< 2 >( mol_ ); }

            void enable( bool enable )              { std::get< 0 >( mol_ ) = enable; }
            void synonym( const std::string& t )    { std::get< 1 >( mol_ ) = t; }
            void formula( std::string& t )          { std::get< 2 >( mol_ ) = t; }
            template< constants::ion_polarity pol > const std::string& adduct()              { return std::get< pol >( adduct_ ); }
            template< constants::ion_polarity pol > void adduct( const std::string& adduct ) { std::get< pol >( adduct_ ) = adduct; }
            double mass() const                     { return std::get< 0 >( mass_window_ ); }
            double mass_window() const              { return std::get< 1 >( mass_window_ ); }
            void mass( double t )                   { std::get< 0 >( mass_window_ ) = t; }
            void mass_window( double t )            { std::get< 1 >( mass_window_ ) = t; }
            double time() const                     { return std::get< 0 >( time_window_ ); }
            double time_window() const              { return std::get< 1 >( time_window_ ); }
            void time( double t )                   { std::get< 0 >( time_window_ ) = t; }
            void time_window( double t )            { std::get< 1 >( time_window_ ) = t; }
            eIntensityAlgorishm algo() const        { return algo_; }
            void algo( eIntensityAlgorishm t )      { algo_ = t; }
            int protocol() const                    { return protocol_; }
            void protocol( int t )                  {  protocol_ = t; }
        };
    }

    class ADCONTROLSSHARED_EXPORT XChromatogramsMethod {
    public:
        ~XChromatogramsMethod();
        XChromatogramsMethod();
        XChromatogramsMethod( const XChromatogramsMethod& );

        static const char * modelClass() { return "TofChromatograms"; }
        static const char * itemLabel()  { return "Chromatograms.1"; }
        static const boost::uuids::uuid& clsid();

        size_t size() const;
        void clear();

        XChromatogramsMethod& operator << ( xic::xic_method&& );

        constants::ion_polarity polarity() const;
        void setPolarity( constants::ion_polarity );

        size_t numberOfTriggers() const;
        void setNumberOfTriggers( size_t );

        bool refreshHistogram() const;
        void setRefreshHistogram( bool );

        std::tuple< bool, xic::eIntensityAlgorishm > tic() const;
        void setTIC( std::tuple< bool, xic::eIntensityAlgorishm >&& );

        static bool archive( std::ostream&, const XChromatogramsMethod& );
        static bool restore( std::istream&, XChromatogramsMethod& );
        static bool xml_archive( std::wostream&, const XChromatogramsMethod& );
        static bool xml_restore( std::wistream&, XChromatogramsMethod& );

    private:
        class impl;
        std::unique_ptr< impl > impl_;

        typedef XChromatogramsMethod X;
        friend ADCONTROLSSHARED_EXPORT void tag_invoke( boost::json::value_from_tag, boost::json::value&, const X& );
        friend ADCONTROLSSHARED_EXPORT X tag_invoke( boost::json::value_to_tag< X >&, const boost::json::value& jv );
    };

    // xic::xic_method
    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const xic::xic_method& );

    ADCONTROLSSHARED_EXPORT
    xic::xic_method tag_invoke( boost::json::value_to_tag< xic::xic_method >&, const boost::json::value& jv );

    // XChromatogramsmethod
    ADCONTROLSSHARED_EXPORT
    void tag_invoke( boost::json::value_from_tag, boost::json::value&, const XChromatogramsMethod& );

    ADCONTROLSSHARED_EXPORT
    XChromatogramsMethod tag_invoke( boost::json::value_to_tag< XChromatogramsMethod >&, const boost::json::value& jv );
}

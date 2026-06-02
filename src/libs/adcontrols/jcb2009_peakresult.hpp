/**************************************************************************
** Copyright (C) 2010-2026 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2026 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include "adcontrols_global.h"
#include <adcontrols/mspeakinfo.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#include <boost/uuid/uuid.hpp>
#include <memory>
#include <optional>

namespace portfolio {
    class Folium;
}

namespace adcontrols {
    class Peak;
    class MSPeakInfoItem;

    class ADCONTROLSSHARED_EXPORT jcb2009_peakresult;

    namespace jcb2009 {
        // dataSource attach to the MassSpectrum
        struct Peak {
            Peak();
            Peak( const Peak& );
            Peak( int32_t id, double tR, double st, double en );
            int32_t pkid_;
            double  peakTime_; // tR (s)
            double peakStart_;
            double peakEnd_;
            friend ADCONTROLSSHARED_EXPORT void
            tag_invoke( const boost::json::value_from_tag, boost::json::value&, const Peak& );
            friend ADCONTROLSSHARED_EXPORT Peak
            tag_invoke( const boost::json::value_to_tag< Peak >&, const boost::json::value& );
        };

        class dataSource {
        public:
            ~dataSource();
            dataSource();
            dataSource( const dataSource& );
            dataSource( std::pair< std::string, boost::uuids::uuid >&& foliumGuid
                        , const boost::uuids::uuid& dataGuid
                        , const adcontrols::Peak& pk
                        , const std::optional< std::string >& formula
                        , const std::string& adduct
                        , double mass
                        , double mass_width
                        , int32_t protocol = (-1));
            const dataSource& operator=( const dataSource& );
            const std::pair< std::string, boost::uuids::uuid >& folium() const;
            const boost::uuids::uuid& dataGuid() const;
            double mass() const;
            double mass_width() const;
            int32_t protocol() const;
            std::string formula() const;
            std::string adduct() const;
            const Peak& pk() const;
        private:
            class impl;
            std::unique_ptr< impl > impl_;
            friend ADCONTROLSSHARED_EXPORT void
            tag_invoke( const boost::json::value_from_tag, boost::json::value&, const dataSource& );
            friend ADCONTROLSSHARED_EXPORT dataSource
            tag_invoke( const boost::json::value_to_tag< dataSource >&, const boost::json::value& );
        };
    }

    class jcb2009_peakresult {
    public:
        jcb2009_peakresult();
        jcb2009_peakresult( const jcb2009_peakresult& );

        jcb2009_peakresult( double   // generator.mass
                            , double // generator.mass_width
                            , int    // protocol
                            , const adcontrols::Peak& peak
                            , std::pair< std::string, boost::uuids::uuid >&& folder
                            , const boost::uuids::uuid& dataGuid );

        void set_found_mass( const adcontrols::MSPeakInfoItem& );
        double tR() const;
        double peak_width() const;
        double peak_height() const;
        double peak_area() const;
        const std::string& peak_name() const;
        double generator_mass() const;
        double generator_mass_width() const;
        double matched_mass() const;
        double matched_mass_width() const;
        double matched_mass_height() const;
        int protocol() const;
        boost::uuids::uuid dataGuid() const;

        bool operator < ( const jcb2009_peakresult& ) const;
    private:
        double chro_tR_;
        double chro_peak_width_;
        double chro_peak_height_;
        double chro_peak_area_;
        std::string chro_peak_name_;
        double chro_generator_mass_;
        double chro_generator_mass_width_;
        double matched_mass_;
        double matched_mass_width_;
        double matched_mass_height_;
        int protocol_;
        std::pair< std::string, boost::uuids::uuid > dataSource_;
        boost::uuids::uuid dataGuid_;
        ssize_t pkid_;

        friend ADCONTROLSSHARED_EXPORT void
        tag_invoke( const boost::json::value_from_tag, boost::json::value&, const jcb2009_peakresult& );

        friend ADCONTROLSSHARED_EXPORT jcb2009_peakresult
        tag_invoke( const boost::json::value_to_tag< jcb2009_peakresult >&, const boost::json::value& );
    };

}

// This is a -*- C++ -*- header.
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
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/optional.hpp>
#include <boost/json/fwd.hpp>
#include <boost/json/value_to.hpp>
#if __cplusplus >= 201703L
#include <optional>
#endif
#include <cstdint>
#include <functional>
#include <string>

namespace adportable { namespace waveform_peakfinder { struct peakinfo; } }

namespace adcontrols {

    namespace internal { class CentroidProcessImpl; }

    class ADCONTROLSSHARED_EXPORT MSPeakInfoItem;

    template< typename Archive > ADCONTROLSSHARED_EXPORT void serialize(Archive & ar, MSPeakInfoItem&, const unsigned int version);

    class MSPeakInfoItem {
    public:
        ~MSPeakInfoItem(void);
        MSPeakInfoItem(void);
        MSPeakInfoItem( const MSPeakInfoItem& );

        MSPeakInfoItem( const adportable::waveform_peakfinder::peakinfo&, size_t idx, double dbase
                        , bool isTime = true
                        , std::function<double( double )> mass_assignee = std::function<double( double )>() );

        double mass() const;

        void assign_mass( double );     // this will change centroid left/right values (for lock mass)
        void set_mass( double mass, double left, double right );

        double area() const;
        void set_area( double );
        //void setArea( double );

        double height() const;
        void set_height( double );
        double time( bool from_time = false ) const;
        void set_time( double, double left, double right, bool from_time = false );
        unsigned int peak_index() const;
        void set_peak_index( unsigned int );
        unsigned int peak_start_index() const;
        void set_peak_start_index( unsigned int );
        unsigned int peak_end_index() const;
        void set_peak_end_index( unsigned int );
        double base_height() const;
        void set_base_height( double );

        double centroid_left( bool time = false ) const;
        void set_centroid_left( double t, bool time );

        double centroid_right( bool time = false ) const;
        void set_centroid_right( double t, bool time );

        double centroid_threshold() const;
        void set_centroid_threshold( double );

        void set_width_hh_lr( double left, double right, bool time = false );

        double widthHH( bool time = false ) const;
        double hh_left_time() const;
        double hh_right_time() const;

        const std::string& formula() const;
        void formula( const std::string& );
        const std::string& annotation() const;
        void annotation( const std::string& );
        void annotation( const std::wstring& );
        bool visible() const;
        void visible( bool );
        bool is_reference() const;
        void is_reference( bool );

        boost::optional<int> mode() const;
        void set_mode( boost::optional<int>&& );

        static bool xml_archive( std::wostream&, const MSPeakInfoItem& );
        static bool xml_restore( std::wistream&, MSPeakInfoItem& );
        static boost::optional< MSPeakInfoItem > fromJson( const std::string& json );
        static boost::optional< MSPeakInfoItem > fromJson( const boost::json::value& );
        std::string toJson() const;

        static void tag_invoke( boost::json::value& jv, const MSPeakInfoItem& );
        static MSPeakInfoItem tag_invoke( const boost::json::value& jv );

    private:
        uint32_t peak_index_;
        uint32_t peak_start_index_;
        uint32_t peak_end_index_;
        double base_height_;
        double mass_;
        double area_;
        double height_;

        double time_from_mass_;
        double time_from_time_;
        double HH_left_mass_;
        double HH_right_mass_;
        double HH_left_time_;
        double HH_right_time_;

        double centroid_left_mass_;
        double centroid_right_mass_;
        double centroid_left_time_;
        double centroid_right_time_;
        double centroid_threshold_; // absolute hight (for graphical rep)

        bool is_visible_;
        bool is_reference_;
        std::string formula_;
        std::string annotation_;
        boost::optional< int32_t > mode_; // use boost's optional for serialization

        template< typename T > class archiver;
        friend class archiver< MSPeakInfoItem >;
        friend class archiver< const MSPeakInfoItem >;

        friend class internal::CentroidProcessImpl;
        template< typename Archive > ADCONTROLSSHARED_EXPORT
        friend void serialize(Archive & ar, MSPeakInfoItem&, const unsigned int version);
    };


    ADCONTROLSSHARED_EXPORT void tag_invoke( const boost::json::value_from_tag
                                             , boost::json::value&, const adcontrols::MSPeakInfoItem& );
    ADCONTROLSSHARED_EXPORT adcontrols::MSPeakInfoItem tag_invoke( const boost::json::value_to_tag< adcontrols::MSPeakInfoItem>&
                                                                   , const boost::json::value& jv );
}


BOOST_CLASS_VERSION( adcontrols::MSPeakInfoItem, 4 )

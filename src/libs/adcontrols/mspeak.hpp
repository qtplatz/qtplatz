/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSPEAK_HPP
#define MSPEAK_HPP

#include "adcontrols_global.h"
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include <cstdint>
#include <string>
#include <compiler/disable_dll_interface.h>

namespace adcontrols {

    class ADCONTROLSSHARED_EXPORT MSPeak {
    public:
        ~MSPeak();
        MSPeak();
        MSPeak( const MSPeak& );
        MSPeak( double time, double mass, int32_t mode, double flength = 1.0 );
        MSPeak( const std::string& formula, double mass );
        static const wchar_t * dataClass() { return L"adcontrols::MSPeak"; }

        double time() const;
        double mass() const;
        int32_t mode() const;
        int32_t fcn() const;
        double flight_length() const;
        double width( bool isTime = false ) const;
        double exit_delay() const;
        const std::string& formula() const;
        const std::wstring& description() const;
        const std::string& spectrumId() const;
        int32_t spectrumIndex() const;
        double exact_mass() const;

        void time( double );
        void mass( double );
        void mode( int32_t );
        void fcn( int32_t );
        void width( double, bool isTime = false );
        void exit_delay( double );
        void flight_length( double );
        void formula( const std::string& );
        void description( const std::wstring& );
        void spectrumId( const std::string& ); // uuid for a spectrum
        void spectrumIndex( int );

    private:
        double time_;
        double mass_;
        int32_t mode_;  // corresponding to flight length
        int32_t fcn_;   // protocol id
        double flength_;
        std::string formula_;
        std::wstring description_;
        std::string spectrumId_;
        int32_t spectrumIndex_;
        double time_width_;
        double mass_width_;
        double exit_delay_;
        double exact_mass_;

        friend class boost::serialization::access;
        template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
            ar & time_
                & mass_
                & mode_
                & flength_
                & formula_
                & description_
                & spectrumId_
                & spectrumIndex_
                & time_width_
                & mass_width_
                ;
            if ( version >= 1 ) {
                ar & fcn_;
                ar & exit_delay_;
                ar & exact_mass_;
            }
        }
    };

}

BOOST_CLASS_VERSION( adcontrols::MSPeak, 1)

#endif // MSPEAK_HPP

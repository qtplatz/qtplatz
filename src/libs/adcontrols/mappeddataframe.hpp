/**************************************************************************
** Copyright (C) 2014-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2014-2015 MS-Cheminformatics LLC
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
#include <boost/serialization/version.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/uuid/uuid.hpp>
#include <vector>
#include <cstdint>
#include <memory>

namespace boost { namespace serialization { class access; } }
namespace adcontrols { class idAudit; }

namespace adcontrols {

    class idAudit;
    class MappedSpectrum;

    /** \class dataFrame
     * 
     * \brief Raw data frame, which is directry sent from malpix.
     * 
     * This structure directry reflects a data stream from MALPIX User FPGA configuration
     * so that it will be changed with respect to MALPIX project progress.
     */
        
    class ADCONTROLSSHARED_EXPORT MappedDataFrame {
    public:
        ~MappedDataFrame( void );
        MappedDataFrame( void );
        MappedDataFrame( const MappedDataFrame & );
            
        size_t size1() const;
        size_t size2() const;

        uint16_t& operator () ( size_t i, size_t j );

        const uint16_t& operator () ( size_t i, size_t j ) const;

        double time( size_t i, size_t j ) const; // seconds

        static double time( uint16_t, uint32_t delay = 0, double clock = 50.0e6 );

        /** set data sent from FPGA
         * \param uint32_t * data array
         * \param data array size in words
         */
        void setData( const uint16_t *, size_t );

        operator const boost::numeric::ublas::matrix< uint16_t >& () const;
        const boost::numeric::ublas::matrix< uint16_t >& matrix() const;
        boost::numeric::ublas::matrix< uint16_t >& matrix();

        uint64_t& timeSinceEpoch();
        uint64_t timeSinceEpoch() const;

        uint32_t trig_number() const;
        uint32_t& trig_number();

        uint64_t trig_timepoint() const;
        uint64_t& trig_timepoint();

        uint32_t coaddSpectrum( adcontrols::MappedSpectrum&, size_t x0 = 0, size_t y0 = 0, size_t w = ( -1 ), size_t h = ( -1 ) ) const;
        bool transform( adcontrols::MappedSpectrum&, size_t x0 = 0, size_t y0 = 0, size_t w = ( -1 ), size_t h = ( -1 ) ) const;

        double trig_delay() const;
        double& trig_delay();
        
        double samplingInterval() const;
        double& samplingInterval();

        void setDataReaderUuid( const boost::uuids::uuid& );
        int64_t rowd() const;
        int64_t& rowd();

        uint32_t numSamples() const;
        uint32_t& numSamples();

        int64_t rowid() const;
        int64_t& rowid();

        bool empty() const;

    private:
        boost::uuids::uuid dataReaderUuid_;
        boost::numeric::ublas::matrix< uint16_t > data_;
        int64_t  rowId_;
        uint64_t timeSinceEpoch_;
        uint64_t trig_timepoint_;
        uint32_t trig_number_;
        double   trig_delay_;
        double   sampInterval_;
        uint32_t numSamples_; // corresponding to time width
        uint32_t wellKnownEvents_;
        uint32_t clock_count_;
        std::shared_ptr< idAudit > ident_;

        friend class boost::serialization::access;
        template<class Archive> void serialize( Archive& ar, const unsigned int version );
    };

}

BOOST_CLASS_VERSION( adcontrols::MappedDataFrame, 1 )

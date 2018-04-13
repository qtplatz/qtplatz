/**************************************************************************
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
** Author: Toshinobu Hondo, Ph.D.
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "pkd_counting_data_writer.hpp"
#include "waveform_accessor.hpp"
#include <acqrscontrols/u5303a/waveform.hpp>
#include <adcontrols/threshold_method.hpp>  // for AcqirisPKD
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>

namespace acqrscontrols {

    pkd_counting_data_writer::pkd_counting_data_writer( std::shared_ptr< acqrscontrols::waveform_accessor_< acqrscontrols::u5303a::waveform > > a )
        : DataWriter( a )
    {
    }

    // This should be called from SampleProcessor::write within a loop of do { } while ( writer.next() )
    // so that here we should write data only for the current single waveform
    bool
    pkd_counting_data_writer::write( adfs::filesystem& fs ) const
    {
        if ( auto accessor = dynamic_cast< acqrscontrols::waveform_accessor_< acqrscontrols::u5303a::waveform > * >( accessor_.get() ) ) {

            if ( auto waveform = accessor->data() ) {

                if ( waveform->meta_.channelMode == acqrscontrols::u5303a::PKD ) {
                    write( fs, *waveform );
                    return true;
                } else {
                    ADDEBUG() << "Not a PKD waveform";
                }
            }
        } else {
            ADDEBUG() << "pkd_counting_data_writer::write failed to get waveform_accessor " << typeid( accessor_.get() ).name();
        }
        return false;
    }

    bool
    pkd_counting_data_writer::write( adfs::filesystem& fs, const acqrscontrols::u5303a::waveform& w ) const
    {
        static uint64_t tp;

        // todo: investigate how to determines serialnumber -- this is step of two (maybe due to avg and pkd was separately counted)

#ifndef NDEBUG
        ADDEBUG() << "found PKD waveform " << w.serialnumber_ << ", " << w.timeSinceEpoch_ << ", " << double((w.timeSinceEpoch_ - tp)/1000000)
                  << ", pkd : " << w.method_._device_method().pkd_raising_delta
                  << ", " << w.method_._device_method().pkd_falling_delta
                  << ", navg: " << w.method_._device_method().nbr_of_averages;
#endif
        
        tp = w.timeSinceEpoch_;

        do {
            adfs::stmt sql( fs.db() );

            // create table code is located 'adacquire::document::prepareStorage'

            sql.prepare( "INSERT INTO trigger ( id,protocol,timeSinceEpoch,elapsedTime,events,algo"
                         ",nbr_of_trigs,raising_delta,falling_delta,front_end_range )"
                         " VALUES (?,?,?,?,?,?,?,?,?,?)" );
            int id(1);
            sql.bind( id++ ) = w.serialnumber_;
            sql.bind( id++ ) = w.method_.protocolIndex();
            sql.bind( id++ ) = w.timeSinceEpoch_;
            sql.bind( id++ ) = w.meta_.initialXTimeSeconds;
            sql.bind( id++ ) = w.wellKnownEvents_;
            sql.bind( id++ ) = int( adcontrols::threshold_method::AcqirisPKD ); // int( rp->algo() );
            sql.bind( id++ ) = w.method_._device_method().nbr_of_averages;
            sql.bind( id++ ) = w.method_._device_method().pkd_raising_delta;
            sql.bind( id++ ) = w.method_._device_method().pkd_falling_delta;
            sql.bind( id++ ) = w.method_._device_method().front_end_range;
            if ( sql.step() != adfs::sqlite_done ) {
                ADDEBUG() << "sql error";
                return true;
            }
        } while ( 0 );

        do {
            adfs::stmt sql( fs.db() );

            if ( w.meta_.dataType != 4 )
                return true; // true for avoid raw data write to file

            size_t idx( 0 );
            for ( auto it = w.begin< int32_t >(); it != w.end< int32_t >(); ++it ) {
                if ( *it ) {
                    double time = w.time( idx );
                    size_t count = *it;

                    // ADDEBUG() << "histogram: idx = " << idx << " count = " << count << " time = " << time;

                    sql.prepare( "INSERT INTO peak"
                                 " (idTrigger,peak_time,peak_counts) VALUES (?,?,?)" );
                    sql.bind( 1 ) = w.serialnumber_;                                   // idTrigger
                    sql.bind( 2 ) = time;
                    sql.bind( 3 ) = count;
                    if ( sql.step() != adfs::sqlite_done ) {
                        ADDEBUG() << "sql error";
                        return true;                                
                    }
                }
                idx++;
            }
            
        } while ( 0 );

        return true;
    }

    // static
    bool
    pkd_counting_data_writer::prepare_storage( adfs::filesystem& fs )
    {
        adfs::stmt sql( fs.db() );
        
        sql.exec(
            "CREATE TABLE trigger ("
            " id INTEGER PRIMARY KEY"
            ", protocol INTEGER"
            ", timeSinceEpoch INTEGER"
            ", elapsedTime REAL"
            ", events INTEGER"
            ", threshold REAL"
            ", algo INTEGER"
            ", nbr_of_trigs INTEGER"
            ", raising_delta INTEGER"
            ", falling_delta INTEGER"
            ", front_end_range REAL"
            " )" );
        sql.exec(
            "CREATE TABLE peak ("
            " idTrigger INTEGER"
            ", peak_time REAL"
            ", peak_counts INTEGER"
            ", FOREIGN KEY( idTrigger ) REFERENCES trigger( id ))" );

        return true;
    }
}

/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "counting_data_writer.hpp"
#include "threshold_result_accessor.hpp"
#include <acqrscontrols/constants.hpp>
#include <acqrscontrols/u5303a/waveform.hpp>
#include <acqrscontrols/ap240/waveform.hpp>
#include <adlog/logger.hpp>
#if ! defined NDEBUG
# include <adportable/debug.hpp>
#endif
#include <adfs/filesystem.hpp>
#include <adfs/sqlite.hpp>

namespace acqrscontrols {

    // static
    bool
    counting_data_writer::prepare_storage( adfs::filesystem& fs )
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
            ", algo INTEGER )" );

        sql.exec(
            "CREATE TABLE peak ("
            " idTrigger INTEGER"
            ", peak_time REAL"
            ", peak_intensity REAL"
            ", front_offset INTEGER"
            ", front_intensity REAL"
            ", back_offset INTEGER"
            ", back_intensity REAL"
            ", FOREIGN KEY( idTrigger ) REFERENCES trigger( id ))" );

        return true;
    }

    counting_data_writer::counting_data_writer( std::shared_ptr< threshold_result_accessor >&& a ) : DataWriter( a )
    {
    }

    counting_data_writer::~counting_data_writer()
    {
    }

    bool
    counting_data_writer::write( adfs::filesystem& fs, const boost::uuids::uuid& objId ) const
    {
        if ( acqrscontrols::u5303a::timecount_observer != objId ) {
#ifndef NDEBUG
            ADDEBUG() << "############### objid does not match !!!! INTERNAL ERROR ###################";
#endif
        }

        if ( auto accessor = dynamic_cast< threshold_result_accessor * >( accessor_.get() ) ) {
            if ( auto rp = accessor->data() ) { // std::shared_ptr< const acqrscontrols::u5303a::threshold_result >

                auto wp = rp->data();  // waveform
#if ! defined NDEBUG
                // -----> debug
                uint32_t tid ( 0 );
                adfs::stmt sql2( fs.db() );
                sql2.prepare( "SELECT MAX(id),timeSinceEpoch FROM trigger" );
                if ( sql2.step() == adfs::sqlite_row )
                    tid = sql2.get_column_value< uint64_t >( 0 );
#endif
                // <----

                do {
                    adfs::stmt sql( fs.db() );

                    sql.prepare( "INSERT INTO trigger ( id,protocol,timeSinceEpoch,elapsedTime,events,threshold,algo ) VALUES (?,?,?,?,?,?,?)" );
                    int id(1);
                    sql.bind( id++ ) = wp->serialnumber_;
                    sql.bind( id++ ) = wp->method_.protocolIndex();
                    sql.bind( id++ ) = wp->timeSinceEpoch_;
                    sql.bind( id++ ) = wp->meta_.initialXTimeSeconds;
                    sql.bind( id++ ) = wp->wellKnownEvents_;
                    sql.bind( id++ ) = rp->threshold_level();  // V
                    sql.bind( id++ ) = int( rp->algo() );
                    if ( sql.step() != adfs::sqlite_done ) {
                        if ( sql.extended_errcode() == 1555 ) {
                            ADERROR() << "sql error extended code: " << sql.extended_errcode() << "\t" << sql.errmsg()
                                      << "\tpos=" << wp->serialnumber_
#if ! defined NDEBUG
                                      << ", max.id=" << tid
#endif
                                      << "\tmyId=" << this->myId();
                        } else {
                            ADERROR() << "sql error extended code: " << sql.extended_errcode() << "\t" << sql.errmsg();
                        }
                        return true;
                    } else {
#if ! defined NDEBUG
                        ADTRACE() << "ok: pos=" << wp->serialnumber_ << ", max trig.id=" << tid << "\tmyId=" << this->myId();
#endif
                    }
                } while ( 0 );

                do {
                    adfs::stmt sql( fs.db() );
                    for ( auto& idx : rp->indices2() ) {

                        sql.prepare( "INSERT INTO peak"
                                     " (idTrigger,peak_time,peak_intensity,front_offset,front_intensity,back_offset,back_intensity )"
                                     " VALUES (?,?,?,?,?,?,?)" );
                        int id = 1;
                        auto apex  = wp->xy( idx.apex );
                        sql.bind( id++ ) = wp->serialnumber_;                                   // idTrigger
                        sql.bind( id++ ) = apex.first;                                          // peak_time
                        sql.bind( id++ ) = wp->toVolts( apex.second ) * std::milli::den;        // peak_intensity, mV
                        sql.bind( id++ ) = idx.first - idx.apex;                                // distance between front and apex
                        sql.bind( id++ ) = wp->toVolts( (*wp)[ idx.first ] ) * std::milli::den; // front mV
                        sql.bind( id++ ) = idx.second - idx.apex;                               // distance between apex and back
                        sql.bind( id++ ) = wp->toVolts( (*wp)[ idx.second ] ) * std::milli::den;// front mV
                        if ( sql.step() != adfs::sqlite_done ) {
                            ADERROR() << "sql error extended code: " << sql.extended_errcode() << "\t" << sql.errmsg();
                            return true;
                        }
                    }
                } while ( 0 );
            }
        }
        return true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////

    template<> ACQRSCONTROLSSHARED_EXPORT
    bool
    counting_data_writer_< acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > >::write( adfs::filesystem& fs, const boost::uuids::uuid& ) const
    {
        if ( auto accessor = dynamic_cast< threshold_result_accessor_type * >( accessor_.get() ) ) {
            if ( auto rp = accessor->data() ) { // std::shared_ptr< const acqrscontrols::u5303a::threshold_result >

                auto wp = rp->data();  // waveform

                do {
                    adfs::stmt sql( fs.db() );

                    sql.prepare( "INSERT INTO trigger ( id,protocol,timeSinceEpoch,elapsedTime,events,threshold,algo ) VALUES (?,?,?,?,?,?,?)" );
                    int id(1);
                    sql.bind( id++ ) = wp->serialnumber_;
                    sql.bind( id++ ) = wp->method_.protocolIndex();
                    sql.bind( id++ ) = wp->timeSinceEpoch_;
                    sql.bind( id++ ) = wp->meta_.initialXTimeSeconds;
                    sql.bind( id++ ) = wp->wellKnownEvents_;
                    sql.bind( id++ ) = rp->threshold_level();  // V
                    sql.bind( id++ ) = int( rp->algo() );
                    if ( sql.step() != adfs::sqlite_done ) {
                        ADERROR() << "sql error extended code: " << sql.extended_errcode() << "\t" << sql.errmsg();
                        return true;
                    }
                } while ( 0 );

                do {
                    adfs::stmt sql( fs.db() );
                    for ( auto& idx : rp->indices2() ) {

                        sql.prepare( "INSERT INTO peak"
                                     " (idTrigger,peak_time,peak_intensity,front_offset,front_intensity,back_offset,back_intensity )"
                                     " VALUES (?,?,?,?,?,?,?)" );
                        int id = 1;
                        auto apex  = wp->xy( idx.apex );
                        sql.bind( id++ ) = wp->serialnumber_;                                   // idTrigger
                        sql.bind( id++ ) = apex.first;                                          // peak_time
                        sql.bind( id++ ) = wp->toVolts( apex.second ) * std::milli::den;        // peak_intensity, mV
                        sql.bind( id++ ) = idx.first - idx.apex;                                // distance between front and apex
                        sql.bind( id++ ) = wp->toVolts( (*wp)[ idx.first ] ) * std::milli::den; // front mV
                        sql.bind( id++ ) = idx.second - idx.apex;                               // distance between apex and back
                        sql.bind( id++ ) = wp->toVolts( (*wp)[ idx.second ] ) * std::milli::den;// front mV
                        if ( sql.step() != adfs::sqlite_done ) {
                            ADERROR() << "sql error extended code: " << sql.extended_errcode() << "\t" << sql.errmsg();
                        }
                    }
                } while ( 0 );
            }
        }
        return true;
    }

}

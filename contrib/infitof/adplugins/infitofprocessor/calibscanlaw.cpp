// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com or info@ms-cheminfo.com
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

#include "calibscanlaw.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msfinder.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adprocessor/processmediator.hpp>
#include <adprocessor/scanlawextractor.hpp>
#include <adwidgets/mslockdialog.hpp>
#include <adwidgets/progresswnd.hpp>
#include <admtcontrols/scanlaw.hpp>
#include <infitofwidgets/scanlawdialog.hpp>
#include <QCoreApplication>
#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <boost/uuid/uuid.hpp>
#include <ratio>
#include <future>
#include <thread>

using namespace infitofprocessor;

CalibScanLaw::CalibScanLaw()
{
}

CalibScanLaw::~CalibScanLaw()
{
}

void
CalibScanLaw::initialSetup( std::shared_ptr< adprocessor::dataprocessor > dp, infitofwidgets::ScanLawDialog& dlg )
{
    double acclVoltage( 4000 ), tDelay( 0 );
    std::shared_ptr< admtcontrols::ScanLaw > scanlaw = std::make_shared< admtcontrols::infitof::ScanLaw >();

    if ( auto db = dp->db() ) {
        {
            adfs::stmt sql( *db );

            sql.prepare( "SELECT acclVoltage,tDelay FROM ScanLaw WHERE objuuid=?" );
            sql.bind( 1 ) = boost::uuids::uuid( { 0 } ); // find master scanlaw

            if ( sql.step() == adfs::sqlite_row ) {
                acclVoltage = sql.get_column_value< double >( 0 );
                tDelay      = sql.get_column_value< double >( 1 );
            }

            sql.prepare(
                "SELECT FLIGHT_LENGTH_L1"
                ", FLIGHT_LENGTH_L2"
                ", FLIGHT_LENGTH_L3"
                ", FLIGHT_LENGTH_LG"
                ", FLIGHT_LENGTH_L4"
                ", FLIGHT_LENGTH_LT"
                ", FLIGHT_LENGTH_EXIT"
                " FROM MULTUM_ANALYZER_CONFIG LIMIT 1");

            if ( sql.step() == adfs::sqlite_row ) {
                int row(0);
                double L1 = sql.get_column_value<double>(row++);
                double L2 = sql.get_column_value<double>(row++);
                double L3 = sql.get_column_value<double>(row++);
                double LG = sql.get_column_value<double>(row++);
                double L4 = sql.get_column_value<double>(row++);
                double LT = sql.get_column_value<double>(row++);
                double LE = sql.get_column_value<double>(row++);

                scanlaw = std::make_shared< admtcontrols::ScanLaw >( acclVoltage
                                                                       , tDelay
                                                                       , L1, L2, L3, LG, L4, LT, LE );
                dlg.setAcceleratorVoltage( acclVoltage, true );
                dlg.setTDelay( tDelay * std::micro::den, true );
                dlg.setL1( L1, true );

                dlg.setOrbitalLength( scanlaw->orbital_length() );
                dlg.setLinearLength( scanlaw->linear_length() );
                dlg.setScanLaw( scanlaw );
            }
        }
        {
            adfs::stmt sql( *db );
            sql.prepare( "SELECT id,description,fLength FROM Spectrometer" );
            bool found(false);
            if ( sql.step() == adfs::sqlite_row ) {
                auto objid = sql.get_column_value< boost::uuids::uuid >( 0 );
                auto sp = adcontrols::MassSpectrometerBroker::make_massspectrometer( objid );
                dlg.setSpectrometerData( sql.get_column_value< boost::uuids::uuid >( 0 )
                                         , QString::fromStdWString( sql.get_column_value< std::wstring >( 1 ) )
                                         , sp );
            }
        }
        {
            adfs::stmt sql( *db );
            sql.prepare( "SELECT objuuid,objtext,acclVoltage,tDelay FROM ScanLaw" );
            while( sql.step() == adfs::sqlite_row ) {
                dlg.addObserver( sql.get_column_value< boost::uuids::uuid >( 0 )
                                 , QString::fromStdString( sql.get_column_value< std::string >(1) )
                                 , sql.get_column_value< double >( 2 )
                                 , sql.get_column_value< double >( 3 )
                                 , dlg.peakCount() > 0 ? true : false );
            }
        }

        dlg.commit();
        dlg.setStyleSheet( "* { font-size: 9pt; }" );
    }
}

// [0]
void
CalibScanLaw::operator()( std::shared_ptr< adprocessor::dataprocessor > dp
                          , std::shared_ptr< const adcontrols::MassSpectrum > ms
                          , const std::pair< double, double >&
                          , bool isTime )
{
    if ( !ms || !dp )
        return;

    infitofwidgets::ScanLawDialog dlg;

    for ( auto& fms: adcontrols::segment_wrapper< const adcontrols::MassSpectrum >( *ms ) ) {
        int mode = fms.getMSProperty().mode();
        for ( const auto& a: fms.get_annotations() ) {
            if ( a.dataFormat() == adcontrols::annotation::dataFormula && a.index() >= 0 ) {
                dlg.addPeak( a.index()
                             , QString::fromStdString( a.text() )
                             , fms.getTime( a.index() )    // observed time-of-flight
                             , fms.getMass( a.index() )    // matched mass
                             , mode );
            }
        }
    }

    initialSetup( dp, dlg );

    double acclVoltage( 4000 ), tDelay( 0 );
    std::shared_ptr< admtcontrols::ScanLaw > scanlaw = std::make_shared< admtcontrols::infitof::ScanLaw >();

    if ( auto db = dp->db() ) {

        if ( dlg.exec() != QDialog::Accepted )
            return;

        QMessageBox::StandardButton reply
            = QMessageBox::question( 0
                                     , "infiTOF's ScanLaw & Dimenstion Calibration"
                                     , "Save result on data file parmanetly?"
                                     , QMessageBox::Yes|QMessageBox::No );

        if ( reply == QMessageBox::No )
            return;

        //----- L1
        do {
            adfs::stmt sql( *(dp->db()) );
            sql.prepare( "UPDATE MULTUM_ANALYZER_CONFIG SET FLIGHT_LENGTH_L1=?" );
            sql.bind( 1 ) = dlg.L1();
            while ( sql.step() == adfs::sqlite_row )
                ;
        } while ( 0 );

        do {
            // update database
            auto list = dlg.checkedObservers();
            for ( auto& obj: list ) {
                adfs::stmt sql( *(dp->db()) );
                sql.prepare( "UPDATE ScanLaw SET acclVoltage=?,tDelay=? WHERE objtext=?" );
                sql.bind( 1 ) = dlg.acceleratorVoltage();
                sql.bind( 2 ) = dlg.tDelay() / std::micro::den;
                sql.bind( 3 ) = obj.toStdString();
                while ( sql.step() == adfs::sqlite_row )
                    ;
            }
        } while ( 0 );
    }
}

// [1]
void
CalibScanLaw::operator()( std::shared_ptr< adprocessor::dataprocessor > dp, const portfolio::Folium& )
{
    adcontrols::MSLockMethod lockm;
    lockm.setEnabled( true );

    if ( auto pm = adprocessor::ProcessMediator::instance()->getProcessMethod() ) {
        if ( auto cm = pm->find< adcontrols::MSChromatogramMethod >() ) {
            lockm.setMolecules( cm->molecules() );
            lockm.setToleranceMethod( adcontrols::idToleranceDaltons );
            lockm.setTolerance( adcontrols::idToleranceDaltons, cm->tolerance() );
        }

        adwidgets::MSLockDialog dlg;
        dlg.setContents( lockm );

        dlg.setStyleSheet( "* { font-size: 9pt; }" );

        if ( dlg.exec() != QDialog::Accepted )
            return;

        dlg.getContents( lockm );

        (*pm) *= lockm;  // replace if already exists
        size_t count(0);
        auto progress( adwidgets::ProgressWnd::instance()->addbar() );

        using adprocessor::v3::ScanLawExtractor;

        auto future = std::async( std::launch::async, [this,dp,pm,&count,progress](){
                ScanLawExtractor()( dp, *pm, "tdcdoc.waveform.1.u5303a.ms-cheminfo.com", -1, [&](size_t a, size_t b){
                        return (*progress)( a, b );
                    } );
            } );

        while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
            QCoreApplication::instance()->processEvents();
        (*progress)( count ); // make it 100%

        do {
            auto progress( adwidgets::ProgressWnd::instance()->addbar() );
            auto future = std::async( std::launch::async, [&](){ computeScanLawTimeCourse( dp, progress ); } );

            while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
                QCoreApplication::instance()->processEvents();

        } while ( 0 );

        // update current massSpectrometer
        if ( auto spectrometer = dp->massSpectrometer() )
            spectrometer->initialSetup( *dp->db(), boost::uuids::uuid{ 0 } );

    }
}

bool
CalibScanLaw::computeScanLawTimeCourse( std::shared_ptr< adprocessor::dataprocessor > dp
                                        , std::shared_ptr< adwidgets::Progress > progress )
{
    // compute scanlaw time course
    auto sql = adfs::stmt( *dp->db() );

    sql.prepare( "SELECT min(rowid),formula,mass,time,mode,exactMass FROM ReferenceTof,MassReference"
                 " WHERE refid=id AND rowid >= ? GROUP BY protocol" );

    adcontrols::MSPeaks peaks;

    int64_t rowid(0), thisid(0);
    sql.bind( 1 ) = rowid;

    results_.clear();

    int pCount(0);

    for ( ;; ) {
        adcontrols::MSPeaks peaks;

        while ( sql.step() == adfs::sqlite_row ) {
            rowid = std::max( rowid, sql.get_column_value< int64_t >( 0 ) );

            if ( peaks.size() == 0 )
                thisid = rowid;

            peaks << adcontrols::MSPeak( sql.get_column_value<std::string>( 1 )        // formula
                                         , sql.get_column_value< double >( 2 )         // mass
                                         , sql.get_column_value< double >( 3 )         // time
                                         , int( sql.get_column_value< int64_t >( 4 ) ) // mode
                                         , -1
                                         , sql.get_column_value< double >( 5 ) );      // exact_mass
            (*progress)( ++pCount );
        }

        if ( peaks.size() == 0 )
            break;

        if ( peaks.size() >= 2 ) {
            double acclVoltage(0), tDelay(0);
            if ( dp->massSpectrometer()->estimateScanLaw( peaks, acclVoltage, tDelay ) )
                results_.emplace_back( thisid, tDelay, acclVoltage );
        }

        sql.reset();
        sql.bind( 1 ) = ++rowid;
    }

    sql.exec( "CREATE TABLE IF NOT EXISTS ScanLawTimeCourse ("
              " rowid INTEGAR PRIMARY KEY"
              ", tDelay REAL"
              ", acclVoltage REAL"
              ")"
        );
    sql.exec( "DELETE FROM ScanLawTimeCourse" );

    sql.prepare( "INSERT INTO ScanLawTimeCourse (rowid,tDelay,acclVoltage) VALUES (?,?,?)" );
    progress->setRange( pCount, pCount + results_.size() );
    for ( auto& value: results_ ) {
        sql.bind( 1 ) = std::get< 0 >( value );
        sql.bind( 2 ) = std::get< 1 >( value );
        sql.bind( 3 ) = std::get< 2 >( value );
        if ( sql.step() != adfs::sqlite_done )
            ADDEBUG() << "sql error";
        sql.reset();
        (*progress)( ++pCount );
    }

    return true;
}

bool
CalibScanLaw::loadScanLawTimeCourse( std::shared_ptr< adprocessor::dataprocessor > dp )
{
    auto sql = adfs::stmt( *dp->db() );

    results_.clear();

    sql.prepare( "SELECT rowid,tDelay,acclVoltage FROM ScanLawTimeCourse ORDER BY rowid" );
    while ( sql.step() == adfs::sqlite_row ) {
        results_.emplace_back( sql.get_column_value< int64_t >( 0 )
                               , sql.get_column_value< double >( 1 )
                               , sql.get_column_value< double >( 2 ) );
    }
    return ! results_.empty();
}

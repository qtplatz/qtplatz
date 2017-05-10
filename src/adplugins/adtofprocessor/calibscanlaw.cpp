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
#include <adcontrols/msproperty.hpp>
#include <adfs/sqlite.hpp>
#include <adportable/debug.hpp>
#include <adportable/timesquaredscanlaw.hpp>
#include <adwidgets/scanlawdialog.hpp>
#include <adwidgets/scanlawdialog2.hpp>
//#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <boost/uuid/uuid.hpp>
#include <ratio>

using namespace adtofprocessor;
    
CalibScanLaw::CalibScanLaw()
{
}

CalibScanLaw::~CalibScanLaw()
{
}

void
CalibScanLaw::operator()( std::shared_ptr< adprocessor::dataprocessor > dp
                          , std::shared_ptr< const adcontrols::MassSpectrum > ms
                          , const std::pair< double, double >&
                          , bool isTime )
{
    if ( !ms || !dp )
        return;

    adwidgets::ScanLawDialog2 dlg;
        
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
    
    dlg.commit();

    if ( auto db = dp->db() ) { // sqlite shared_ptr
        {
            adfs::stmt sql( *db );
            sql.prepare( "SELECT id,description,fLength FROM Spectrometer" );
            bool found(false);
            if ( sql.step() == adfs::sqlite_row ) {
                dlg.setSpectrometerData( sql.get_column_value< boost::uuids::uuid >( 0 )
                                         , QString::fromStdWString( sql.get_column_value< std::wstring >( 1 ) )
                                         , sql.get_column_value< double >( 2 ) ); // fLength
            } else {
                // dlg.setSpectrometerData( iid_spectrometer, "", 0 );
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
    }
    
    if ( dlg.exec() != QDialog::Accepted )
        return;

    QMessageBox::StandardButton reply
        = QMessageBox::question( 0
                                 , "Time-squared Scan-law & Dimenstion Calibration"
                                 , "Save result on data file parmanetly?"
                                 , QMessageBox::Yes|QMessageBox::No );
    
    if ( reply == QMessageBox::No )
        return;
    
    // update database (datafile)
    
    double t0 = dlg.tDelay() / std::micro::den;
    double acclV = dlg.acceleratorVoltage();
    adportable::TimeSquaredScanLaw law( acclV, t0, dlg.length() ); // <- todo: get 'L' from Spectrometer on db
    
    // update database
    auto list = dlg.checkedObservers();
    for ( auto& obj: list ) {
        adfs::stmt sql( *(dp->db()) );
        sql.prepare( "UPDATE ScanLaw SET acclVoltage=?,tDelay=? WHERE objtext=?" );
        sql.bind( 1 ) = acclV;
        sql.bind( 2 ) = t0;
        sql.bind( 3 ) = obj.toStdString();
        while ( sql.step() == adfs::sqlite_row )
            ;
    }        

#if 0
    // Todo
    // assign masses for processed peak
    for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
        fms.getMSProperty().setAcceleratorVoltage( acclV );
        fms.getMSProperty().setTDelay( t0 );
        fms.assign_masses( [&]( double time, int mode ){ return law.getMass( time, mode ); } );
    }

    // assign masses for profile spectrum
    if ( auto ms = pProfileSpectrum_.second.lock() ) {
        for ( auto& fms: adcontrols::segment_wrapper< adcontrols::MassSpectrum >( *ms ) ) {
            fms.getMSProperty().setAcceleratorVoltage( acclV );
            fms.getMSProperty().setTDelay( t0 );
            fms.assign_masses( [&]( double time, int mode ){ return law.getMass( time, mode ); } );
        }
    }
    // handleDataMayChanged();   
#endif
}

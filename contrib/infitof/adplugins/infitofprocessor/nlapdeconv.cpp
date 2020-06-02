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

#include "nlapdeconv.hpp"
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
#include <infitofwidgets/nlapdialog.hpp>
#include <QCoreApplication>
#include <QMenu>
#include <QMessageBox>
#include <QString>
#include <boost/uuid/uuid.hpp>
#include <ratio>
#include <future>
#include <thread>

using namespace infitofprocessor;

nLapDeconv::nLapDeconv()
{
}

nLapDeconv::~nLapDeconv()
{
}

void
nLapDeconv::initialSetup( std::shared_ptr< adprocessor::dataprocessor > dp, infitofwidgets::nLapDialog& dlg )
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
        dlg.commit();
        dlg.setStyleSheet( "* { font-size: 9pt; }" );
    }
}

// [0]
void
nLapDeconv::operator()( std::shared_ptr< adprocessor::dataprocessor > dp
                          , std::shared_ptr< const adcontrols::MassSpectrum > ms
                          , const std::pair< double, double >&
                          , bool isTime )
{
    if ( !ms || !dp )
        return;

    infitofwidgets::nLapDialog dlg;

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

    dlg.exec();

    return;
}

// [1]
void
nLapDeconv::operator()( std::shared_ptr< adprocessor::dataprocessor > dp, const portfolio::Folium& )
{
}

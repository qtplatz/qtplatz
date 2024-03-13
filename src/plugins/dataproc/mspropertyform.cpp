/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mspropertyform.hpp"
#include "ui_mspropertyform.h"
#include "sessionmanager.hpp"
#include "dataprocessor.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/constants.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adportable/debug.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportable/is_type.hpp>
#include <adlog/logger.hpp>
#include <adportable/utf.hpp>
#include <adutils/processeddata_t.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>
#include <iomanip>
#include <variant>
#include <QJsonDocument>

namespace dataproc {
    struct safe_string {
        const char * operator()( const char * p ) const {
            return p ? p : "(null)";
        }
    };
}

using namespace dataproc;

MSPropertyForm::MSPropertyForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MSPropertyForm)
{
    ui->setupUi(this);
}

MSPropertyForm::~MSPropertyForm()
{
    delete ui;
}

QWidget *
MSPropertyForm::create( QWidget * parent )
{
    return new MSPropertyForm( parent );
}

// adplugin::LifeCycle
void
MSPropertyForm::OnCreate( const adportable::Configuration& )
{
}

void
MSPropertyForm::OnInitialUpdate()
{
}

void
MSPropertyForm::OnFinalClose()
{
}

bool
MSPropertyForm::getContents( boost::any& ) const
{
    return false;
}

bool
MSPropertyForm::setContents( boost::any&& a )
{
    // ScopedDebug (x);
    // x << "setContents";

    if ( adportable::a_type< portfolio::Folium >::is_a( a ) ) {
        portfolio::Folium& folium = boost::any_cast< portfolio::Folium& >( a );
        ui->textEdit->clear();

        std::ostringstream o;
        o << "<p>";
        render( o, folium );
        o << "</p>";
        //-----------------
        if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
            if ( auto sp = dp->massSpectrometer() ) {
                if ( auto calibResult = sp->calibrateResult() ) {
                    o << "<p>";
                    render( o, *calibResult );
                    o << "</p>";
                }
            }
        }
        ui->textEdit->setText( o.str().c_str() );
    }
    return false;
}

void
MSPropertyForm::getLifeCycle( adplugin::LifeCycle*& p )
{
    p = this;
}


void
MSPropertyForm::render( std::ostream& o, const portfolio::Folium& folium )
{
    using adportable::utf;

    o << "<em>" << folium.fullpath() << "</em>";

    o << "<table border=\"1\" cellpadding=\"4\">";
    o << "<caption>File information</caption>";
    o << "<tr>";
    for ( auto att: folium.attributes() )
        o << "<th>" << att.first << "</th>";
    o << "</tr> <tr>";
    for ( auto att: folium.attributes() )
        o << "<td>" << att.second << "</th>";
    o << "</tr>";
    o << "</table>";

    if ( folium.dataClass() == adcontrols::MassSpectrum::dataClass() ||
         folium.dataClass() == adcontrols::Chromatogram::dataClass() ) {
        using dataTuple = std::tuple< std::shared_ptr< adcontrols::MassSpectrum >
                                      , std::shared_ptr< const adcontrols::MassSpectrum >
                                      , std::shared_ptr< adcontrols::Chromatogram >
                                      , std::shared_ptr< const adcontrols::Chromatogram >
                                      >;

        if ( auto var = adutils::to_std_variant< dataTuple >()( static_cast< const boost::any& >( folium ) ) ) {
            std::visit( [&]( auto&& arg ){ render( o, *arg ); }, *var );
        }
    }
}

void
MSPropertyForm::render( std::ostream& o, const adcontrols::MassSpectrum& ms )
{
    using adportable::utf;
    using namespace adcontrols::metric;

    static const char * const polarities [] = { "Indeterminant", "Positive", "Negative", "Mixed" };

    adcontrols::segment_wrapper< const adcontrols::MassSpectrum > segments( ms );

    std::vector< std::pair< std::string, std::string > > temp;
    temp.push_back( std::make_pair( "Spectrum", ( ms.isCentroid() ? "Centroid" : "Profile" ) ) );
    temp.push_back( std::make_pair( "Polarity", polarities[ ms.polarity() ] ) );

    o << "<hr>";
    o << "<table border=\"1\" cellpadding=\"1\">";
    o << "<caption>Descriptions</caption>";
    o << "<tr>";
    o << "<th>key</th>"
      << "<th>description</th>"
      << "</tr>";
    for ( auto it: temp ) {
        o << "<tr>"
          << "<td>" << it.first << "</td>"
          << "<td>" << it.second << "</td>"
          << "</tr>";
    }
    for ( size_t i = 0; i < ms.getDescriptions().size(); ++i ) {
        auto desc = ms.getDescriptions()[ i ];
        o << "<tr>"
          << "<td>" << desc.key<char>() << "</td>"
          << "<td>" << desc.text<char>() << "</td>"
          << "</tr>";
    }
    o << "</table>";

    std::pair< double, double > massrange = ms.getAcquisitionMassRange();
    o << "Acquisition mass range: " << boost::format( "%.3lf -- %.3lf" ) % massrange.first % massrange.second;

    o << "<table border=\"1\" cellpadding=\"4\">";
    o << "<caption>Acquisition and calibration parameter(s)</caption>";
    o << "<tr>";
    o << "<th>#seg</th>"
      << "<th>samp. interval(ps)</th>"
      << "<th>sampling start (&mu;s)</th>"
      << "<th>sampling end (&mu;s)</th>"
      << "<th>num. samples</th>"
      << "<th>num. average</th>"
      << "<th>mode</th>"
      << "<th>massSpectrometerClsid</th>"
      << "</tr>";

    int n = 0;
    for ( auto& m: segments ) {

        auto calib =  m.calibration();
        size_t nrowspan = calib ? 2 : 1; // calib->coeffs().empty() ? 1 : 2;

        const adcontrols::MSProperty& prop = m.getMSProperty();
        const adcontrols::SamplingInfo& info = prop.samplingInfo();
        double start_delay = info.delayTime();
        double time_end = info.delayTime() + info.fSampInterval() * info.nSamples();
        o << "<tr>"
          << boost::format( "<td rowspan=\"%1%\">" ) % nrowspan << n++ << "</td>"
          << "<td>" << boost::format( "%.1lfns" ) % scale_to_nano( info.fSampInterval() ) << "</td>"
          << "<td>" << boost::format( "%.4lf&mu;s" ) % scale_to_micro( start_delay ) << "</td>"
          << "<td>" << boost::format( "%.4lf&mu;s" ) % scale_to_micro( time_end ) << "</td>"
          << "<td>" << info.nSamples() << "</td>"
          << "<td>" << info.numberOfTriggers() << "</td>"
          << "<td>" << info.mode() << "</td>"
          << "<td>" << prop.massSpectrometerClsid() << "</td>"
          << "</tr>";

        //-----------------------------
        if ( calib ) {
            o << "<tr>";
            o << "<td colspan=7><b>Calibration ID:</b><i>" << calib->calibrationUuid() << "</i>";
            o << "&#8287;<b>Process date:</b>" << calib->date();
            o << "<hr>";
            o << calib->formulaText( true );
            o << "</td></tr>";
        }
        //-----------------------------
    }

    o << "</table>";
    o << "<hr>";

    // make_protocol_text( textv, ms.getMSProperty() );
    std::vector < std::pair< std::string, std::string > > textv;
    auto& prop = ms.getMSProperty();

    o << "<table border=\"1\" cellpadding=\"4\">";
    o << "<caption>Rapid protocol information</caption>";
    o << "<tr>";
    o << "<th>#seg</th>";
    std::for_each( textv.begin(), textv.end(), [&] ( const std::pair< std::string, std::string>& text ) { o << "<th>" << text.first << "</th>"; } );
    o << "</tr>";
    int seg = 0;
    for ( auto& m : segments ) {
        make_protocol_text( textv, m.getMSProperty() );
        o << "<tr>";
        o << "<td>" << seg++ << "</td>";
        std::for_each( textv.begin(), textv.end(), [&] ( const std::pair< std::string, std::string>& text ) { o << "<td>" << text.second << "</td>"; } );
        o << "</tr>";
    }
    o << "</table>";
    o << "<hr>";

    bool processed( false );
    // addatafile v3 will be handled here
    if ( auto dp = SessionManager::instance()->getActiveDataprocessor() ) {
        if ( auto spectrometer = dp->massSpectrometer() ) {

            o << "<pre>"
              << "Mass spectrometer: " << safe_string()( spectrometer->massSpectrometerName() ) << "; " << spectrometer->massSpectrometerClsid() << std::endl
              << "Data interpreter:  " << safe_string()( spectrometer->dataInterpreterText() ) << "; " << spectrometer->dataInterpreterUuid() << std::endl
              << "MSProperty dataInterpreterClsid: " << safe_string()( ms.getMSProperty().dataInterpreterClsid_v2() ) << std::endl;
            o << "</pre>" << std::endl;

            // high level app oriented interpreter
            if ( auto interpreter = adcontrols::DataInterpreterBroker::make_datainterpreter( spectrometer->dataInterpreterUuid() ) ) {
                if ( interpreter->make_device_text( textv, ms.getMSProperty() ) ) {
                    o << "<table border=\"1\" cellpadding=\"4\">";
                    std::for_each( textv.begin(), textv.end(), [&]( const auto& text ){
                            o << "<tr>"
                              << "<td>" << text.first << "</td><td>" << text.second << "</td>"
                              << "</tr>";
                        });
                    o << "</tr>" << std::endl;
                    o << "</table>";
                }
            }
            processed = true;
        }
    }

    if ( !processed ) {
        // device (averager) dependent data (require data interpreter)
        if ( auto ipClsid = prop.dataInterpreterClsid_v2() ) { // addatafile v2 only, v3 and later version no longer use this information
            o << "<h2>digitizer device dependent data '" << ( ( ipClsid && ipClsid[0] ) ? ipClsid : "none" ) << "'</h2>" << std::endl;
            o << "<p>data reader uuid: " << ms.dataReaderUuid() << "</p>" << std::endl;
            if ( auto interpreter = adcontrols::DataInterpreterBroker::make_datainterpreter( ipClsid ) ) {
                if ( interpreter->make_device_text( textv, ms.getMSProperty() ) ) {
                    o << "<table border=\"1\" cellpadding=\"4\">";
                    std::for_each( textv.begin(), textv.end()
                                   , [&]( const auto& text ){
                                         o << "<tr>"
                                           << "<td>" << text.first << "</td><td>" << text.second << "</td>"
                                           << "</tr>";
                                     });
                    o << "</tr>" << std::endl;
                    o << "</table>";
                }
            }
        }
    }
}

void
MSPropertyForm::render( std::ostream& o, const adcontrols::MSCalibrateResult& result )
{
    using adportable::utf;

    // o << "<em>" << "calibration process date: " << result.calibration().date() << "</em>";
    o << "<table border=\"1\" cellpadding=\"4\">";
    o << "<caption>MS Calibration Result (process date: " << result.calibration().date() << ")</caption>";
    o << "<tr>";
    o << "<th>" << "formula" << "</th>";
    o << "<th>" << "exact mass" << "</th>";
    o << "<th>" << "enable" << "</th>";
    o << "<th>" << "charge" << "</th>";
    o << "</tr>";

    for ( const auto& ref: result.references() ) {
        o << "<tr>"
          << "<td>" << adcontrols::ChemicalFormula().formatFormula( ref.display_formula() ) << "</td>"
          << "<td>" << ref.exact_mass() << "</td>"
          << "<td>" << (ref.enable() ? "enable" : "disable" ) << "</td>"
          << "<td>" << ref.charge_count() << "</td>"
          << "</tr>";
    }
    o << "</table>";
    //-------
    o << "<table border=\"1\" cellpadding=\"4\">";
    o << "<caption>" << "Assigned masses" << "</caption>";
    o << "<tr>";
    o << "<th>" << "formula" << "</th>";
    o << "<th>" << "ref. id" << "</th>";
    o << "<th>" << "peak id" << "</th>";
    o << "<th>" << "exact mass" << "</th>";
    o << "<th>" << "time (&mu;s)" << "</th>";
    o << "<th>" << "mass" << "</th>";
    o << "<th>" << "error (mDa)" << "</th>";
    o << "<th>" << "enable" << "</th>";
    o << "<th>" << "flags" << "</th>";
    o << "<th>" << "mode" << "</th>";
    o << "</tr>";
    for ( const auto& a: result.assignedMasses() ) {
        o << "<tr>"
          << "<td>" << adcontrols::ChemicalFormula().formatFormula( a.formula() ) << "</td>"
          << "<td>" << a.idReference() << "</td>"
          << "<td>" << a.idPeak() << "</td>"
          << "<td>" << a.exactMass() << "</td>"
          << "<td>" << boost::format( "%.3f" ) % (a.time() * std::micro::den) << "</td>"
          << "<td>" << a.mass() << "</td>"
          << "<td>" << boost::format( "%.3f" ) % ( (a.exactMass() - a.mass()) * 1000 ) << "</td>"
          << "<td>" << a.enable() << "</td>"
          << "<td>" << a.flags() << "</td>"
          << "<td>" << a.mode() << "</td>"
          << "</tr>";
    }
    o << "</table>";
}

void
MSPropertyForm::render( std::ostream& o, const adcontrols::Chromatogram& chro )
{
    o << "<br>";
    o << "<pre>Chromatogram (time of injection: " << chro.time_of_injection_iso8601() << ")</pre";
    o << "<br>";

    for ( const auto& desc:  chro.getDescriptions() ) {
        if ( desc.encode() == adcontrols::Encode_JSON ) {
            const auto& [key,value] = desc.keyValue();
            auto jobj = QJsonDocument::fromJson( value.data() );
            o << "<pre>"
              << key << "\n"
              << jobj.toJson( QJsonDocument::Indented ).toStdString()
              << "</pre>";
        }
    }

}


void
MSPropertyForm::make_protocol_text( std::vector< std::pair< std::string, std::string > >& textv
                                    , const adcontrols::MSProperty& prop ) const
{
    textv.clear();

    if ( const auto protocol = prop.tofProtocol() ) {

        textv.emplace_back( "number_of_triggers", ( boost::format("%1%") % protocol->number_of_triggers() ).str() );
        textv.emplace_back( "mode", ( boost::format( "%1%") % protocol->mode() ).str() );
        {
            std::ostringstream o;
            for ( auto& formula: protocol->formulae() )
                o << formula << ";";
            textv.emplace_back( "formulae", o.str() );
        }
        textv.emplace_back( "devicedata", (boost::format("%1% octets") % protocol->devicedata().size()).str() );

        for ( size_t i = 0; i < protocol->delay_pulses().size(); ++i ) {
            auto& dp = protocol->delay_pulses()[ i ];
            textv.emplace_back( ( boost::format( "delay/pulse(%1%)" ) % i ).str()
                                , ( boost::format( "%.3f (%.3f) &mu;s" ) % ( dp.first * std::micro::den ) % ( dp.second * std::micro::den ) ).str() );
        }

    }
}

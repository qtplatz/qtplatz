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

#include "mspropertyform.hpp"
#include "ui_mspropertyform.h"
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/metric/prefix.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/folium.hpp>
#include <adportable/is_type.hpp>
#include <adlog/logger.hpp>
#include <adportable/utf.hpp>
#include <boost/any.hpp>
#include <boost/exception/all.hpp>
#include <boost/format.hpp>
#include <sstream>
#include <iomanip>

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
MSPropertyForm::setContents( boost::any& a )
{
    if ( adportable::a_type< portfolio::Folium >::is_a( a ) ) {
        portfolio::Folium& folium = boost::any_cast< portfolio::Folium& >( a );
        ui->textEdit->clear();

        std::ostringstream o;
        o << "<p>";
        render( o, folium );
        o << "</p>";
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
        o << "<th>" << utf::to_utf8( att.first ) << "</th>";
    o << "</tr> <tr>";
    for ( auto att: folium.attributes() )
        o << "<td>" << utf::to_utf8( att.second ) << "</th>";
    o << "</tr>";
    o << "</table>";

    if ( folium.dataClass() == adcontrols::MassSpectrum::dataClass() ) {
        try {
            const adcontrols::MassSpectrumPtr ptr = boost::any_cast< adcontrols::MassSpectrumPtr >( folium.data() );
            if ( ptr ) 
                render( o, *ptr );
        } catch( std::exception() ) {
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
          << "<td>" << utf::to_utf8( desc.key() ) << "</td>"
          << "<td>" << utf::to_utf8( desc.text() ) << "</td>"
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
      << "<th>classid</th>"
      << "</tr>";
    int n = 0;
    for ( auto& m: segments ) {
        const adcontrols::MSCalibration& calib =  m.calibration();
        size_t nrowspan = calib.coeffs().empty() ? 1 : 2;
        const adcontrols::MSProperty& prop = m.getMSProperty();
        const adcontrols::MSProperty::SamplingInfo& info = prop.getSamplingInfo();
        double start_delay = scale_to<double, micro>( info.sampInterval * info.nSamplingDelay, pico );
        double time_end = scale_to<double, micro>( info.sampInterval * ( info.nSamplingDelay + info.nSamples ), pico );
        o << "<tr>"
          << boost::format( "<td rowspan=\"%1%\">" ) % nrowspan << n++ << "</td>"
          << "<td>" << info.sampInterval << "</td>"
          << "<td>" << boost::format( "%.4lf&mu;s" ) % start_delay << "</td>"
          << "<td>" << boost::format( "%.4lf&mu;s" ) % time_end << "</td>"
          << "<td>" << info.nSamples << "</td>"
          << "<td>" << info.nAverage << "</td>"
          << "<td>" << info.mode << "</td>"
		  << "<td>" << prop.dataInterpreterClsid() << "</td>"
          << "</tr>";

        if ( ! calib.coeffs().empty() ) {
            //-----------------------------
            o << "<tr>";
            o << "<td colspan=7><b>Calibration ID:</b><i>" << utf::to_utf8( calib.calibId() ) << "</i>"
              << "     " << calib.date();
            o << "<hr>";
            o << "&radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span> = "
              << boost::format( "%.14lf" ) % calib.coeffs()[0] << " + ";
            for ( size_t i = 1; i < calib.coeffs().size(); ++i )
                o << boost::format( "\t%.14lf &times; t<sup>%d</sup>" ) % calib.coeffs()[i] % i;

            if ( ! calib.t0_coeffs().empty() ) {
                o << "<br>"
                  << "T<sub>0</sub> = " << calib.t0_coeffs()[0];
                for ( size_t i = 1; i < calib.t0_coeffs().size(); ++i )
                    o << boost::format( "\t%.14lf &times; &radic;<span style=\"text-decoration: overline\">&nbsp;<i>m/z</i></span><sup>%d</sup>" )
                        % calib.coeffs()[i] % i;
                if ( calib.algorithm() == adcontrols::MSCalibration::MULTITURN_NORMALIZED )
                    o << "\t(MULTITURN_NORMAILZED algorithm)";
            }
            o << "</tr>";
            //-----------------------------            
		}
    }
    o << "</table>";
    o << "<hr>";

    try {
        // device (averager) dependent data (require data interpreter)
        std::vector < std::pair< std::string, std::string > > textv;
        auto& prop = ms.getMSProperty();
        const char * ipClsid = prop.dataInterpreterClsid();
        if ( ipClsid && (std::strlen( ipClsid )) > 0 ) {
            auto& interpreter = prop.spectrometer().getDataInterpreter();
            if ( interpreter.make_device_text( textv, ms.getMSProperty() ) ) {
                o << "<table border=\"1\" cellpadding=\"4\">";
                o << "<caption>Averager/digitizer dependent information</caption>";
                o << "<tr>";
                o << "<th>#seg</th>";
                std::for_each( textv.begin(), textv.end(), [&] ( const std::pair< std::string, std::string>& text ) { o << "<th>" << text.first << "</th>"; } );
                o << "</tr>";
                int seg = 0;
                for ( auto& m : segments ) {
                    if ( interpreter.make_device_text( textv, m.getMSProperty() ) ) {
                        o << "<tr>";
                        o << "<td>" << seg++ << "</td>";
                        std::for_each( textv.begin(), textv.end(), [&] ( const std::pair< std::string, std::string>& text ) { o << "<td>" << text.second << "</td>"; } );
                        o << "</tr>";
                    }
                }
                o << "</table>";
                o << "<hr>";
            }
        }
        else {
            o << "<hr>No dataInterpreterClsid specifid.</hr>";
            ADERROR() << "no dataInterpreterClisidSpecified.";
        }
    }
    catch ( boost::exception& ex ) {
        o << "<hr>data exception: " << boost::diagnostic_information( ex ) << "</hr>";
        ADERROR() << boost::diagnostic_information( ex );
    }
    catch ( ... ) {
        o << "<hr>data exception: " << boost::current_exception_diagnostic_information() << "</hr>";
        ADERROR() << boost::current_exception_diagnostic_information();
    }
}


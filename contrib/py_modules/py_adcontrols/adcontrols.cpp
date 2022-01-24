// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019-2020 MS-Cheminformatics LLC
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

#include "chemicalformula.hpp"
#include "isotopecluster.hpp"
#include "peakresult.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#if ! defined WIN32
#include <adcontrols/chromatogram.hpp>
#endif
#include <adcontrols/datainterpreter.hpp>
#include <adcontrols/datainterpreterbroker.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/typelist.hpp>
#include <adportable/debug.hpp>
#include <adportable/xml_serializer.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <compiler/decl_export.h>

using namespace boost::python;

void exportUUID();

namespace py_module {

    template< typename T >
    std::wstring DECL_EXPORT to_xml( const T& self )
    {
        std::wostringstream xml;
        if ( adportable::xml::serialize<>()( self, xml ) )
            return xml.str();
        return {};
    }

    boost::python::tuple MassSpectrum_getitem( const adcontrols::MassSpectrum& self, int index )
    {
        if ( index >= 0 && index < self.size() ) {
            if ( self.getColorArray() ) {
                return boost::python::make_tuple( self.time( index ), self.mass( index ), self.intensity( index ), self.getColor( index ) );
            } else {
                return boost::python::make_tuple( self.time( index ), self.mass( index ), self.intensity( index ) );
            }
        }
        PyErr_SetString(PyExc_IndexError, "index out of range");
        throw boost::python::error_already_set();;
    }

    const adcontrols::MassSpectrum& MassSpectra_getitem( const adcontrols::MassSpectra& self, int index )
    {
        if ( index >= 0 && index < self.size() )
            return self[ index ];
        PyErr_SetString(PyExc_IndexError, "index out of range");
        throw boost::python::error_already_set();;
    }
#ifndef WIN32
    boost::python::tuple Chromatogram_getitem( const adcontrols::Chromatogram& self, int index )
    {
        if ( index >= 0 && index < self.size() ) {
            return boost::python::make_tuple( self.time( index ), self.intensity( index ) );
        }
        PyErr_SetString(PyExc_IndexError, "index out of range");
        throw boost::python::error_already_set();;
    }
#endif

    boost::python::tuple MSProperty_massRange( const adcontrols::MSProperty& self )
    {
        auto range = self.instMassRange();
        return boost::python::make_tuple( range.first, range.second );
    }

    boost::python::dict MSProperty_sampInfo( const adcontrols::MSProperty& self )
    {
        boost::python::dict d;
        const auto& info = self.samplingInfo();
        d[ "sampInterval" ] = info.fSampInterval();
        d[ "sampDelay" ]    = info.fSampDelay();
        d[ "delayTime" ]    = info.delayTime();
        d[ "horPos" ]       = info.horPos();
        d[ "numTriggers" ]  = info.numberOfTriggers();
        return d;
    }

    boost::python::dict MSProperty_device_data( const adcontrols::MSProperty& self, const adcontrols::MassSpectrometer& sp ) {
        if ( auto interpreter = adcontrols::DataInterpreterBroker::make_datainterpreter( sp.dataInterpreterUuid() ) ) {
            std::vector < std::pair< std::string, std::string > > textv;
            interpreter->make_device_text( textv, self );
            dict d;
            for ( const auto& value: textv )
                d[ value.first ] = value.second;
            return d;
        }
        return {};
    }

    boost::uuids::uuid uuid() {
        return {};
    }
    boost::uuids::uuid string_to_uuid( const std::string& str ) {
        return boost::uuids::string_generator()( str );
    }

    std::string MassSpectrometer_massSpectrometerName( const adcontrols::MassSpectrometer& self ) {
        return std::string( self.massSpectrometerName() );
    }

    std::shared_ptr< adcontrols::MassSpectrometer > MassSpectrometer_create( const std::string& str ) {
        auto clsid = boost::uuids::string_generator()( str );
        if ( clsid != boost::uuids::uuid{} )
            return adcontrols::MassSpectrometer::create( clsid );
        return {}; // adcontrols::MassSpectrometer::create( str );
    }

    std::vector< boost::python::tuple > MassSpectrometer_installed_models() {
        std::vector< boost::python::tuple > a;
        for ( const auto& t: adcontrols::MassSpectrometer::installed_models() )
            a.emplace_back( make_tuple( t.first, t.second ) );
        return a;
    }

}


BOOST_PYTHON_MODULE( py_adcontrols )
{
    exportUUID();

    register_ptr_to_python< std::shared_ptr< adcontrols::Chromatogram > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::Chromatogram > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::DataInterpreter > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::DataInterpreter > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::MassSpectrometer > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MassSpectrometer > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::MassSpectrum > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MassSpectrum > >();
#ifndef WIN32
    register_ptr_to_python< std::shared_ptr< adcontrols::PeakResult > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::PeakResult > >();
#endif
    register_ptr_to_python< std::shared_ptr< adcontrols::ProcessMethod > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::ProcessMethod > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::MSCalibrateResult > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MSCalibrateResult > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::MSPeakInfo > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MSPeakInfo > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::MassSpectra > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MassSpectra > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::Targeting > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::Targeting > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::QuanSample > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::QuanSample > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::QuanSequence > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::QuanSequence > >();

    def( "uuid",                            py_module::uuid );
    def( "str_to_uuid",                     py_module::string_to_uuid );
    def( "MassSpectrometer_create",         py_module::MassSpectrometer_create );
    def( "MassSpectrometer_list",           py_module::MassSpectrometer_installed_models );

    class_< std::vector< boost::python::tuple > >("std_vector_tuple")
        .def( vector_indexing_suite< std::vector< boost::python::tuple >,true >() )
        ;

    class_< std::vector< boost::python::dict > >("std_vector_dict")
        .def( vector_indexing_suite< std::vector< boost::python::dict >,true >() )
        ;

    class_< adcontrols::annotation >( "Annotation" )
        .def< const std::string& (adcontrols::annotation::*)() const >(
            "text"  , &adcontrols::annotation::text, return_value_policy<copy_const_reference>() )
        .def< int (adcontrols::annotation::*)() const >    ( "index",    &adcontrols::annotation::index )
        .def< adcontrols::annotation::DataFormat (adcontrols::annotation::*)() const >( "dataFormat", &adcontrols::annotation::dataFormat )
        .def< int (adcontrols::annotation::*)() const >    ( "priority", &adcontrols::annotation::priority )
        .def< uint32_t (adcontrols::annotation::*)() const>( "flags",     &adcontrols::annotation::flags )
        .def< double (adcontrols::annotation::*)() const > ( "x",        &adcontrols::annotation::x )
        .def< double (adcontrols::annotation::*)() const > ( "y",        &adcontrols::annotation::y )
        .def< double (adcontrols::annotation::*)() const > ( "width",    &adcontrols::annotation::width )
        .def< double (adcontrols::annotation::*)() const > ( "height",   &adcontrols::annotation::height )
        ;

    class_< adcontrols::annotations >( "Annotations" )
        .def( "__len__",            &adcontrols::annotations::size )
        .def< const adcontrols::annotation& (adcontrols::annotations::*)(size_t) const>(
            "__getitem__"
            , &adcontrols::annotations::operator[], return_internal_reference<>() )
        ;

    class_< py_module::ChemicalFormula >( "ChemicalFormula", boost::python::init< const std::string >() )
        .def( "formula",            &py_module::ChemicalFormula::formula )
        .def( "monoIsotopicMass",   &py_module::ChemicalFormula::monoIsotopicMass )
        .def( "standardFormula",    &py_module::ChemicalFormula::standardFormula )
        .def( "formatFormula",      &py_module::ChemicalFormula::formatFormula )
        .def( "composition",        &py_module::ChemicalFormula::composition )
        .def( "composition_dict",   &py_module::ChemicalFormula::composition_dict )
        .def( "charge",             &py_module::ChemicalFormula::charge )
        ;

    class_< py_module::IsotopeCluster >( "IsotopeCluster", boost::python::init< const std::string >() )
        .def( "formula",           &py_module::IsotopeCluster::formula )
        .def( "setCharge",         &py_module::IsotopeCluster::setCharge )
        .def( "charge",            &py_module::IsotopeCluster::charge )
        .def( "setResolvingPower", &py_module::IsotopeCluster::setResolvingPower )
        .def( "resolvingPower",    &py_module::IsotopeCluster::resolvingPower )
        .def( "compute",           &py_module::IsotopeCluster::compute )
        ;

    class_< adcontrols::MSProperty >( "MSProperty" )
        .def( "timeSinceInjection", &adcontrols::MSProperty::timeSinceInjection )
        .def( "timeSinceEpoch",     &adcontrols::MSProperty::timeSinceEpoch )
        .def( "trigNumber",         &adcontrols::MSProperty::trigNumber )
        .def( "massRange",          &py_module::MSProperty_massRange )
        .def( "numAverage",         &adcontrols::MSProperty::numAverage )
        .def( "samplingInfo",       &py_module::MSProperty_sampInfo )
        .def( "massSpectrometerClsid"
              , &adcontrols::MSProperty::massSpectrometerClsid, return_value_policy<copy_const_reference>() )
        .def( "device_data",        &py_module::MSProperty_device_data )
        .def( "xml",                &py_module::to_xml< adcontrols::MSProperty > )
        ;

    class_< adcontrols::MassSpectrometer, boost::noncopyable >( "MassSpectrometer", no_init )
        .def( "massSpectrometerClsid"
              , &adcontrols::MassSpectrometer::massSpectrometerClsid, return_value_policy<copy_const_reference>() )
        .def( "dataInterpreterUuid"
              , &adcontrols::MassSpectrometer::dataInterpreterUuid, return_value_policy<copy_const_reference>() )
        .def( "massSpectrometerName"      , &py_module::MassSpectrometer_massSpectrometerName )
        .def( "dataInterpreterText"       , &adcontrols::MassSpectrometer::dataInterpreterText )
        ;

    class_< adcontrols::MassSpectrum >( "MassSpectrum" )
        .def( "__len__",            &adcontrols::MassSpectrum::size )
        .def( "__getitem__",        &py_module::MassSpectrum_getitem )
        .def( "resize",             &adcontrols::MassSpectrum::resize )
        .def( "mass",               &adcontrols::MassSpectrum::mass )
        .def( "time",               &adcontrols::MassSpectrum::time )
        .def( "intensity",          &adcontrols::MassSpectrum::intensity )
        .def( "numProtocols",       &adcontrols::MassSpectrum::numSegments )
        .def( "protocol",           &adcontrols::MassSpectrum::getProtocol )
        .def< const adcontrols::annotations& (adcontrols::MassSpectrum::*)() const >(
            "annotations",          &adcontrols::MassSpectrum::get_annotations, return_internal_reference<>() )
        .def( "isCentroid",         &adcontrols::MassSpectrum::isCentroid )
        .def( "isHistogram",        &adcontrols::MassSpectrum::isHistogram )
        .def( "polarity",           &adcontrols::MassSpectrum::polarity )
        .def( "mode",               &adcontrols::MassSpectrum::mode )
        .def< const adcontrols::MSProperty& (adcontrols::MassSpectrum::*)() const >(
            "property",             &adcontrols::MassSpectrum::getMSProperty, return_internal_reference<>() )
        .def( "xml",                &py_module::to_xml< adcontrols::MassSpectrum > )
        ;
#ifndef WIN32
    class_< adcontrols::Chromatogram >( "Chromatogram" )
        .def( "__len__",            &adcontrols::Chromatogram::size )
        .def( "__getitem__",        &py_module::Chromatogram_getitem )
        .def( "time",               &adcontrols::Chromatogram::time )
        .def( "intensity",          &adcontrols::Chromatogram::intensity )
        .def( "protocol",           &adcontrols::Chromatogram::protocol )
        .def( "xml",                &py_module::to_xml< adcontrols::Chromatogram > )
        ;
#endif
    class_< adcontrols::Baselines >( "Baselines" )
        .def( "__len__",            &adcontrols::Baselines::size )
        .def( "__getitem__",        &py_module::baselines_getitem )
        ;

    class_< adcontrols::Peaks >( "Peaks" )
        .def( "__len__",            &adcontrols::Peaks::size )
        .def( "__getitem__",        &py_module::peaks_getitem )
        ;
#ifndef WIN32
    class_< adcontrols::PeakResult >( "PeakResult" )
        .def( "xml",                &py_module::to_xml< adcontrols::PeakResult > )
        .def< const adcontrols::Baselines& (adcontrols::PeakResult::*)() const>(
            "baselines", &adcontrols::PeakResult::baselines, return_internal_reference<>() )
        .def< const adcontrols::Peaks& (adcontrols::PeakResult::*)() const>(
            "peaks", &adcontrols::PeakResult::peaks, return_internal_reference<>() )
        ;
#endif
    class_< adcontrols::ProcessMethod >( "ProcessMethod" )
        .def( "xml",                &py_module::to_xml< adcontrols::ProcessMethod > )
        ;

    class_< adcontrols::MSCalibrateResult >( "MSCalibrateResult" )
        .def( "xml",                &py_module::to_xml< adcontrols::MSCalibrateResult > )
        ;

    class_< adcontrols::MSPeakInfo >( "MSPeakInfo" )
        .def( "xml",                &py_module::to_xml< adcontrols::MSPeakInfo > )
        ;

    class_< adcontrols::MassSpectra >("MassSpectra" )
        .def( "__len__",            &adcontrols::MassSpectra::size )
        .def( "__getitem__",        &py_module::MassSpectra_getitem, return_internal_reference<>() )
        ;

    class_< adcontrols::Targeting >( "Targeting" )
        .def( "xml",                &py_module::to_xml< adcontrols::Targeting > )
        ;

    class_< adcontrols::QuanSample >( "QuanSample" )
        .def( "xml",                &py_module::to_xml< adcontrols::QuanSample > )
        ;

    class_< adcontrols::QuanSequence >( "QuanSequence" )
        .def( "xml",                &py_module::to_xml< adcontrols::QuanSequence > )
        ;
}

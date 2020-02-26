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
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/typelist.hpp>
#include <adcontrols/msproperty.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/debug.hpp>

using namespace boost::python;

namespace py_module {

    template< typename T >
    std::wstring to_xml( const T& self )
    {
        std::wostringstream xml;
        if ( adportable::xml::serialize<>()( self, xml ) )
            return xml.str();
        return {};
    }

    std::wstring MassSpectrum_property( const adcontrols::MassSpectrum& self )
    {
        return to_xml< adcontrols::MSProperty >( self.getMSProperty() );
    }

    std::vector< boost::python::tuple > MassSpectrum_values( const adcontrols::MassSpectrum& self )
    {
        std::vector< boost::python::tuple > a;
        if ( self.isCentroid() && self.getColorArray() ) {
            for ( size_t i = 0; i < self.size(); ++i )
                a.emplace_back( boost::python::make_tuple( self.time( i ), self.mass( i ), self.intensity( i ), self.getColor( i ) ) );
        } else {
            for ( size_t i = 0; i < self.size(); ++i )
                a.emplace_back( boost::python::make_tuple( self.time( i ), self.mass( i ), self.intensity( i ) ) );
        }
        return a;
    }

    const adcontrols::MassSpectrum& MassSpectra_getitem( const adcontrols::MassSpectra& self, int index )
    {
        if ( index >= 0 && index < self.size() )
            return self[ index ];
        PyErr_SetString(PyExc_IndexError, "index out of range");
        throw boost::python::error_already_set();;
    }

}


BOOST_PYTHON_MODULE( adControls )
{
    register_ptr_to_python< std::shared_ptr< adcontrols::MassSpectrum > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MassSpectrum > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::Chromatogram > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::Chromatogram > >();

    register_ptr_to_python< std::shared_ptr< adcontrols::PeakResult > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::PeakResult > >();

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

    class_< std::vector< boost::python::tuple > >("std_vector_tuple")
        .def( vector_indexing_suite< std::vector< boost::python::tuple >,true >() )
        ;

    class_< std::vector< boost::python::dict > >("std_vector_dict")
        .def( vector_indexing_suite< std::vector< boost::python::dict >,true >() )
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

    class_< adcontrols::MassSpectrum >( "MassSpectrum" )
        .def( "__len__",            &adcontrols::MassSpectrum::size )
        .def( "size",               &adcontrols::MassSpectrum::size )
        .def( "resize",             &adcontrols::MassSpectrum::resize )
        .def( "getMass",            &adcontrols::MassSpectrum::mass )
        .def( "getTime",            &adcontrols::MassSpectrum::time )
        .def( "getIntensity",       &adcontrols::MassSpectrum::intensity )
        .def( "numProtocols",       &adcontrols::MassSpectrum::numSegments )
        .def( "getProtocol",        &adcontrols::MassSpectrum::getProtocol )
        .def( "values",             &py_module::MassSpectrum_values )
        .def( "propertyXml",        &py_module::MassSpectrum_property )
        .def( "xml",                &py_module::to_xml< adcontrols::MassSpectrum > )
        ;

    class_< adcontrols::Chromatogram >( "Chromatogram" )
        .def( "__len__",            &adcontrols::Chromatogram::size )
        .def( "size",               &adcontrols::Chromatogram::size )
        .def( "getTime",            &adcontrols::Chromatogram::time )
        .def( "getIntensity",       &adcontrols::Chromatogram::intensity )
        .def( "protocol",           &adcontrols::Chromatogram::protocol )
        .def( "xml",                &py_module::to_xml< adcontrols::Chromatogram > )
        ;

    class_< adcontrols::Baselines >( "Baselines" )
        .def( "__len__",            &adcontrols::Baselines::size )
        .def( "__getitem__",        &py_module::baselines_getitem )
        ;

    class_< adcontrols::Peaks >( "Peaks" )
        .def( "__len__",            &adcontrols::Peaks::size )
        .def( "__getitem__",        &py_module::peaks_getitem )
        ;

    class_< adcontrols::PeakResult >( "PeakResult" )
        .def( "xml",                &py_module::to_xml< adcontrols::PeakResult > )
        .def< const adcontrols::Baselines& (adcontrols::PeakResult::*)() const>( "baselines", &adcontrols::PeakResult::baselines, return_internal_reference<>() )
        .def< const adcontrols::Peaks& (adcontrols::PeakResult::*)() const>( "peaks", &adcontrols::PeakResult::peaks, return_internal_reference<>() )
        ;

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

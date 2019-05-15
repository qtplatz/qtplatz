// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2019 MS-Cheminformatics LLC
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

#include <boost/python.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/massspectrum.hpp>
#include <memory>

double getitem_ms_intensity( const adcontrols::MassSpectrum& ms, size_t index ) {
    return ms.getMass( index );
}


using namespace boost::python;

BOOST_PYTHON_MODULE( adControls )
{
    // register_ptr_to_python<std::shared_ptr< adcontrols::ChemicalFormula > >();
    register_ptr_to_python< std::shared_ptr< adcontrols::MassSpectrum > >();
    register_ptr_to_python< std::shared_ptr< const adcontrols::MassSpectrum > >();

    double (adcontrols::ChemicalFormula::*d1)( const std::string& ) const = &adcontrols::ChemicalFormula::getMonoIsotopicMass;

    class_< adcontrols::ChemicalFormula >( "ChemicalFormula" )
        .def( "getMonoIsotopicMass", d1 )
        ;

    class_< adcontrols::MassSpectrum >( "MassSpectrum" )
        .def( "__len__", &adcontrols::MassSpectrum::size )
        .def( "size",    &adcontrols::MassSpectrum::size )
        .def( "resize",  &adcontrols::MassSpectrum::resize )
        .def( "getMass", &adcontrols::MassSpectrum::getMass )
        .def( "getTime", &adcontrols::MassSpectrum::getTime )
        .def( "getIntensity", &adcontrols::MassSpectrum::getIntensity )
        .def( "numProtocols", &adcontrols::MassSpectrum::numSegments )
        .def( "getProtocol", &adcontrols::MassSpectrum::getProtocol )
        ;
}

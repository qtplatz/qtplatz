// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2019-2020 Toshinobu Hondo, Ph.D.
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

#include "dataprocessor.hpp"
#include "datareader.hpp"
#include "singleton.hpp"
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/isotopecluster.hpp>
// #include <adcontrols/datafile.hpp>
// #include <adcontrols/datareader.hpp>
// #include <adportable/debug.hpp>
// #include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/molecule.hpp>
#include <adportable/debug.hpp>
#include <adportfolio/portfolio.hpp>
#include <boost/filesystem.hpp>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "WolframLibrary.h"
#include <unordered_map>
#include <codecvt>
#include <memory>
#include <string>
#include <sstream>

typedef std::unordered_map<mint, MTensor *> MTensorHash_t;
static MTensorHash_t map;

using namespace ws_adprocessor;

DLLEXPORT void manage_instance(WolframLibraryData libData, mbool mode, mint id)
{
    ADDEBUG() << __FUNCTION__;

	if (mode == 0) {
		MTensor *T = new(MTensor);
		map[id] = T;
		*T = 0;
	} else {
		MTensor *T = map[id];
		if (T != 0) {
			if (*T != 0) libData->MTensor_free(*T);
			map.erase(id);
		}
	}
}

EXTERN_C DLLEXPORT int
releaseInstance(WolframLibraryData libData, mint Argc, MArgument *Args, MArgument res)
{
    mint id;

    ADDEBUG() << __FUNCTION__;
    if (Argc != 1)
        return LIBRARY_FUNCTION_ERROR;
	id = MArgument_getInteger(Args[0]);

    return libData->releaseManagedLibraryExpression("LCG", id);
}

/* Return the version of Library Link */
EXTERN_C DLLEXPORT mint
WolframLibrary_getVersion()
{
	return WolframLibraryVersion;
}

/* Initialize Library */
EXTERN_C DLLEXPORT int
WolframLibrary_initialize(WolframLibraryData libData)
{
	return libData->registerLibraryExpressionManager("LCG", manage_instance);
}

/* Uninitialize Library */
EXTERN_C DLLEXPORT void
WolframLibrary_uninitialize(WolframLibraryData libData)
{
	int err = libData->unregisterLibraryExpressionManager("LCG");
}

/* Adds one to the input, returning the result  */
EXTERN_C DLLEXPORT int demo_I_I( WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res)
{
    mint I0;
    mint I1;
    I0 = MArgument_getInteger(Args[0]);
    I1 = I0 + 1;
    MArgument_setInteger(Res, I1);
    return LIBRARY_NO_ERROR;
}

EXTERN_C DLLEXPORT int
monoIsotopicMass( WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res ) // const char * formula )
{
    auto formula = MArgument_getUTF8String( Args[0] );
    ADDEBUG() << __FUNCTION__ << "(" << formula << ")";
    double exactMass = adcontrols::ChemicalFormula().getMonoIsotopicMass( adcontrols::ChemicalFormula::split( formula ) ).first;
    MArgument_setReal(Res, exactMass);
    //return exactMass;
    return LIBRARY_NO_ERROR;
}

EXTERN_C DLLEXPORT int
isotopeCluster( WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res )
{
    auto formula = MArgument_getUTF8String( Args[ 0 ] );
    auto resolving_power = MArgument_getReal( Args[ 1 ] );

    adcontrols::MassSpectrum ms;
    double mass = adcontrols::ChemicalFormula().getMonoIsotopicMass( formula );
    int charge = 0;
    std::vector< std::tuple< std::string, double, int > > fmc{ std::make_tuple( formula, mass, charge ) };
    adcontrols::isotopeCluster()( ms, fmc, resolving_power );

    MTensor T0;
    mint i, dims[2];
    int err = LIBRARY_NO_ERROR;

    //I0 = MArgument_getInteger(Args[0]);
    dims[0] = 2;
    dims[1] = ms.size();

    err = libData->MTensor_new(MType_Real, 2, dims, &T0);
    mint addr[2] = { 1, 1 };
    for ( int i = 0; i < ms.size() && !err; i++) {
        addr[0] = 1; addr[1] = i + 1;
        err = libData->MTensor_setReal( T0, addr, ms.mass(i-1));
        addr[0] = 2; addr[1] = i + 1;
        err = libData->MTensor_setReal( T0, addr, ms.intensity(i-1));
    }
    MArgument_setMTensor(Res, T0);
    return err;
}

EXTERN_C DLLEXPORT int
adFileOpen( WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res )
{
    auto name = MArgument_getUTF8String( Args[ 0 ] );

    ADDEBUG() << __FUNCTION__ << "(" << name << ")";

    int id(-1);
    boost::filesystem::path path( name );
    if ( boost::filesystem::exists( path ) ) {
        if ( auto dp = std::make_shared< dataProcessor >() ) {
            if ( dp->open( path.wstring() ) ) {
                id = singleton::instance()->set_dataProcessor( dp );
            }
        }
    }
    MArgument_setInteger(Res, id);
    return LIBRARY_NO_ERROR;
}

EXTERN_C DLLEXPORT int
adFileClose( WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res )
// adFileClose( const char * name )
{
    auto id = MArgument_getInteger(Args[0]);

    singleton::instance()->remove_dataProcessor( id );
    MArgument_setInteger(Res, 0);
    return LIBRARY_NO_ERROR;
}

EXTERN_C DLLEXPORT int
dataReaders( WolframLibraryData libData, mint Argc, MArgument *Args, MArgument Res )
{
    auto id = MArgument_getInteger(Args[0]);

    std::ostringstream o;

    if ( auto dp = singleton::instance()->dataProcessor( id ) ) {

        auto size = dp->dataReaders().size();

        boost::property_tree::ptree pt;
        boost::property_tree::ptree child;

        for ( auto reader : dp->dataReaders() ) {
            boost::property_tree::ptree a;
            a.put( "display_name", reader->display_name() );
            a.put( "objtext", reader->objtext() );
            a.put( "objuuid", reader->objuuid() );

            child.push_back( std::make_pair( "", a ) );
        }

        pt.add_child( "DataReader", child );

        write_json( o, pt );
    } else {
        o << "{}";
    }

    MArgument_setUTF8String( Res, const_cast< char * >( o.str().c_str() ) );
    return LIBRARY_NO_ERROR;
}

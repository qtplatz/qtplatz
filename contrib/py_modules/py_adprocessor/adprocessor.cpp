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

#include "dataprocessor.hpp"
#include "datareader.hpp"
#include "file.hpp"
#include "folder.hpp"
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <memory>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace boost::python;
// void exportUUID();

boost::uuids::uuid
gen_uuid() {
    return boost::uuids::random_generator()();
}

void set_uuid( const std::string& uuid ) {
    ADDEBUG() << "***** invoke set_uuid: " << uuid;
}

std::vector< std::string > uuids() {
    std::vector< std::string > a;
    for ( int i = 0; i < 5;  ++i )
        a.emplace_back( boost::uuids::to_string ( gen_uuid() ) );
    return a;
}

boost::python::tuple
my_tuple()
{
    return boost::python::make_tuple( std::string("abcde"), 1.0, std::string("xyz" ) );
}

std::vector< boost::python::tuple >
my_tuples()
{
    std::vector< boost::python::tuple > a;
    for ( size_t i = 0; i < 5; ++i )
        a.emplace_back( boost::python::make_tuple( std::string("abcde"), 1.0, gen_uuid() ) );
        //a.emplace_back( boost::python::make_tuple( std::string("abcde"), 1.0, std::to_string(i) ) );
    return a;
}

BOOST_PYTHON_MODULE( adProcessor )
{
    // exportUUID();

    register_ptr_to_python< std::shared_ptr< py_module::DataReader > >();

    def( "gen_uuid", gen_uuid );
    def( "set_uuid", set_uuid );
    def( "uuids", &uuids );
    def( "tuple", my_tuple );
    def( "tuples", my_tuples );

    class_< std::vector< std::string > >("std_vector_string")
        .def( vector_indexing_suite< std::vector< std::string > >() )
        ;

    class_< std::vector< std::shared_ptr< py_module::DataReader > > >("std_vector_std_shared_ptr_DataReader")
        .def( vector_indexing_suite< std::vector< std::shared_ptr< py_module::DataReader > >, true >() )
        ;

    class_< py_module::dataProcessor >( "processor" )
        .def( "open",               &py_module::dataProcessor::open )
        .def( "dataReaderTuples",   &py_module::dataProcessor::dataReaderTuples )
        .def( "dataReaders",        &py_module::dataProcessor::dataReaders )
        .def( "dataReader",         &py_module::dataProcessor::dataReader )
        .def( "filename",           &py_module::dataProcessor::filename )
        //.def( "root",               &py_module::dataProcessor::root )
        .def( "findFolder",         &py_module::dataProcessor::findFolder )
        .def( "massSpectrometer",   &py_module::dataProcessor::massSpectrometer )
        ;

    class_< py_module::folder >( "folder" )
        .def( "rowid",              &py_module::folder::rowid )
        .def( "name",               &py_module::folder::name )
        .def( "id",                 &py_module::folder::id )
        .def( "attributes",         &py_module::folder::attributes )
        .def( "folders",            &py_module::folder::folders )
        .def( "files",              &py_module::folder::files )
        ;

    class_< py_module::file >( "file" )
        .def( "rowid",              &py_module::file::rowid )
        .def( "name",               &py_module::file::name )
        .def( "id",                 &py_module::file::id )
        .def( "attributes",         &py_module::file::attributes )
        .def( "attachments",        &py_module::file::attachments )
        .def( "body",               &py_module::file::body )
        ;

    class_< py_module::DataReader >( "dataReader", no_init )
        .def( "objuuid",            &py_module::DataReader::objuuid )
        .def( "objtext",            &py_module::DataReader::objtext )
        .def( "display_name",       &py_module::DataReader::display_name )
        .def( "size",               &py_module::DataReader::size, py_module::DataReader_overloads() )
        .def( "readSpectrum",       &py_module::DataReader::readSpectrum )
        .def( "rewind",             &py_module::DataReader::rewind )
        .def( "next",               &py_module::DataReader::next )
        .def( "rowid",              &py_module::DataReader::rowid )
        .def( "epoch_time",         &py_module::DataReader::epoch_time )
        .def( "elapsed_time",       &py_module::DataReader::elapsed_time )
        .def( "time_since_inject",  &py_module::DataReader::time_since_inject )
        .def( "protocol",           &py_module::DataReader::protocol )
        ;
}

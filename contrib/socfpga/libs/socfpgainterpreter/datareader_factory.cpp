/**************************************************************************
** Copyright (C) 2010-2019 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2019 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "datareader_factory.hpp"
#include "datareader.hpp"
#include "factory_plugin.hpp"
#include <adcontrols/datareader.hpp>
#include <adplugin/visitor.hpp>
#include <adplugin/iid.hpp>
#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/type.hpp>

namespace socfpgainterpreter {

    std::shared_ptr< adplugin::plugin > datareader_factory::instance_;

    struct IID_DataInterpreter {
        static const std::string value;
    };

    struct IID_DataReader {
        static const std::string value;
    };

    const std::string IID_DataInterpreter::value = std::string( adplugin::iid::iid_datainterpreter ) + ".ms-cheminfo.com";
    const std::string IID_DataReader::value = std::string( adplugin::iid::iid_datareader ) + ".ms-cheminfo.com";

}

using namespace socfpgainterpreter;

datareader_factory::~datareader_factory()
{
}

datareader_factory::datareader_factory()
{
}

adplugin::plugin *
datareader_factory::instance()
{
    static std::once_flag flag;
    std::call_once( flag, [](){
            struct make_shared_enabler : public datareader_factory {};
            instance_ = std::make_shared< make_shared_enabler >();
        });

    return instance_.get();
}

void
datareader_factory::accept( adplugin::visitor& visitor, const char * adplugin )
{
    visitor.visit( this, adplugin );

    adcontrols::DataReader::register_factory(
        [] ( const char * traceid ) {
            return std::make_shared< DataReader >( traceid );
        }, typeid( DataReader ).name() );

    // register all supported 'traceid' by name
    for ( const auto& traceid: DataReader::traceid_list() )
        adcontrols::DataReader::assign_reader( typeid( DataReader ).name(), traceid.c_str() );

    //if ( auto ptr = factory_plugin< waveform::DataInterpreter< ads54j::waveform >, IID_DataInterpreter >::make_this() ) {
    //    ptr->accept( visitor, adplugin );
    //}

}

const char *
datareader_factory::iid() const
{
    return IID_DataReader::value.c_str();
}

void *
datareader_factory::query_interface_workaround( const char * typname )
{
    if ( std::strcmp( typname, typeid( adcontrols::DataReader ).name() ) == 0 )
        return reinterpret_cast< void * >(this);

    return nullptr;
}

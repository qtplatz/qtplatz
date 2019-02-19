/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@scienceliaison.com
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

#include "simpleobserver.hpp"
#include <boost/version.hpp>
#if BOOST_VERSION < 106000
#include <boost/uuid/uuid_io.hpp>
#endif
#include <boost/uuid/uuid_generators.hpp>

using namespace adacquire;

SimpleObserver::SimpleObserver( const char * objtext
                                , const char * dataInterpreterClsid
                                , const so::Description& desc ) : objid_( boost::uuids::name_generator( base_uuid() )( objtext ) )
                                                                , objtext_( objtext )
                                                                , clsid_( dataInterpreterClsid )
                                                                , desc_( desc )
{
    desc_.set_trace_id( objtext );  // unique name for the trace, can be used as 'data storage name'
	desc_.set_objtext(objtext);
	desc_.set_objid(objid_);
    setDescription( desc_ );
}

SimpleObserver::SimpleObserver( const char * objtext
                                , const boost::uuids::uuid&  objid
                                , const char * dataInterpreterClsid
                                , const so::Description& desc ) : objid_( objid )
                                                                , objtext_( objtext )
                                                                , clsid_( dataInterpreterClsid )
                                                                , desc_( desc )
{
    desc_.set_trace_id( objtext );  // unique name for the trace, can be used as 'data storage name'
	desc_.set_objtext(objtext);
	desc_.set_objid(objid_);
    setDescription( desc_ );
}

SimpleObserver::~SimpleObserver()
{
}

const char *
SimpleObserver::dataInterpreterClsid() const
{
    return clsid_.c_str();
}

const char *
SimpleObserver::objtext() const
{
    return objtext_.c_str();
}

const boost::uuids::uuid&
SimpleObserver::objid() const
{
    return objid_;
}

uint64_t
SimpleObserver::uptime() const
{
    return 0;
}

void
SimpleObserver::uptime_range( uint64_t& oldest, uint64_t& newest ) const
{
    oldest = newest = 0;
}

std::shared_ptr< so::DataReadBuffer >
SimpleObserver::readData( uint32_t pos )
{
    return 0;
}

int32_t
SimpleObserver::posFromTime( uint64_t nsec ) const
{
    return 0;
}

bool
SimpleObserver::prepareStorage( SampleProcessor& sp ) const
{
    if ( preparing_ )
        return preparing_( sp );
    return false;
}

bool
SimpleObserver::closingStorage( SampleProcessor& sp ) const
{
    if ( closing_ )
        return closing_( sp );
    return false;
}

void
SimpleObserver::setPrepareStorage( std::function< bool( SampleProcessor& ) > f )
{
    preparing_ = f;
}

void
SimpleObserver::setClosingStorage( std::function< bool( SampleProcessor& ) > f )
{
    closing_ = f;
}

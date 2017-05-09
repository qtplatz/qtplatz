/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC
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

#include "processmediator.hpp"
#include <adcontrols/massspectrum.hpp>

using namespace adprocessor;

ProcessMediator::ProcessMediator()
{
}
    
ProcessMediator::~ProcessMediator()
{
}

ProcessMediator *
ProcessMediator::instance()
{
    static ProcessMediator __instance;
    return &__instance;
}

boost::signals2::connection
ProcessMediator::registerOnCreate( const boost::uuids::uuid& uuid, onCreate_t::slot_type slot )
{
    return onCreate_[ uuid ].connect( slot );
}

boost::signals2::connection
ProcessMediator::registerAddContextMenu( const boost::uuids::uuid& uuid, addContextMenu_t::slot_type slot )
{
    return addContextMenu_[ uuid ].connect( slot );
}

boost::signals2::connection
ProcessMediator::registerEstimateScanLaw( const boost::uuids::uuid& uuid, estimateScanLaw_t::slot_type slot )
{
    return estimateScanLaw_[ uuid ].connect( slot );
}

void
ProcessMediator::onCreate( const boost::uuids::uuid& uuid, std::shared_ptr< adprocessor::dataprocessor > dp )
{
    auto it = onCreate_.find( uuid );
    if ( it != onCreate_.end() )
        it->second( dp, false );
}

void
ProcessMediator::onDestroy( const boost::uuids::uuid& uuid, std::shared_ptr< adprocessor::dataprocessor > dp )
{
    auto it = onCreate_.find( uuid );
    if ( it != onCreate_.end() )
        it->second( dp, true );
}

void
ProcessMediator::unregister( const boost::uuids::uuid& uuid )
{
    addContextMenu_.erase( uuid );
    onCreate_.erase( uuid );
}

void
ProcessMediator::addContextMenu( const boost::uuids::uuid& uuid
                                 , std::shared_ptr< adprocessor::dataprocessor > dp
                                 , ContextID context
                                 , QMenu& menu
                                 , std::shared_ptr< const adcontrols::MassSpectrum > ms
                                 , const std::pair< double, double >& range
                                 , bool isTime )
{
    auto it = addContextMenu_.find( uuid );

    if ( it != addContextMenu_.end() )
        it->second( dp, context, menu, ms, range, isTime );
}

bool
ProcessMediator::estimateScanLaw( const boost::uuids::uuid& uuid
                                  , std::shared_ptr< adprocessor::dataprocessor > dp
                                  , std::shared_ptr< const adcontrols::MassSpectrum > ms
                                  , const std::vector< std::pair< int, int > >& refs )
{
    auto it = estimateScanLaw_.find( uuid );

    if ( it != estimateScanLaw_.end() ) {
        boost::optional<bool> result = it->second( dp, ms, refs );
        return result.get();
    }

    return false;
}


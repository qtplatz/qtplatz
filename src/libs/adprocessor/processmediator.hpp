// This is a -*- C++ -*- header.
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

#pragma once

#include "adprocessor_global.hpp"
#include "dataprocessor.hpp"
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/signals2/signal.hpp>

class QMenu;

namespace adcontrols {
    class MassSpectrum;
}

namespace adprocessor {

    class ADPROCESSORSHARED_EXPORT ProcessMediator {

        ProcessMediator();
        ProcessMediator( const ProcessMediator& ) = delete;
        ProcessMediator& operator = ( const ProcessMediator& ) = delete;

    public:
        virtual ~ProcessMediator();
        static ProcessMediator * instance();
        
        typedef boost::signals2::signal< void( std::shared_ptr< adprocessor::dataprocessor >, bool ) > onCreate_t;

        typedef boost::signals2::signal< void( std::shared_ptr< adprocessor::dataprocessor >
                                               , ContextID
                                               , QMenu&
                                               , std::shared_ptr< const adcontrols::MassSpectrum >
                                               , const std::pair< double, double >&, bool isTime ) > addContextMenu_t;

        typedef boost::signals2::signal< bool( std::shared_ptr< adprocessor::dataprocessor >
                                               , std::shared_ptr< const adcontrols::MassSpectrum >
                                               , const std::vector< std::pair< int, int > >& refs ) > estimateScanLaw_t;

        void onCreate( const boost::uuids::uuid&, std::shared_ptr< adprocessor::dataprocessor > );
        void onDestroy( const boost::uuids::uuid&, std::shared_ptr< adprocessor::dataprocessor > );
        
        boost::signals2::connection registerOnCreate( const boost::uuids::uuid&, onCreate_t::slot_type );
        boost::signals2::connection registerAddContextMenu( const boost::uuids::uuid&, addContextMenu_t::slot_type );
        boost::signals2::connection registerEstimateScanLaw( const boost::uuids::uuid&, estimateScanLaw_t::slot_type );

        void unregister( const boost::uuids::uuid& );
        
        void addContextMenu( const boost::uuids::uuid&
                             , std::shared_ptr< adprocessor::dataprocessor >
                             , ContextID
                             , QMenu&
                             , std::shared_ptr< const adcontrols::MassSpectrum >
                             , const std::pair< double, double >&
                             , bool isTime );

        bool estimateScanLaw( const boost::uuids::uuid&
                              , std::shared_ptr< adprocessor::dataprocessor >
                              , std::shared_ptr< const adcontrols::MassSpectrum >
                              , const std::vector< std::pair< int, int > >& );
        
    private:
        std::map< boost::uuids::uuid, onCreate_t > onCreate_;
        std::map< boost::uuids::uuid, addContextMenu_t > addContextMenu_;
        std::map< boost::uuids::uuid, estimateScanLaw_t > estimateScanLaw_;
    };

}



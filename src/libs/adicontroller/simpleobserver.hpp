/**************************************************************************
** Copyright (C) 2015-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2015-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#pragma once

#include "adicontroller_global.hpp"
#include "simpleobserver.hpp"
#include "signalobserver.hpp"
#include <workaround/boost/uuid/uuid.hpp>
#include <functional>

namespace adicontroller {

    namespace so = adicontroller::SignalObserver;

    class ADICONTROLLERSHARED_EXPORT SimpleObserver : public so::Observer {
    
    public:
        virtual ~SimpleObserver();
        SimpleObserver( const char * objtext, const char * dataInterpreterClsid, const so::Description& desc );
        
        const boost::uuids::uuid& objid() const override;
        const char * objtext() const override;
        
        uint64_t uptime() const override;
        void uptime_range( uint64_t& oldest, uint64_t& newest ) const override;
        
        std::shared_ptr< so::DataReadBuffer > readData( uint32_t pos ) override;
        const char * dataInterpreterClsid() const override;
        
        int32_t posFromTime( uint64_t usec ) const override;

        bool prepareStorage( SampleProcessor& ) const override;

        bool closingStorage( SampleProcessor& ) const override;
        
        //
        void setPrepareStorage( std::function< bool( SampleProcessor& ) > );
        void setClosingStorage( std::function< bool( SampleProcessor& ) > );

    private:
        const boost::uuids::uuid objid_;
        const std::string objtext_;
        const std::string clsid_;
        so::Description desc_;
        std::function< bool( SampleProcessor& ) > preparing_;
        std::function< bool( SampleProcessor& ) > closing_;
    };
}

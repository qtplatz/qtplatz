/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC
*
** Contact: toshi.hondo@qtplatz.com
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

#include "datawriter.hpp"
#include "signalobserver.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <atomic>
#include <algorithm>
#include <chrono>
#include <mutex>

namespace adicontroller {

    namespace SignalObserver {

        ///////
        DataWriter::DataWriter() : elapsed_time_( 0 )
                                 , epoch_time_( 0 )
                                 , pos_( 0 )
                                 , fcn_( 0 )
                                 , ndata_( 0 )
                                 , events_( 0 )
        {
        }

        DataWriter::DataWriter( const DataReadBuffer& rb ) : elapsed_time_( rb.elapsed_time() )
                                                           , epoch_time_( rb.epoch_time() )
                                                           , pos_( rb.pos() )
                                                           , fcn_( rb.fcn() )
                                                           , ndata_( rb.ndata() )
                                                           , events_( rb.events() )
        {
        }

        DataWriter::DataWriter( boost::any&& a ) : any_( a )
        {
        }
        
        DataWriter::~DataWriter()
        {
        }
        
        // uint64_t& DataWriter::timepoint()    { return epoch_time_; }
        // uint64_t& DataWriter::epoch_time()   { return epoch_time_; }
        // uint64_t& DataWriter::elapsed_time() { return elapsed_time_; }
        // uint64_t& DataWriter::pos()          { return pos_; }
        // uint32_t& DataWriter::fcn()          { return fcn_; }
        // uint32_t& DataWriter::ndata()        { return ndata_; }
        // uint32_t& DataWriter::events()       { return events_; }
        
        uint64_t DataWriter::timepoint() const       { return epoch_time_; }
        uint64_t DataWriter::epoch_time() const      { return epoch_time_; }
        uint64_t DataWriter::elapsed_time() const    { return elapsed_time_; }
        uint64_t DataWriter::pos() const             { return pos_; }       
        uint32_t DataWriter::fcn() const             { return fcn_; }       
        uint32_t DataWriter::ndata() const           { return ndata_; }     
        uint32_t DataWriter::events() const          { return events_; }    

        const boost::any& DataWriter::data() const   { return any_; }
        void DataWriter::setData( boost::any&& d )     { any_ = d; }

    };

}



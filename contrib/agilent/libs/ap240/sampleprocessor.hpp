/**************************************************************************
** Copyright (C) 2010-2015 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC
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

#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <atomic>
#include <string>
#include <memory>

namespace adfs { class filesystem; class file; }

namespace signalobserver { struct DataReadBuffer; class Observer; }

namespace ap240 {

    class SampleProcessor {
	public:
        ~SampleProcessor();
        SampleProcessor( boost::asio::io_service& );

        void prepare_storage( signalobserver::Observer * );
        void handle_data( uint32_t objId, int32_t pos, const signalobserver::DataReadBuffer& );
        boost::asio::io_service::strand& strand() { return strand_; }
        void pos_front( unsigned int pos, unsigned long objId );
        void stop_triggered();
        
    private:
		void create_acquireddata_table();
        void populate_descriptions( signalobserver::Observer * );
        void populate_calibration( signalobserver::Observer * );

        boost::filesystem::path storage_name_;
        std::unique_ptr< adfs::filesystem > fs_;
		bool inProgress_;
        size_t myId_;
        boost::asio::io_service::strand strand_;
        std::atomic<unsigned long> objId_front_;
        std::atomic<unsigned int> pos_front_;
        std::atomic< bool > stop_triggered_;
    };

}


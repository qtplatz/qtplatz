/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC
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

#include "adacquire_global.hpp"
#include <adportable/semaphore.hpp>
#include <boost/asio.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>
#include <filesystem>
#include <string>
#include <thread>
#include <memory>
#include <atomic>
#include <chrono>
#include <deque>
#include <future>

namespace adfs { class filesystem; class file; class sqlite; }
namespace adcontrols { class SampleRun; namespace ControlMethod { class Method; } }
namespace adacquire { namespace SignalObserver { class DataReadBuffer; class Observer; class DataWriter; } }
// namespace boost { namespace uuids { struct uuid; } }

namespace adacquire {

    class ADACQUIRESHARED_EXPORT SampleProcessor : public std::enable_shared_from_this< SampleProcessor > {
        SampleProcessor( const SampleProcessor& ) = delete;
        const SampleProcessor& operator = ( const SampleProcessor& ) = delete;
	public:
        ~SampleProcessor();
        explicit SampleProcessor( std::shared_ptr< adcontrols::SampleRun >
                                  , std::shared_ptr< adcontrols::ControlMethod::Method > );

        void prepare_storage( SignalObserver::Observer * );

        void write( const boost::uuids::uuid& objId, std::shared_ptr< adacquire::SignalObserver::DataWriter > );

        void pos_front( unsigned int pos, unsigned long objId );
        void stop_triggered();

        bool inject_triggered() const;
        void set_inject_triggered( bool );

        std::shared_ptr< const adcontrols::SampleRun > sampleRun() const;

        std::shared_ptr< const adcontrols::ControlMethod::Method > controlMethod() const;

        const uint64_t& elapsed_time() const;

        adfs::filesystem& filesystem() const;

        static std::filesystem::path prepare_sample_run( adcontrols::SampleRun&, bool createDirectory = false );

        const std::filesystem::path& storage_name() const;

        bool prepare_snapshot_storage( adfs::sqlite& db ) const;

        void close( bool detach = true );

    private:
		void create_acquireddata_table();
        void populate_descriptions( SignalObserver::Observer * );
        void populate_calibration( SignalObserver::Observer * );

        void writer_thread();

        uint32_t __write( const boost::uuids::uuid& objId, std::shared_ptr< adacquire::SignalObserver::DataWriter > );
        void __close();

        static void populate_descriptions( SignalObserver::Observer *, adfs::sqlite& );
        static void populate_calibration( SignalObserver::Observer *, adfs::sqlite& );

        std::filesystem::path storage_name_;
        std::unique_ptr< adfs::filesystem > fs_;
        bool c_acquisition_active_;
        size_t myId_;
        std::atomic<unsigned long> objId_front_;
        std::atomic<unsigned int> pos_front_;
        std::atomic< bool > stop_triggered_;
        std::shared_ptr< adcontrols::SampleRun > sampleRun_;
        std::shared_ptr< adcontrols::ControlMethod::Method > ctrl_method_;
        std::chrono::system_clock::time_point tp_inject_trigger_;
        std::chrono::steady_clock::time_point tp_close_trigger_;
        std::weak_ptr< adacquire::SignalObserver::Observer > masterObserver_;

        uint64_t ts_inject_trigger_;
        uint64_t elapsed_time_;
        std::mutex mutex_;
        adportable::semaphore sema_;
        std::deque< std::pair<boost::uuids::uuid, std::shared_ptr<adacquire::SignalObserver::DataWriter>> > que_;
        std::thread thread_;
        std::atomic_bool closed_flag_;
        size_t deffered_count_;
        boost::optional< std::future< void > > close_future_;
    };

}

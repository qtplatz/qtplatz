// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <vector>
#include <deque>
#include <map>

// #include <adinterface/controlserverC.h>
// #include <adinterface/signalobserverC.h>
// #include <adinterface/receiverC.h>
// #include <adplugin/orbservant.hpp>

#include <workaround/boost/asio.hpp>

#include <thread>
#include <mutex>
#include <atomic>
#include <adlog/logger.hpp>

namespace adplugin { class orbServant; };
namespace Broker { class Manager; }

namespace acquire {

    class OrbConnection {
        static std::atomic< OrbConnection * > instance_;
        static std::mutex mutex_;
        OrbConnection();
        ~OrbConnection();
    public:
        static OrbConnection * instance();

        bool initialize();
        void shutdown();

        typedef std::vector< adplugin::orbServant * > orbservant_vector_type;
        typedef std::vector< adplugin::orbServant * >::iterator iterator;
        typedef std::vector< adplugin::orbServant * >::const_iterator const_iterator;

        bool empty() const { return orbServants_.empty(); }

        Broker::Manager * brokerManager();

        iterator begin() { return orbServants_.begin(); }
        iterator end() { return orbServants_.end(); }
        const_iterator begin() const { return orbServants_.begin(); }
        const_iterator end() const { return orbServants_.end(); }
    private:
        bool initialized_;
        std::vector< adplugin::orbServant * > orbServants_;
        void initialize_broker();
    };
}


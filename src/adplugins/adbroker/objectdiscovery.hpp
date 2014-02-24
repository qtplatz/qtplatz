// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
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

#ifndef OBJECTDISCOVERY_H
#define OBJECTDISCOVERY_H

#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace boost { namespace asio { class io_service; } }

namespace acewrapper { class iorQuery; }

namespace adbroker {

    class ObjectDiscovery {
    public:
        ~ObjectDiscovery();
        ObjectDiscovery();
        bool open();
        void close();
        void register_lookup( const std::string& name, const std::string& ident );
        bool unregister_lookup( const std::string& ident, std::string& name );

    private:
        void reply_handler( const std::string&, const std::string& );
        std::unique_ptr< boost::asio::io_service > io_service_;
		std::unique_ptr< acewrapper::iorQuery > iorQuery_;
        std::vector< std::thread > threads_;
        bool suspend_;
		std::map< std::string, std::string > list_;
        std::mutex mutex_;
    };

}

#endif // OBJECTDISCOVERY_H

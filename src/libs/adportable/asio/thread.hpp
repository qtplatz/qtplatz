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

// I've been using std::thread for host boost::asio::io_service::run() without problem 
// on Windows VS2013, Mac clang++ 5.x, 6.0 and Linux gcc 4.7 for x86 and x64 for years.
// However, I hit segmentatin fault error when compiled for ARM (R_PI) using 
// arm-linux-gnueabihf- toolchain.

// here is the workaround

#if defined __ARM_EABI__

#include <boost/thread.hpp>

namespace adportable { namespace asio {
    typedef boost::thread thread;
}
}

#else

#include <thread>

namespace adportable {
    namespace asio {
        typedef std::thread thread;
    }
}

#endif


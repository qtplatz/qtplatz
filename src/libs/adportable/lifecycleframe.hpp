/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef LIFECYCLEFRAME_HPP
#define LIFECYCLEFRAME_HPP

#include <adportable/protocollifecycle.hpp>

namespace adportable {

    class LifeCycleFrame {
    public:
        boost::uint16_t endian_mark;    // 0xfffe
        boost::uint16_t proto_version;  // 0x0001
        boost::uint16_t ctrl;           // 0
        boost::uint16_t hoffset;        // 8
        boost::uint32_t command;        // CONN_SYN etc.
        LifeCycleFrame( protocol::LifeCycleCommand cmd = protocol::NOTHING );
    };

}

#endif // LIFECYCLEFRAME_HPP

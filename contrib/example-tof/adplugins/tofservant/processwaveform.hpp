/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#include <tofinterface/tofC.h>
#include <boost/noncopyable.hpp>
#include <vector>
#include <memory>

namespace tofinterface { class tofDATA; }

namespace tofservant {

    class traceObserver_i;
    class profileObserver_i;

    class ProcessWaveform : boost::noncopyable {
        size_t npos_;
        uint32_t wellKnownEvents_;
        std::shared_ptr< tofinterface::tofDATA > data_;
    public:
        ~ProcessWaveform();
        ProcessWaveform( std::shared_ptr< tofinterface::tofDATA >& );
        bool push_traces( std::vector< std::shared_ptr< traceObserver_i > >& );
        bool push_profile( profileObserver_i * );
    };

}


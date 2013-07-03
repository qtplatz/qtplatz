/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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

#ifndef tofDEVICEID_HPP
#define tofDEVICEID_HPP

#include "cstdint.hpp"
#include <string>
#include <vector>

namespace tofinterface {

    class tofDeviceId {
    public:
	class Configuration {
	public:
	    Configuration();
	    Configuration( const Configuration& );
	    void configured( bool );
	    bool configured() const;
	    void option_type( const std::string& ); // IonSource, Autosampler, LC, GC, etc
	    void option_name( const std::string& );
	private:
	    bool configured_;
	    std::string option_type_;
	    std::string option_name_;
	};

        tofDeviceId();
        tofDeviceId( const tofDeviceId& );

        uint32_t protocolId() const;
        void manufacturer( const std::string& );
        const std::string& manufacturer() const;
        void revision( const std::string& );
        const std::string& revision() const;
        const std::vector< Configuration >& configurations() const;
        std::vector< Configuration >& configurations();
    private:
        const uint32_t protocolId_;
        std::string manufacturer_;
        std::string revision_;
        std::vector< Configuration > configurations_;
    };
    
}

#endif // tofDEVICEID_HPP

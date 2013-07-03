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

#ifndef tofMASSCOMMAND_HPP
#define tofMASSCOMMAND_HPP

#include "cstdint.hpp"
#include <boost/array.hpp>

namespace tofinterface {

    class tofMassCommand {
    public:
	tofMassCommand();
        tofMassCommand( const tofMassCommand& );
        uint32_t protocolId() const;

		void masscommand( const boost::array< uint16_t, 65536 >& );
		const boost::array< uint16_t, 65536 >&  masscommand() const;

        void dcx( const boost::array< uint16_t, 65536 >& );
        const boost::array< uint16_t, 65536 >&  dcx() const;

        void dcy( const boost::array< uint16_t, 65536 >& );
        const boost::array< uint16_t, 65536 >&  dcy() const;

    private:
        const uint32_t protocolId_;
		boost::array< uint16_t, 65536 > masscommand_;
		boost::array< uint16_t, 65536 > dcx_;
		boost::array< uint16_t, 65536 > dcy_;
    };

}

#endif // tofMASSCOMMAND_HPP

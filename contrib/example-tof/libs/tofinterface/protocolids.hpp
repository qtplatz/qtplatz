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

#ifndef PROTCOLIDS_HPP
#define PROTCOLIDS_HPP

#include "cstdint.hpp"

namespace tofinterface {

# define FPGA_PROT_ID(X,Y,Z,T) static_cast<uint32_t>( X << 24 | Y << 16 | Z << 8 | T )

    namespace Constants {

	enum fpgaProtocolId {
	    ID00 = FPGA_PROT_ID( 'I','D','0','0')
	    , SSET = FPGA_PROT_ID( 'S','S','E','T')
	    , QACT = FPGA_PROT_ID( 'Q','A','C','T')
	    , MCMD = FPGA_PROT_ID( 'M','C','M','D')
	    , SMTD = FPGA_PROT_ID( 'S','M','T','D')
	    , DATA = FPGA_PROT_ID( 'D','A','T','A')
	    , DBG0 = FPGA_PROT_ID( 'D','B','G','1') // control method
	};

    }

}

#endif // PROTCOLIDS_HPP

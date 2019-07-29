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

#include "tofscanlaw.hpp"
#include <boost/assert.hpp>

using namespace tofspectrometer;

tofScanLaw::tofScanLaw()
{
}

double
tofScanLaw::getMass( double secs, int type ) const
{
	(void)secs;
	(void)type;
	BOOST_ASSERT( 0 );
	return 0;
}

double
tofScanLaw::getTime( double mass, int type ) const
{
	(void)mass;
	(void)type;
	BOOST_ASSERT( 0 );
	return 0;
}

double
tofScanLaw::getMass( double secs, double fLength ) const
{
	(void)secs;
	(void)fLength;
	BOOST_ASSERT( 0 );
	return 0;
}

double
tofScanLaw::getTime( double mass, double fLength ) const
{
	(void)mass;
	(void)fLength;
	BOOST_ASSERT( 0 );
	return 0;
}


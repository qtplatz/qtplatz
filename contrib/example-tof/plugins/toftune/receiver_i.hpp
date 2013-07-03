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

#ifndef RECEIVER_I_HPP
#define RECEIVER_I_HPP

#include <adinterface/receiverS.h>

namespace toftune {

namespace Internal {

	template<class T> class Receiver_i : public POA_Receiver {
		T& parent_;
		~Receiver_i() { };

	public:
		Receiver_i( T& t ) : parent_( t ) {};
     
		void message( Receiver::eINSTEVENT msg, CORBA::ULong value ) {
			parent_.onMessage( msg, static_cast< unsigned long >( value ) );
		}

		void log( const EventLog::LogMessage& log ) {
			parent_.onLog( log );
		}
		void shutdown() {}
		void debug_print( CORBA::Long pri, CORBA::Long cat, const char * text ) {
			parent_.onPrint( pri, cat, text);
		}

		TOF::Session_var session_;
		Instrument::eInstStatus status_;

	};

}
}

#endif // RECEIVER_I_HPP

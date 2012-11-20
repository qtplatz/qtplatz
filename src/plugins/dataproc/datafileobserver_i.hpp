/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
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

#ifndef DATAFILEOBSERVER_I_HPP
#define DATAFILEOBSERVER_I_HPP

#include <adinterface/signalobserverS.h>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
	class LCMSDataset;
	class Chromatogram;
}

namespace dataproc {

	class datafileObserver_i : public virtual POA_SignalObserver::Observer {
	public:
		~datafileObserver_i();
		datafileObserver_i( const adcontrols::LCMSDataset& accessor );

		virtual ::SignalObserver::Description * getDescription (void);
		virtual ::CORBA::Boolean setDescription ( const ::SignalObserver::Description & desc );
		virtual ::CORBA::ULong objId();
		virtual void assign_objId( CORBA::ULong oid );
		virtual ::CORBA::Boolean connect( ::SignalObserver::ObserverEvents_ptr cb
			                             , ::SignalObserver::eUpdateFrequency frequency
										 , const CORBA::WChar * );
		virtual ::CORBA::Boolean disconnect( ::SignalObserver::ObserverEvents_ptr cb );
		virtual ::CORBA::Boolean isActive (void);
		virtual ::SignalObserver::Observers * getSiblings (void);
		virtual ::CORBA::Boolean addSibling ( ::SignalObserver::Observer_ptr observer);
        virtual ::SignalObserver::Observer * findObserver( CORBA::ULong objId, CORBA::Boolean recursive );
		virtual void uptime ( ::CORBA::ULongLong_out usec );
        virtual void uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest );
		virtual ::CORBA::Boolean readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer);
		virtual ::CORBA::WChar * dataInterpreterClsid (void);
        virtual ::CORBA::Long posFromTime( CORBA::ULongLong usec );
	private:
		const adcontrols::LCMSDataset& accessor_;
		unsigned long objId_;
		SignalObserver::Description desc_;
		boost::scoped_ptr< adcontrols::Chromatogram > tic_;
	};

}

#endif // DATAFILEOBSERVER_I_HPP

/**************************************************************************
** Copyright (C) 2010-2012 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC / Advanced Instrumentation Project
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

#include "datafileobserver_i.hpp"
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/portable_binary_oarchive.hpp>

using namespace dataproc;

datafileObserver_i::~datafileObserver_i()
{
}

datafileObserver_i::datafileObserver_i( const adcontrols::LCMSDataset& accessor ) : accessor_( accessor )
                                                                                  , objId_(0)
																				  , tic_( new adcontrols::Chromatogram() )
{
	desc_.trace_method = SignalObserver::eTRACE_SPECTRA;
	desc_.spectrometer = SignalObserver::eMassSpectrometer;
	desc_.trace_id = CORBA::wstring_dup( L"MS.PROFILE" );
	desc_.trace_display_name = CORBA::wstring_dup( L"MS Spectra" );
	desc_.axis_x_label = CORBA::wstring_dup( L"m/z" );
	desc_.axis_y_label = CORBA::wstring_dup( L"Intens" );
	desc_.axis_x_decimals = 2;
	desc_.axis_y_decimals = 0;
	accessor_.getTIC( 0, *tic_ );
}

::SignalObserver::Description * 
datafileObserver_i::getDescription ( void )
{
	SignalObserver::Description_var var( new SignalObserver::Description( desc_ ) );
    return var._retn();
}

::CORBA::Boolean
datafileObserver_i::setDescription ( const ::SignalObserver::Description & desc )
{
    desc_ = desc;
	return true;
}

::CORBA::ULong
datafileObserver_i::objId()
{
	return objId_;
}

void
datafileObserver_i::assign_objId( CORBA::ULong oid )
{
	objId_ = oid;
}

::CORBA::Boolean
datafileObserver_i::connect( ::SignalObserver::ObserverEvents_ptr /* cb */
							, ::SignalObserver::eUpdateFrequency /* frequency */
							, const CORBA::Char * )
{
	return false;
}

::CORBA::Boolean
datafileObserver_i::disconnect( ::SignalObserver::ObserverEvents_ptr /* cb */ )
{
	return false;
}

::CORBA::Boolean
datafileObserver_i::isActive (void)
{
	return true;
}

::SignalObserver::Observers *
datafileObserver_i::getSiblings (void)
{
	return 0;
}

::CORBA::Boolean 
datafileObserver_i::addSibling ( ::SignalObserver::Observer_ptr /* observer */)
{
	return false;
}

::SignalObserver::Observer *
datafileObserver_i::findObserver( CORBA::ULong /* objId */, CORBA::Boolean /* recursive */)
{
	return 0;
}

void
datafileObserver_i::uptime ( ::CORBA::ULongLong_out usec )
{
	usec = 0;
}

void
datafileObserver_i::uptime_range( ::CORBA::ULongLong_out oldest, ::CORBA::ULongLong_out newest )
{
	oldest = 0;
    newest = 0;
}

struct DataReadBuffer : public std::streambuf {
	::SignalObserver::DataReadBuffer_var& var_;
	size_t count_;
	size_t size_;
	unsigned char * p_;

	DataReadBuffer( SignalObserver::DataReadBuffer_var& var ) : var_( var ), count_(0), size_(0), p_(0) {
		resize();
	}

	void resize() { 
		var_->array.length( var_->array.length() + 2048 );
		size_ = var_->array.length() * sizeof( CORBA::Long );
		p_ = reinterpret_cast< unsigned char * >( var_->array.get_buffer() );
	}

	virtual int_type overflow ( int_type c ) {
		if ( count_ >= size_ )
			resize();
		p_[ count_++ ] = c;
		return c;
	}

	virtual std::streamsize xsputn( const char * s, std::streamsize num ) {
		while ( count_ + num >= size_ )
			resize();
		for ( int i = 0; i < num; ++i )
			p_[ count_++ ] = *s++;
		return num;
	}
};

::CORBA::Boolean
datafileObserver_i::readData ( ::CORBA::Long pos, ::SignalObserver::DataReadBuffer_out dataReadBuffer )
{
	adcontrols::MassSpectrum ms;
	if ( accessor_.getSpectrum(0, pos, ms ) ) {
		SignalObserver::DataReadBuffer_var res = new SignalObserver::DataReadBuffer;
		res->uptime = tic_->getTimeArray()[ pos ] * 60 * 1000000; // us
        res->pos = pos;
        res->events = 0;

		DataReadBuffer buffer( res );
		std::ostream ostm( &buffer );
		adcontrols::MassSpectrum::archive( ostm, ms );
		dataReadBuffer = res._retn();
		return true;
	}
	return false;
}

::CORBA::WChar *
datafileObserver_i::dataInterpreterClsid (void)
{
	return CORBA::wstring_dup( L"adcontrols::MassSpectrum" );
}

::CORBA::Long
datafileObserver_i::posFromTime( CORBA::ULongLong usec )
{
	using adportable::array_wrapper;

	if ( tic_ ) {
		double minutes = double( usec ) / ( 60.0 * 1.0e6 );
		array_wrapper< const double > times( tic_->getTimeArray(), tic_->size() );
		array_wrapper< const double >::iterator it = std::lower_bound( times.begin(), times.end(), minutes );
		long pos = std::distance( times.begin(), it );
		return pos;
	}
	return 0;
}

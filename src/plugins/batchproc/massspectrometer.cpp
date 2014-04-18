/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "massspectrometer.hpp"
#include "datainterpreter.hpp"
#include "importdata.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adportable/serializer.hpp>
#include <adportable/bzip2.hpp>
#include <adlog/logger.hpp>
#include <boost/exception/all.hpp>

namespace batchproc {
    class MassSpectrometerException : public boost::exception, public std::exception {};
}

using namespace batchproc;

MassSpectrometer::~MassSpectrometer(void)
{
}

MassSpectrometer::MassSpectrometer( adcontrols::datafile * datafile ) : adcontrols::MassSpectrometer( datafile )
                                                                      , interpreter_( std::make_shared< DataInterpreter >() )
                                                                      , accessor_(0)
{
}

#include <compiler/diagnostic_push.h>
#include <compiler/disable_extra.h>
MassSpectrometer::MassSpectrometer( const MassSpectrometer& t ) : adcontrols::MassSpectrometer( t )
                                                                , interpreter_( t.interpreter_ )
                                                                , accessor_( t.accessor_ )
{
}
#include <compiler/diagnostic_pop.h>

bool
MassSpectrometer::subscribe( const adcontrols::LCMSDataset& data )
{
    accessor_ = &data;
    return true;
}
        
const wchar_t * 
MassSpectrometer::name() const 
{
    return L"batchproc::import";
}

const adcontrols::ScanLaw&
MassSpectrometer::getScanLaw() const 
{
    BOOST_THROW_EXCEPTION( std::bad_cast() );
}

const adcontrols::DataInterpreter&
MassSpectrometer::getDataInterpreter() const 
{
    return *interpreter_;
}

std::shared_ptr<adcontrols::ScanLaw>
MassSpectrometer::scanLaw( const adcontrols::MSProperty& ) const
{
    return 0;
}

void
MassSpectrometer::setCalibration( int mode, const adcontrols::MSCalibrateResult& )
{
    (void)mode;
}

const std::shared_ptr< adcontrols::MSCalibrateResult >
MassSpectrometer::getCalibrateResult( size_t idx ) const 
{
    (void)idx;
    return 0;
}

const adcontrols::MSCalibration *
MassSpectrometer::findCalibration( int mode ) const 
{
    (void)mode;
    return 0;
}

const import_continuum_massarray& 
MassSpectrometer::continuum_massarray() const
{
    if ( !continuum_massarray_ )
        const_cast< MassSpectrometer *>(this)->load_continuum_massarray();
    return *continuum_massarray_;
}

bool
MassSpectrometer::load_continuum_massarray()
{
    if ( !accessor_ && datafile_ )
        datafile_->accept( *this );

    if ( accessor_ ) {
        continuum_massarray_ = std::make_shared< import_continuum_massarray >();

        uint32_t objid = accessor_->findObjId( L"MS.PROFILE" );
        if ( objid > 0 ) {
            uint64_t fcn;
            std::vector<char> data, meta;
            if ( accessor_->getRaw( objid, 0, fcn, data, meta ) ) {
                if ( !meta.empty() ) {
                    if ( adportable::bzip2::is_a( meta.data(), meta.size() ) ) {
                        ADTRACE() << "load continuum massarray w/ decompress";
                        std::string ar;
                        adportable::bzip2::decompress( ar, meta.data(), meta.size() );
                        try {
                            ADTRACE() << "deserialize continuum massarray size=" << ar.size();
                            adportable::serializer< import_continuum_massarray >::deserialize( *continuum_massarray_, ar.data(), ar.size() );
                        } catch ( boost::archive::archive_exception& ex ) {
                            ADTRACE() << "archive_exception::code : " << ex.code << " " << ex.what();
                            BOOST_THROW_EXCEPTION( ex );
                        }
                    } else {
                        ADTRACE() << "load continuum massarray w/o decompress";
                        try {
                            adportable::serializer< import_continuum_massarray >::deserialize( *continuum_massarray_, meta.data(), meta.size() );
                        } catch ( boost::archive::archive_exception& ex ) {
                            ADWARN() << boost::current_exception_diagnostic_information() << ex.code;
                            BOOST_THROW_EXCEPTION( ex );
                        }
                    }
                    return true;
                }
            }
        }
    }
    return false;
}

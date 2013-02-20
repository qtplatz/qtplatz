// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
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

#include "dataprocessor.hpp"
#include "ifileimpl.hpp"
#include "constants.hpp"
#include "sessionmanager.hpp"
#include "dataprochandler.hpp"
#include "datafileobserver_i.hpp"
#include <adcontrols/datafile.hpp>
#include <qtwrapper/qstring.hpp>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/ifile.h>
#include <portfolio/portfolio.hpp>
#include <portfolio/folium.hpp>
#include <portfolio/folder.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adutils/processeddata.hpp>

#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/waveform.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adplugin/orbmanager.hpp>
#include <boost/filesystem/path.hpp>
#include <stack>
#include <qdebug.h>

using namespace dataproc;

namespace dataproc {

    struct DataprocessorImpl {
        static adcontrols::MassSpectrumPtr findAttachedMassSpectrum( portfolio::Folium& folium );
        static bool applyMethod( portfolio::Folium&, const adcontrols::IsotopeMethod& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::MSCalibrateMethod& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::MSCalibrateMethod&, const adcontrols::MSAssignedMasses& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::CentroidMethod&, const adcontrols::MassSpectrum& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::PeakMethod&, const adcontrols::Chromatogram& );
    };

    struct methodselector {
        const adcontrols::ProcessMethod& m;
        methodselector( const adcontrols::ProcessMethod& _m ) : m(_m) {}
        template<class T> bool append( adcontrols::ProcessMethod& method ) {
            const T * p = m.find<T>();
            if ( p ) {
                method.appendMethod( *p );
                return true;
            }
            assert( p );
            return false;
        }
    };
}

Dataprocessor::~Dataprocessor()
{
}

Dataprocessor::Dataprocessor() : portfolio_( new portfolio::Portfolio() )
{
}

bool
Dataprocessor::create(const QString& token )
{
    boost::filesystem::path path( qtwrapper::wstring::copy( token ) );
    path.replace_extension( L".adfs" );
    portfolio_->create_with_fullpath( path.wstring() );

    adcontrols::datafile * file = adcontrols::datafile::create( path.wstring() );
    if ( file ) {
        ifileimpl_.reset( new IFileImpl( file, *this ) );
        file->accept( *ifileimpl_ );
        file->accept( *this );
        return true;
    }
    return false;
}

bool
Dataprocessor::open(const QString &fileName )
{
    adcontrols::datafile * file = adcontrols::datafile::open( qtwrapper::wstring::copy( fileName ), true );
    if ( file ) {
        ifileimpl_.reset( new IFileImpl( file, *this ) );
        file->accept( *ifileimpl_ );
        file->accept( *this );
        return true;
    }
    return false;
}

Core::IFile *
Dataprocessor::ifile()
{
    return static_cast<Core::IFile *>( ifileimpl_.get() );
}

adcontrols::datafile&
Dataprocessor::file()
{
    return ifileimpl_->file();
}

const std::wstring&
Dataprocessor::filename() const
{
	return ifileimpl_->file().filename();
}

const adcontrols::LCMSDataset *
Dataprocessor::getLCMSDataset()
{
    return ifileimpl_->getLCMSDataset();
}

portfolio::Portfolio
Dataprocessor::getPortfolio()
{
    return * portfolio_;
}

void
Dataprocessor::setCurrentSelection( portfolio::Folder& folder )
{
	idActiveFolium_ = folder.id();
}

void
Dataprocessor::setCurrentSelection( portfolio::Folium& folium )
{
    if ( folium.empty() ) {

        folium = file().fetch( folium.id(), folium.dataClass() );
 
        portfolio::Folio attachs = folium.attachments();
        for ( portfolio::Folio::iterator it = attachs.begin(); it != attachs.end(); ++it ) {
            if ( it->empty() )
                *it = file().fetch( it->id(), it->dataClass() );
        }
    }
    idActiveFolium_ = folium.id();
    SessionManager::instance()->selectionChanged( this, folium );
}

namespace dataproc {

    // dispatch method
    struct doSpectralProcess : public boost::static_visitor<bool> {
        const adutils::MassSpectrumPtr& ptr_;

        portfolio::Folium& folium;

        doSpectralProcess( const adutils::MassSpectrumPtr& p, portfolio::Folium& f ) : ptr_(p), folium(f) {
        }

        template<typename T> bool operator () ( T& ) const {
            adportable::debug(__FILE__, __LINE__) << "doSpectraolProcess( " << typeid( T ).name() << ") -- ignored";
            return false;
        }

        bool operator () ( const adcontrols::CentroidMethod& m ) const {
            return DataprocessorImpl::applyMethod( folium, m, *ptr_ );
        }

        bool operator () ( const adcontrols::IsotopeMethod& m ) const {
            return DataprocessorImpl::applyMethod( folium, m );
        }

        bool operator () ( const adcontrols::MSCalibrateMethod& m ) const {
            return DataprocessorImpl::applyMethod( folium, m );
        }
    };

    // dispatch method
    struct doChromatogramProcess : public boost::static_visitor<bool> {
        const adutils::ChromatogramPtr& ptr_;

        portfolio::Folium& folium;

        doChromatogramProcess( const adutils::ChromatogramPtr& p, portfolio::Folium& f ) : ptr_(p), folium(f) {
        }

        template<typename T> bool operator () ( T& ) const {
            adportable::debug(__FILE__, __LINE__) << "doChromatogramProcess( " << typeid( T ).name() << ") -- ignored";
            return false;
        }

        bool operator () ( const adcontrols::PeakMethod& m ) const {
            return DataprocessorImpl::applyMethod( folium, m, *ptr_ );
        }
    };


    // dispatch data type
    struct processIt : public boost::static_visitor<bool> {
        const adcontrols::ProcessMethod::value_type& m_;
        portfolio::Folium& folium_;

        processIt( const adcontrols::ProcessMethod::value_type& m, portfolio::Folium& f ) : m_(m), folium_(f) {
        }

        template<typename T> bool operator ()( T& ) const {
            return false;
        }

        bool operator () ( adutils::MassSpectrumPtr& ptr ) const {
            return boost::apply_visitor( doSpectralProcess(ptr, folium_), m_ );
        }

        bool operator () ( adutils::ChromatogramPtr& ) const {
            // todo:  add doChromatographicProcess
            return false;
        }
    };
    //-----
}

void
Dataprocessor::applyProcess( const adcontrols::ProcessMethod& m, ProcessType procType )
{
    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );

    if ( folium ) {
        adcontrols::ProcessMethod method;

        methodselector selector( m );

        if ( procType == CentroidProcess ) {
            selector.append< adcontrols::CentroidMethod >( method );
        } else if ( procType == IsotopeProcess ) {
            selector.append< adcontrols::CentroidMethod >( method );
            selector.append< adcontrols::IsotopeMethod >( method );
        } else if ( procType == CalibrationProcess ) {
            // should not be here
        } else if ( procType == PeakFindProcess ) {
            selector.append< adcontrols::PeakMethod >( method );
        }

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
     
        for ( adcontrols::ProcessMethod::vector_type::const_iterator it = method.begin(); it != method.end(); ++it )
            boost::apply_visitor( processIt(*it, folium), data );
        
#ifdef _DEBUG
        std::wstring xml = portfolio_->xml();
#endif
        SessionManager::instance()->selectionChanged( this, folium );
    } else {
        // no selected folium, peak find to raw TIC
        if ( procType == PeakFindProcess ) {
            Dataprocessor * d_processor = SessionManager::instance()->getActiveDataprocessor();
            if ( d_processor ) {
                const adcontrols::LCMSDataset * dataset = d_processor->getLCMSDataset();
                if ( dataset ) {
                    adcontrols::Chromatogram c;
                    if ( dataset->getTIC( 0, c ) ) {
                        c.addDescription( adcontrols::Description( L"Create", L"TIC" ) );
                        d_processor->addChromatogram( c, m );
                    }
                }
            }
        }
    }
}

void
Dataprocessor::applyCalibration( const adcontrols::ProcessMethod& m )
{
    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );
    if ( folium ) {
        //----------------------- take centroid and calibration method w/ modification ---------------------
        const adcontrols::MSCalibrateMethod * pCalibMethod = m.find< adcontrols::MSCalibrateMethod >();
        const adcontrols::CentroidMethod * pCentroidMethod = m.find< adcontrols::CentroidMethod >();
        if ( pCentroidMethod == 0 || pCalibMethod == 0 ) {
            assert( pCalibMethod && pCentroidMethod );
            return;
        }
        
        adcontrols::CentroidMethod centroidMethod( *pCentroidMethod );
        centroidMethod.centroidAreaIntensity( false );  // force hight for easy overlay

        adcontrols::ProcessMethod method;
        method.appendMethod( centroidMethod );
        method.appendMethod( *pCalibMethod );

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
        if ( data.type() == typeid( adutils::MassSpectrumPtr ) )
            addCalibration( * boost::get< adutils::MassSpectrumPtr >( data ), method );
    }
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"MSCalibration" );
    portfolio::Folium folium = folder.addFolium( L"CalibrantSpectrum" );

    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    adcontrols::waveform::fft::lowpass_filter( *ms );
    // workaround for unsure acquired mass range (before calibration)
    ms->setAcquisitionMassRange( 1.0, 1000.0 );
    //
    folium.assign( ms, ms->dataClass() );

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( doSpectralProcess( ms, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
}

void
Dataprocessor::applyCalibration( const adcontrols::ProcessMethod& m
                                 , const adcontrols::MSAssignedMasses& assigned )
{
    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );

    if ( folium ) {
        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
        if ( data.type() != typeid( adutils::MassSpectrumPtr ) )
            return;
        adutils::MassSpectrumPtr profile( boost::get< adutils::MassSpectrumPtr >( data ) );

        portfolio::Folio attachments = folium.attachments();
        portfolio::Folio::iterator it
            = portfolio::Folium::find_first_of<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
        if ( it == attachments.end() )
            return;
        
        adutils::MassSpectrumPtr centroid = boost::any_cast< adutils::MassSpectrumPtr >( *it );
        
        const adcontrols::MSCalibrateMethod * pCalibMethod = m.find< adcontrols::MSCalibrateMethod >();
        if ( pCalibMethod )
            addCalibration( *profile, *centroid, *pCalibMethod, assigned );
    }
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& profile
                               , const adcontrols::MassSpectrum& centroid
                               , const adcontrols::MSCalibrateMethod& calibMethod
                               , const adcontrols::MSAssignedMasses& assigned )
{
    portfolio::Folder folder = portfolio_->addFolder( L"MSCalibration" );
    portfolio::Folium folium = folder.addFolium( L"CalibrantSpectrum" );
    
    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( profile ) );  // deep copy
    folium.assign( ms, ms->dataClass() );

    // copy centroid spectrum, which manually assigned by user and result stored in 'assined'
    portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
    adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum( centroid ) ); // deep copy
    att.assign( pCentroid, pCentroid->dataClass() );

    // todo: process method to be added
    
    if ( DataprocessorImpl::applyMethod( folium, calibMethod, assigned ) ) 
        SessionManager::instance()->updateDataprocessor( this, folium );
}

portfolio::Folium
Dataprocessor::addSpectrum( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectra" );

    const adcontrols::Descriptions& descs = src.getDescriptions();
    std::wstring name;
    for ( size_t i = 0; i < descs.size(); ++i )
        name += descs[i].text();

    portfolio::Folium folium = folder.addFolium( name );
    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    folium.assign( ms, ms->dataClass() );

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( doSpectralProcess( ms, folium ), *it );

#if defined _DEBUG
	std::wstring xml = portfolio_->xml();
#endif

    SessionManager::instance()->updateDataprocessor( this, folium );
	return folium;
}

portfolio::Folium
Dataprocessor::addChromatogram( const adcontrols::Chromatogram& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Chromatograms" );

    const adcontrols::Descriptions& descs = src.getDescriptions();
    std::wstring name;
    for ( size_t i = 0; i < descs.size(); ++i )
        name += descs[i].text();

    portfolio::Folium folium = folder.addFolium( name );
    adutils::ChromatogramPtr c( new adcontrols::Chromatogram( src ) );  // profile, deep copy
	folium.assign( c, c->dataClass() );

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
		boost::apply_visitor( doChromatogramProcess( c, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
	return folium;
}

SignalObserver::Observer_ptr
Dataprocessor::observer()
{
	adplugin::ORBManager * mgr = adplugin::ORBManager::instance();
	if ( mgr && fileObserver_ ) {
		CORBA::Object_var obj = mgr->poa()->servant_to_reference( fileObserver_.get() );
		return SignalObserver::Observer::_narrow( obj );
	}
	return 0;
}

///////////////////////////
bool
Dataprocessor::subscribe( const adcontrols::LCMSDataset& data )
{
	// datafile has a raw (acquired) data stream
	// data should point same object with ifileimpl_;
	fileObserver_.reset( new datafileObserver_i( data ) );
	return true;
/* 
   size_t nfcn = data.getFunctionCount();
   for ( size_t i = 0; i < nfcn; ++i ) {
        adcontrols::Chromatogram c;
        if ( data.getTIC( i, c ) )
            ; // ticVec_.push_back( c );
    }
*/
}

bool
Dataprocessor::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::wstring xml = processed.xml();
    portfolio_.reset( new portfolio::Portfolio( xml ) );
#if defined _DEBUG
    // portfolio_->save( L"portfolio.xml" );
#endif
    return true;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

adcontrols::MassSpectrumPtr
DataprocessorImpl::findAttachedMassSpectrum( portfolio::Folium& folium )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr ptr;

    Folium::vector_type atts = folium.attachments();
    Folium::vector_type::iterator it = Folium::find_first_of< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() )
        Folium::get< adcontrols::MassSpectrumPtr >( ptr, *it);

    return ptr; // can be null
}

bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::IsotopeMethod& m )
{
    adcontrols::MassSpectrumPtr prev = findAttachedMassSpectrum( folium );
    // copy centroid result if exist, for meta data copy
    if ( prev ) {
        adcontrols::MassSpectrumPtr pResult( new adcontrols::MassSpectrum( *prev ) );
        if ( DataprocHandler::doIsotope( *pResult, m ) ) {
            portfolio::Folium att = folium.addAttachment( L"Isotope Cluster" );
            att.assign( pResult, pResult->dataClass() );
        }
        return true;
    }
    return false;
}

bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::MSCalibrateMethod& m )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr pProfile = boost::any_cast< adcontrols::MassSpectrumPtr >( folium );

    Folium::vector_type atts = folium.attachments();
    Folium::vector_type::iterator it = Folium::find_first_of< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() ) {
        adcontrols::MassSpectrumPtr pCentroid = boost::any_cast< adcontrols::MassSpectrumPtr >( static_cast<boost::any&>( *it ) );
        if ( pCentroid ) {
            adcontrols::MSCalibrateResultPtr pResult( new adcontrols::MSCalibrateResult );
            if ( DataprocHandler::doMSCalibration( *pResult, *pCentroid, m ) ) {
                portfolio::Folium att = folium.addAttachment( L"Calibrate Result" );
                att.assign( pResult, pResult->dataClass() );
                
                // rewrite calibration := change m/z asssing on the spectrum
                pCentroid->setCalibration( pResult->calibration() );
                
                // update profile mass array
                if ( pProfile ) {
                    pProfile->setCalibration( pResult->calibration() );
                    const std::vector<double>& coeffs = pResult->calibration().coeffs();
                    for ( size_t i = 0; i < pProfile->size(); ++i ) {
                        double tof = pProfile->getTime( i );
                        double mq = adcontrols::MSCalibration::compute( coeffs, tof );
                        pProfile->setMass( i, mq * mq );
                    }
                }
            } else {
                // set centroid result for user manual peak assign possible
                portfolio::Folium att = folium.addAttachment( L"Calibrate Result" );
                att.assign( pResult, pResult->dataClass() );
            }
            return true;
        }
    }
    return false;
}

bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium
                                , const adcontrols::MSCalibrateMethod& m
                                , const adcontrols::MSAssignedMasses& assigned )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr pProfile = boost::any_cast< adcontrols::MassSpectrumPtr >( folium );

    Folium::vector_type atts = folium.attachments();
    Folium::vector_type::iterator it = Folium::find_first_of< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );

    if ( it != atts.end() ) {
        adcontrols::MassSpectrumPtr pCentroid = boost::any_cast< adcontrols::MassSpectrumPtr >( static_cast<boost::any&>( *it ) );
        if ( pCentroid ) {
            adcontrols::MSCalibrateResultPtr pResult( new adcontrols::MSCalibrateResult );
            if ( DataprocHandler::doMSCalibration( *pResult, *pCentroid, m, assigned ) ) {

                portfolio::Folium att = folium.addAttachment( L"Calibrate Result" );
                att.assign( pResult, pResult->dataClass() );
                
                // rewrite calibration := change m/z asssing on the spectrum
                pCentroid->setCalibration( pResult->calibration() );
                
                // update profile mass array
                if ( pProfile ) {
                    pProfile->setCalibration( pResult->calibration() );
                    const std::vector<double>& coeffs = pResult->calibration().coeffs();
                    if ( ! coeffs.empty() ) {
                        for ( size_t i = 0; i < pProfile->size(); ++i ) {
                            double tof = pProfile->getTime( i );
                            double mq = adcontrols::MSCalibration::compute( coeffs, tof );
                            if ( mq > 0.0 )
                                pProfile->setMass( i, mq * mq );
                            else
                                pProfile->setMass( i, -( mq * mq ) );
                        }
                    }
                }
            }
            return true;
        }
    }
    return false;
}

bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium
                                , const adcontrols::CentroidMethod& m
                                , const adcontrols::MassSpectrum& profile )
{
    portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
    adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum );

    if ( DataprocHandler::doCentroid( *pCentroid, profile, m ) ) {
        att.assign( pCentroid, pCentroid->dataClass() );

        adcontrols::ProcessMethodPtr ptr( new adcontrols::ProcessMethod() );
        ptr->appendMethod( m );
        att.addAttachment( L"Process Method" ).assign( ptr, ptr->dataClass() );

        return true;
    }

    return false;
}


// static
bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::PeakMethod& m, const adcontrols::Chromatogram& c )
{
    portfolio::Folium att = folium.addAttachment( L"Peak Result" );
    adcontrols::PeakResultPtr pResult( new adcontrols::PeakResult() );
    
    if ( DataprocHandler::doFindPeaks( *pResult, c, m ) ) {
        att.assign( pResult, pResult->dataClass() );
        
        adcontrols::ProcessMethodPtr ptr( new adcontrols::ProcessMethod() );
        ptr->appendMethod( m );
        att.addAttachment( L"Process Method" ).assign( ptr, ptr->dataClass() );
        
        return true;
    }
    return false;
}

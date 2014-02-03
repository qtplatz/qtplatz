// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this
** file in accordance with the MS-Cheminformatics Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and MS-Cheminformatics.
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
#include "mainwindow.hpp"
#include "dataprocessworker.hpp"
#include <adcontrols/datafile.hpp>
#include <adcontrols/lockmass.hpp>
#include <qtwrapper/qstring.hpp>
#include <extensionsystem/pluginmanager.h>
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
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/waveform.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/targetingmethod.hpp>
//#include <adorbmgr/orbmgr.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/profile.hpp>
#include <adportable/debug.hpp>
#include <adportable/serializer.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adfs/adfs.hpp>
#include <adfs/file.hpp>
#include <adfs/attributes.hpp>
#include <adutils/fsio.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <stack>
#include <fstream>
#include <QMessageBox>

using namespace dataproc;

namespace dataproc {

    struct DataprocessorImpl {
        static adcontrols::MassSpectrumPtr findAttachedMassSpectrum( portfolio::Folium& folium );
        static bool applyMethod( portfolio::Folium&, const adcontrols::IsotopeMethod& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::MSCalibrateMethod& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::MSCalibrateMethod&, const adcontrols::MSAssignedMasses& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::CentroidMethod&, const adcontrols::MassSpectrum& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::PeakMethod&, const adcontrols::Chromatogram& );
        static bool fixupDataInterpreterClsid( portfolio::Folium& );
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
    try {
        if ( adcontrols::datafile * file = adcontrols::datafile::open( fileName.toStdWString(), false ) ) {
            ifileimpl_.reset( new IFileImpl( file, *this ) );
            try {
                file->accept( *ifileimpl_ );
                file->accept( *this );
                return true;
            } catch ( boost::exception& ex ) {
                adportable::debug(__FILE__, __LINE__) << boost::diagnostic_information( ex );
            } catch ( std::exception& ex ) {
                adportable::debug(__FILE__, __LINE__) << ex.what();
            } catch ( ... ) {
                adportable::debug(__FILE__, __LINE__) << "got an exception '...'";
            }
        }
    } catch ( boost::exception& ex ) {
        adportable::debug(__FILE__, __LINE__) << boost::diagnostic_information( ex );
    } catch ( ... ) {
        adportable::debug(__FILE__, __LINE__) << "got an exception '...'";
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

bool
Dataprocessor::load( const std::wstring& path, const std::wstring& id )
{
    return ifileimpl_->file().loadContents( path, id, *this );
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
    qtwrapper::waitCursor wait;
	fetch( folium );
    idActiveFolium_ = folium.id();
    SessionManager::instance()->selectionChanged( this, folium );
}

bool
Dataprocessor::fetch( portfolio::Folium& folium )
{
	if ( folium.empty() ) {
		try {
			folium = file().fetch( folium.id(), folium.dataClass() );
			portfolio::Folio attachs = folium.attachments();
			for ( auto att: attachs ) {
				if ( att.empty() )
					fetch( att ); // recursive call make sure for all blongings load up in memory.
			}
		} catch ( std::bad_cast& ) {
		}
	}
	return true;
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
            // adportable::debug(__FILE__, __LINE__) << "doChromatogramProcess( " << typeid( T ).name() << ") -- ignored";
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

        bool operator () ( adutils::ChromatogramPtr& ptr ) const {
            return boost::apply_visitor( doChromatogramProcess(ptr, folium_), m_ );
        }
    };
    //-----
}

void
Dataprocessor::applyProcess( const adcontrols::ProcessMethod& m, ProcessType procType )
{
    adportable::debug(__FILE__, __LINE__) << "applyProcess: " << procType;

    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );
    if ( folium )
        applyProcess( folium, m, procType );
}

void
Dataprocessor::applyProcess( portfolio::Folium& folium
                             , const adcontrols::ProcessMethod& m, ProcessType procType )
{
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
            // adportable::debug(__FILE__, __LINE__) << "============== select PeakFindProcess";
            selector.append< adcontrols::PeakMethod >( method );
        }

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
     
        for ( auto it = method.begin(); it != method.end(); ++it )
            boost::apply_visitor( processIt(*it, folium), data );
        SessionManager::instance()->selectionChanged( this, folium );
		ifileimpl_->setModified();
    }
}

void
Dataprocessor::removeCheckedItems()
{
    for ( auto& folder: portfolio_->folders() ) {
        for ( auto& folium: folder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"false" ) {
                folder.removeFolium( folium );
            }
        }
    }
}

void
Dataprocessor::sendCheckedSpectraToCalibration( Dataprocessor * processor )
{
    qtwrapper::waitCursor wait;

    // obtain data from source (may not be my-self)
    portfolio::Folder spectra = processor->portfolio().findFolder( L"Spectra" );

    adcontrols::ProcessMethod mproc;
    MainWindow::instance()->getProcessMethod( mproc );
    const adcontrols::MSCalibrateMethod * pCalibMethod = mproc.find< adcontrols::MSCalibrateMethod >();
    const adcontrols::CentroidMethod * pCentroidMethod = mproc.find< adcontrols::CentroidMethod >();
    if ( pCentroidMethod == 0 || pCalibMethod == 0 ) {
        assert( pCalibMethod && pCentroidMethod );
        return;
    }
    adcontrols::CentroidMethod centroidMethod( *pCentroidMethod ); // copy
    centroidMethod.centroidAreaIntensity( false );  // force hight for overlay with profile

    // add to my-self
    portfolio::Folder calibFolder = portfolio_->addFolder( L"MSCalibration" );

    for ( auto& folium: spectra.folio() ) {
        if ( folium.attribute( L"isChecked" ) == L"true" ) {
            if ( folium.empty() )
                processor->fetch( folium );

            if ( auto profile = portfolio::get< adutils::MassSpectrumPtr >( folium ) ) {

                const adcontrols::CentroidMethod * hasCentroidMethod = 0;
                
                portfolio::Folio atts = folium.attachments();
                auto itCentroid = std::find_if( atts.begin(), atts.end(), []( portfolio::Folium& f ) {
                        return f.name() == Constants::F_CENTROID_SPECTRUM;
                    });
                if ( itCentroid != atts.end() ) {
                    portfolio::Folio atts2 = itCentroid->attachments();
					if ( portfolio::Folium fmethod
                         = portfolio::find_first_of( itCentroid->attachments(), []( portfolio::Folium& a ){
                                 return portfolio::is_type< adcontrols::ProcessMethodPtr >( a ); 
                             }) ) {
                        if ( auto ptr = portfolio::get< adcontrols::ProcessMethodPtr>( fmethod ) ) 
                            hasCentroidMethod = ptr->find< adcontrols::CentroidMethod >();
					}
                }

                adcontrols::ProcessMethod method;
                centroidMethod.noiseFilterMethod( pCentroidMethod->noiseFilterMethod() ); // restore NF
                method.appendMethod( hasCentroidMethod ? *hasCentroidMethod : centroidMethod );
                method.appendMethod( *pCalibMethod );

                if ( profile->getDescriptions().size() == 0 ) 
                    profile->addDescription( adcontrols::Description( L"create", folium.name() ) );

                addCalibration( *profile, method );
            }
        }
    }
	ifileimpl_->setModified();
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
		if ( adutils::MassSpectrumPtr ptr = boost::get< adutils::MassSpectrumPtr >( data ) ) {
			if ( ptr->getDescriptions().size() == 0 ) 
				ptr->addDescription( adcontrols::Description( L"create", folium.name() ) );

			addCalibration( * boost::get< adutils::MassSpectrumPtr >( data ), method );
		}
		ifileimpl_->setModified();
    }
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    const adcontrols::Descriptions& descs = src.getDescriptions();
    std::wstring name;
    for ( size_t i = 0; i < descs.size(); ++i )
        name += descs[i].text();
    
    portfolio::Folder folder = portfolio_->addFolder( L"MSCalibration" );
    portfolio::Folium folium = folder.addFolium( name );

	SessionManager::instance()->updateDataprocessor( this, folium );

	if ( const adcontrols::MSCalibrateMethod * pCalibMethod = m.find< adcontrols::MSCalibrateMethod >() ) {
		std::pair<double, double> range = std::make_pair( pCalibMethod->lowMass(), pCalibMethod->highMass() );
		
		adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
		const adcontrols::MassSpectrum& tail = ms->numSegments() == 0 ? *ms : ms->getSegment( ms->numSegments() - 1 );
		double lMass = ms->getMass( 0 );
		double hMass = tail.getMass( tail.size() - 1 );
		// workaround for unsure acquired mass range (before calibration)
		range.first = std::min( range.first, lMass );
		range.second = std::max( range.second, hMass );
		ms->setAcquisitionMassRange( range.first, range.second );
        //
		folium.assign( ms, ms->dataClass() );

		for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
			boost::apply_visitor( doSpectralProcess( ms, folium ), *it );

		SessionManager::instance()->updateDataprocessor( this, folium );
		ifileimpl_->setModified();
	}
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
            = portfolio::Folium::find<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
        if ( it == attachments.end() )
            return;
        
        adutils::MassSpectrumPtr centroid = boost::any_cast< adutils::MassSpectrumPtr >( *it );
        
        const adcontrols::MSCalibrateMethod * pCalibMethod = m.find< adcontrols::MSCalibrateMethod >();
        if ( pCalibMethod )
            addCalibration( *profile, *centroid, *pCalibMethod, assigned );
    }
	ifileimpl_->setModified();
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& profile
                               , const adcontrols::MassSpectrum& centroid
                               , const adcontrols::MSCalibrateMethod& calibMethod
                               , const adcontrols::MSAssignedMasses& assigned )
{
    portfolio::Folder folder = portfolio_->addFolder( L"MSCalibration" );
    std::wstring name;
    const adcontrols::Descriptions& descs = centroid.getDescriptions();
    for ( size_t i = 0; i < descs.size(); ++i )
        name += descs[ i ].text();
    name += L", manually assigned";

    portfolio::Folium folium = folder.addFolium( name );
    
    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( profile ) );  // deep copy
    folium.assign( ms, ms->dataClass() );

    // copy centroid spectrum, which manually assigned by user and result stored in 'assined'
    portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
    adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum( centroid ) ); // deep copy
    att.assign( pCentroid, pCentroid->dataClass() );

    // todo: process method to be added
    
    if ( DataprocessorImpl::applyMethod( folium, calibMethod, assigned ) ) 
        SessionManager::instance()->updateDataprocessor( this, folium );

	ifileimpl_->setModified();
}

void
Dataprocessor::applyCalibration( const adcontrols::ProcessMethod& m
                                 , const adcontrols::MSAssignedMasses& assigned
								 , portfolio::Folium& folium )
{
	const adcontrols::MSCalibrateMethod * mcalib = m.find< adcontrols::MSCalibrateMethod >();
	assert( mcalib && folium );
	if ( mcalib && folium ) {
		adcontrols::MassSpectrumPtr profile;
		if ( ! portfolio::Folium::get< adcontrols::MassSpectrumPtr >( profile,  folium ) )
			return;

        portfolio::Folio attachments = folium.attachments();
        portfolio::Folio::iterator it
            = portfolio::Folium::find<adcontrols::MassSpectrumPtr>( attachments.begin(), attachments.end() );
        if ( it == attachments.end() )
            return;
		adutils::MassSpectrumPtr centroid = boost::any_cast< adutils::MassSpectrumPtr >( *it );
		// replace calibration
		if ( DataprocessorImpl::applyMethod( folium, *mcalib, assigned ) )
			SessionManager::instance()->updateDataprocessor( this, folium );

    }
}

void
Dataprocessor::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& calibration )
{
    if ( portfolio::Folder folder = portfolio_->findFolder( L"Spectra" ) ) {

        for ( portfolio::Folium folium: folder.folio() ) {
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr > ( folium ) ) {

				if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                    DataprocHandler::apply_calibration( *ptr, calibration.calibration() );

                    portfolio::Folio atts = folium.attachments();

                    for ( portfolio::Folium& att: atts ) {
                        if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( att ) ) {

							if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( att ) ) {

								DataprocHandler::apply_calibration( *ptr, calibration.calibration() );
                                
                                if ( auto fchild = portfolio::find_first_of( att.attachments(), []( portfolio::Folium& child ){
                                            return portfolio::is_type< adcontrols::MSPeakInfoPtr >( child );} ) ) {
                                    auto pkinfo = portfolio::get< adcontrols::MSPeakInfoPtr >( fchild );
                                    DataprocHandler::reverse_copy( *pkinfo, *ptr );
                                }
                            }
						}
					}
                    
                }
            }
        }
    }
	file().applyCalibration( dataInterpreterClsid, calibration );
    ifileimpl_->setModified();
    MainWindow::instance()->dataMayChanged(); // notify for dockwidgets
}

void
Dataprocessor::lockMassHandled( const std::wstring& foliumId
                                , const adcontrols::MassSpectrumPtr& ms
                                , const adcontrols::lockmass& lockmass )
{
	if ( portfolio::Folium folium = this->portfolio().findFolium( foliumId ) ) {
        
        bool verified = false;
        if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

            portfolio::Folio atts = folium.attachments();
            auto it = std::find_if( atts.begin(), atts.end(), []( portfolio::Folium& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; });
            if ( it != atts.end() ) {
                auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( *it );
                if ( centroid == ms ) {
                    verified = true;
                    // update attached peakinfo
                    if ( auto fchild = portfolio::find_first_of( it->attachments(), []( portfolio::Folium& child ){
                                return portfolio::is_type< adcontrols::MSPeakInfoPtr >( child );} ) ) {
                        auto pkinfo = portfolio::get< adcontrols::MSPeakInfoPtr >( fchild );
                        DataprocHandler::reverse_copy( *pkinfo, *centroid );
                    }
                }
            }
            if ( verified ) {
                auto it = std::find_if( atts.begin(), atts.end(), []( portfolio::Folium& f ){ return f.name() == Constants::F_DFT_FILTERD; });
                if ( it != atts.end() ) {
                    if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( *it ) )
                        lockmass( *ptr );
                }

                lockmass( *ptr ); // update profile spectrum
                ifileimpl_->setModified();
            }
        }
	}
}

void
Dataprocessor::formulaChanged()
{
    ifileimpl_->setModified();
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

    SessionManager::instance()->updateDataprocessor( this, folium );
	return folium;
}

portfolio::Folium
Dataprocessor::addChromatogram( const adcontrols::Chromatogram& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Chromatograms" );

    std::wstring name;
	for ( auto& desc: src.getDescriptions() ) {
		if ( !name.empty() )
			name += L"/";
		name += desc.text();
	}

    portfolio::Folium folium = folder.addFolium( name );
    adutils::ChromatogramPtr c( new adcontrols::Chromatogram( src ) );  // profile, deep copy
	folium.assign( c, c->dataClass() );
    
    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
		boost::apply_visitor( doChromatogramProcess( c, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
	return folium;
}

portfolio::Folium
Dataprocessor::addSpectrogram( std::shared_ptr< adcontrols::MassSpectra >& spectra )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectrograms" );
    portfolio::Folium folium = folder.addFolium( L"Spectrogram" );  // "Spectrograms/Spectrogram"
	folium.assign( spectra, spectra->dataClass() );
    SessionManager::instance()->updateDataprocessor( this, folium ); // this cause an error on emit since this is in worker thread
	return folium;
}

void
Dataprocessor::createSpectrogram()
{
	DataprocessWorker::instance()->createSpectrogram( this );
}

void
Dataprocessor::clusterSpectrogram()
{
	DataprocessWorker::instance()->clusterSpectrogram( this );
}

///////////////////////////
bool
Dataprocessor::subscribe( const adcontrols::LCMSDataset& data )
{
    (void)data;
	return true;
}

bool
Dataprocessor::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::string xml = processed.xml();
    portfolio_.reset( new portfolio::Portfolio( xml ) );
    return true;
}

bool
Dataprocessor::onFileAdded( const std::wstring& path, adfs::file& file )
{
	boost::filesystem::path pathname( path );
	if ( std::distance( pathname.begin(), pathname.end() ) < 3 )
		return false;
	if ( path.find( L"/Processed" ) == path.npos ) 
		return false;
	std::wstring foldername = pathname.filename().wstring();

	portfolio::Folder folder = portfolio_->addFolder( foldername );
	portfolio::Folium folium = folder.addFolium( static_cast< adfs::attributes& >(file).name() );

    std::for_each( file.begin(), file.end(), [&]( const adfs::attributes::vector_type::value_type& a ){
            folium.setAttribute( a.first, a.second );            
        });
	// for ( const auto attrib = file.begin(); attrib != file.end(); ++attrib ) 
	// 	folium.setAttribute( attrib->first, attrib->second );
	
	SessionManager::instance()->updateDataprocessor( this, folium );

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
    Folium::vector_type::iterator it = Folium::find< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
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

//static
bool
DataprocessorImpl::fixupDataInterpreterClsid( portfolio::Folium& folium )
{
    std::vector< std::wstring > models = adcontrols::MassSpectrometer::get_model_names();
    if ( models.empty() ) {
        QMessageBox::warning(0, "Calibration", "It has no mass spectrometer installed so that is not possible to re-assin masses");
        return false;
    }
    std::string dataInterpreter = adportable::utf::to_utf8( models[0] );
    QMessageBox::warning(0, "Calibration"
                         , (boost::format("Data has no mass spectrometer scan law, assuming %1%") % dataInterpreter).str().c_str());
    
    auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium );
	adcontrols::segment_wrapper<> segments( *profile );
    for ( auto& fms: segments ) {
        adcontrols::MSProperty prop( fms.getMSProperty() );
        prop.setDataInterpreterClsid( dataInterpreter.c_str() );
        fms.setMSProperty( prop );
    }
    
    portfolio::Folium::vector_type atts = folium.attachments();
    std::for_each( atts.begin(), atts.end(), [&]( portfolio::Folium& att ){
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( static_cast< boost::any& >( att ) ) ) {
                auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( att );
                adcontrols::segment_wrapper<> segments( *centroid );
                for ( auto& fms: segments ) {
                    adcontrols::MSProperty prop( fms.getMSProperty() );                    
                    prop.setDataInterpreterClsid( dataInterpreter.c_str() );
                    fms.setMSProperty( prop );                
                }
            }
        });
    return true;
}

bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::MSCalibrateMethod& m )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr pProfile = boost::any_cast< adcontrols::MassSpectrumPtr >( folium );
	if ( ! adcontrols::MassSpectrometer::find( pProfile->getMSProperty().dataInterpreterClsid() ) ) {
		fixupDataInterpreterClsid( folium );
	}

    Folium::vector_type atts = folium.attachments();
	auto attCentroid = Folium::find< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
    if ( attCentroid != atts.end() ) {
        adcontrols::MassSpectrumPtr pCentroid = boost::any_cast< adcontrols::MassSpectrumPtr >( attCentroid->data() );
        if ( pCentroid ) {
			adcontrols::segment_wrapper<> segs( *pCentroid );
            
			double y = adcontrols::segments_helper::max_intensity( *pCentroid );
			double y_threshold = y * m.minimumRAPercent() / 100.0;
            
			// threshold color filter
			for ( auto& ms: segs ) {
				for ( size_t i = 0; i < ms.size(); ++i ) {
					if ( ms.getIntensity( i ) < y_threshold )
						ms.setColor( i, 16 ); // transparent
				}
			}

            adcontrols::MSCalibrateResultPtr pCalibResult( new adcontrols::MSCalibrateResult );
            portfolio::Folium fCalibResult = folium.addAttachment( L"Calibrate Result" );

            if ( DataprocHandler::doMSCalibration( *pCalibResult, *pCentroid, m ) ) {
                fCalibResult.assign( pCalibResult, pCalibResult->dataClass() );
            } else {
                // set centroid result for user manual peak assign possible
                fCalibResult.assign( pCalibResult, pCalibResult->dataClass() );
            }

            adcontrols::ProcessMethodPtr method( new adcontrols::ProcessMethod() );
            method->appendMethod( m );
            fCalibResult.addAttachment( L"Process Method" ).assign( method, method->dataClass() );
            
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
    Folium::vector_type::iterator it = Folium::find< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );

    if ( it != atts.end() ) {
        adcontrols::MassSpectrumPtr pCentroid = boost::any_cast< adcontrols::MassSpectrumPtr >( static_cast<boost::any&>( *it ) );
        if ( pCentroid ) {
            adcontrols::MSCalibrateResultPtr pResult( new adcontrols::MSCalibrateResult );
            if ( DataprocHandler::doMSCalibration( *pResult, *pCentroid, m, assigned ) ) {

                portfolio::Folium att = folium.addAttachment( L"Calibrate Result" );
                att.assign( pResult, pResult->dataClass() );
                
                // rewrite calibration := change m/z asssing on the spectrum
                pCentroid->setCalibration( pResult->calibration(), true );
                
                // update profile mass array
                if ( pProfile )
                    pProfile->setCalibration( pResult->calibration(), true );
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
    portfolio::Folium att = folium.addAttachment( Constants::F_CENTROID_SPECTRUM );
    std::shared_ptr< adcontrols::MassSpectrum > pCentroid = std::make_shared< adcontrols::MassSpectrum >();
    bool centroid;

	// make sure no 'processed profile' data exist
	folium.removeAttachment( Constants::F_DFT_FILTERD ); // L"DFT Low Pass Filtered Spectrum";
    folium.removeAttachment( Constants::F_MSPEAK_INFO );

    std::shared_ptr< adcontrols::MSPeakInfo > pkInfo( std::make_shared< adcontrols::MSPeakInfo >() );

    if ( m.noiseFilterMethod() == adcontrols::CentroidMethod::eDFTLowPassFilter ) {
        adcontrols::MassSpectrumPtr profile2( new adcontrols::MassSpectrum( profile ) );
        adcontrols::segment_wrapper< adcontrols::MassSpectrum > segments( *profile2 );
        for ( auto& ms: segments ) {
            adcontrols::waveform::fft::lowpass_filter( ms, m.cutoffFreqHz() );
            double base(0), rms(0);
            const double * intens = ms.getIntensityArray();
            adportable::spectrum_processor::tic( uint32_t( ms.size() ), intens, base, rms );
            for ( size_t i = 0; i < ms.size(); ++i )
                ms.setIntensity( i, intens[i] - base );
        }
        portfolio::Folium filterd = folium.addAttachment( Constants::F_DFT_FILTERD );
        profile2->addDescription( adcontrols::Description( L"process", Constants::F_DFT_FILTERD ) );
        filterd.assign( profile2, profile2->dataClass() );

        centroid = DataprocHandler::doCentroid( *pkInfo, *pCentroid, *profile2, m );

    } else {
        centroid = DataprocHandler::doCentroid( *pkInfo, *pCentroid, profile, m );
    }

    if ( centroid ) {
        pCentroid->addDescription( adcontrols::Description( L"process", L"Centroid" ) );
        att.assign( pCentroid, pCentroid->dataClass() );
        adcontrols::ProcessMethodPtr ptr( new adcontrols::ProcessMethod() );
        ptr->appendMethod( m );
        att.addAttachment( L"Process Method" ).assign( ptr, ptr->dataClass() );

        att.addAttachment( L"MSPeakInfo" ).assign( pkInfo, pkInfo->dataClass() );

        return true;
    }
    return false;
}


// static
bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::PeakMethod& m, const adcontrols::Chromatogram& c )
{
    portfolio::Folium att = folium.addAttachment( L"Peak Result" );
    adcontrols::PeakResultPtr pResult( std::make_shared< adcontrols::PeakResult >() );

    if ( DataprocHandler::doFindPeaks( *pResult, c, m ) ) {
        att.assign( pResult, pResult->dataClass() );
        
        adcontrols::ProcessMethodPtr ptr( new adcontrols::ProcessMethod() );
        ptr->appendMethod( m );
        att.addAttachment( L"Process Method" ).assign( ptr, ptr->dataClass() );
        
        return true;
    }
    return false;
}


// static
const adcontrols::ProcessMethodPtr
Dataprocessor::findProcessMethod( const portfolio::Folium& folium )
{
    portfolio::Folio atts = folium.attachments();
    auto fMethod = portfolio::Folium::find< adcontrols::ProcessMethodPtr >( atts.begin(), atts.end() );
    if ( fMethod != atts.end() ) {
        const adcontrols::ProcessMethodPtr pMethod = boost::any_cast< adcontrols::ProcessMethodPtr >( fMethod->data() );
        return pMethod;
    }
    return adcontrols::ProcessMethodPtr(0);
}

// static
bool
Dataprocessor::saveMSCalibration( portfolio::Folium& folium )
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< char >() );
    dir /= "data";
    if ( ! boost::filesystem::exists( dir ) ) 
        if ( ! boost::filesystem::create_directories( dir ) )
            return false;

    boost::filesystem::path fname = dir / "default.msclb";
    
    adfs::filesystem dbf;
    if ( !adutils::fsio::create( dbf, fname.wstring() ) )
        return false;

    // argment folium should be profile spectrum
    portfolio::Folio atts = folium.attachments();
    auto it = portfolio::Folium::find< adcontrols::MSCalibrateResultPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() ) {
        const adcontrols::MSCalibrateResultPtr ptr = boost::any_cast< adcontrols::MSCalibrateResultPtr >( it->data() );
        adutils::fsio::save_mscalibfile( dbf, *ptr );

        // for debugging convension
        std::string xml;
        if ( adportable::xml_serializer< adcontrols::MSCalibrateResult >::serialize( *ptr, xml ) ) {
            fname.replace_extension( ".msclb.xml" );
            std::ofstream of( fname.string() );
            of << xml;
        }
    }

    it = portfolio::Folium::find< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() ) {
        const adcontrols::MassSpectrumPtr ptr = boost::any_cast< adcontrols::MassSpectrumPtr >( it->data() );
        adutils::fsio::save_mscalibfile( dbf, *ptr );
    }

    return true;
}

// static
bool
Dataprocessor::saveMSCalibration( const adcontrols::MSCalibrateResult& calibResult, const adcontrols::MassSpectrum& calibSpectrum )
{
    boost::filesystem::path dir( adportable::profile::user_data_dir< char >() );
    dir /= "data";
    if ( ! boost::filesystem::exists( dir ) ) 
        if ( ! boost::filesystem::create_directories( dir ) )
            return false;

    boost::filesystem::path fname = dir / "default.msclb";
    
    adfs::filesystem dbf;
    if ( !adutils::fsio::create( dbf, fname.wstring() ) )
        return false;

    try {
        adutils::fsio::save_mscalibfile( dbf, calibResult );
    } catch ( std::exception& ex ) {
        QMessageBox::warning( 0, "saveMSCalibration", (boost::format("%1% @%2% #%3%") % ex.what() % __FILE__ % __LINE__).str().c_str() );
    }
    try {
        adutils::fsio::save_mscalibfile( dbf, calibSpectrum );
    } catch ( std::exception& ex ) {
        QMessageBox::warning( 0, "saveMSCalibration", (boost::format("%1% @%2% #%3%") % ex.what() % __FILE__ % __LINE__).str().c_str() );
    }

    // for debugging convension
    std::string xml;
    if ( adportable::xml_serializer< adcontrols::MSCalibrateResult >::serialize( calibResult, xml ) ) {
        fname.replace_extension( ".msclb.xml" );
        std::ofstream of( fname.string() );
        of << xml;
    }

    return true;
}

// static
bool
Dataprocessor::loadMSCalibration( const std::wstring& filename, adcontrols::MSCalibrateResult& r, adcontrols::MassSpectrum& ms )
{
    boost::filesystem::path path( filename );
    if ( ! boost::filesystem::exists( path ) ) 
        return false;

    adfs::filesystem fs;
	if ( ! fs.mount( path.wstring().c_str() ) )
        return false;

    if ( adutils::fsio::load_mscalibfile( fs, r ) && 
         adutils::fsio::load_mscalibfile( fs, ms ) ) {
        return true;
    }
    return false;
}



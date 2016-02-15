// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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
#include "constants.hpp"
#include "sessionmanager.hpp"
#include "dataprochandler.hpp"
#include "mainwindow.hpp"
#include "dataprocessworker.hpp"

#include <adportable/array_wrapper.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/lcmsdataset.hpp>
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
#include <adcontrols/processmethod.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/spectrogram.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adfs/adfs.hpp>
#include <adfs/file.hpp>
#include <adfs/attributes.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
//#include <adportable/serializer.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adutils/fsio.hpp>
#include <adutils/processeddata.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/id.h>
#include <coreplugin/idocument.h>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/qstring.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <stack>
#include <fstream>
#include <QMessageBox>
#include <QFontMetrics>
#include <QApplication>

using namespace dataproc;

namespace dataproc {

    struct DataprocessorImpl {
        static adcontrols::MassSpectrumPtr findAttachedMassSpectrum( portfolio::Folium& folium );
        static bool applyMethod( portfolio::Folium&, const adcontrols::IsotopeMethod& );
        static bool applyMethod( portfolio::Folium&, const adcontrols::TargetingMethod& );
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
    disconnect( this, &Dataprocessor::onNotify, MainWindow::instance(), &MainWindow::handleWarningMessage );
}

Dataprocessor::Dataprocessor() : portfolio_( new portfolio::Portfolio() )
                               , rawDataset_( 0 )
                               , modified_( false )
{
    connect( this, &Dataprocessor::onNotify, MainWindow::instance(), &MainWindow::handleWarningMessage );
}

void
Dataprocessor::setDisplayName( const QString& fullpath )
{
    QFontMetrics fm( QApplication::fontMetrics() );
    IDocument::setDisplayName( fm.elidedText( fullpath, Qt::ElideLeft, 200 ) );
}

void
Dataprocessor::setModified( bool modified )
{
    modified_ = modified;
}

// IDocument

bool
Dataprocessor::isModified() const
{
    return modified_;
}

bool
Dataprocessor::isFileReadOnly() const
{
    return false;
}

Core::IDocument::ReloadBehavior
Dataprocessor::reloadBehavior( ChangeTrigger state, ChangeType type ) const
{
    return IDocument::BehaviorSilent;
}


bool
Dataprocessor::save( QString * errorString, const QString& filename, bool /* autoSave */)
{
	boost::filesystem::path path( file_->filename() ); // original name

    if ( filename.isEmpty() && path.extension() == ".adfs" ) {
        // Save
        if ( file_->saveContents( L"/Processed", *portfolio_ ) ) {
            setModified( false );
            return true;
        }
    }
    if ( !filename.isEmpty() )
        path = filename.toStdWString();

    path.replace_extension( L".adfs" );
    if ( boost::filesystem::exists( path ) ) {
        int id = 1;
        do {
            boost::filesystem::path backup( path.branch_path() / boost::filesystem::path( path.stem().string() + (boost::format( "~%1%.adfs" ) % id++).str() ) );
            if ( !boost::filesystem::exists( backup ) ) {
                boost::system::error_code ec;
                boost::filesystem::rename( path, backup, ec );
                if ( ec ) {
                    *errorString = QString::fromStdString( ec.message() );
                    return false;
                }
                break;
            }
        } while ( true );
    }

    // save as 'filename
    std::shared_ptr< adcontrols::datafile > file( adcontrols::datafile::create( path.wstring() ) );

    if ( (file && file->saveContents( L"/Processed", *portfolio_, *file_ )) ) {

        file_ = file;
        setModified( false );

        // for debugging convension
        path.replace_extension( ".xml" );
        boost::filesystem::remove( path );
        pugi::xml_document dom;
        dom.load( portfolio_->xml().c_str() );
        dom.save_file( path.string().c_str() );
        return true;

    }
    return false;
}

bool
Dataprocessor::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

QString
Dataprocessor::defaultPath() const
{
	return adportable::profile::user_data_dir<char>().c_str();
}

QString
Dataprocessor::suggestedFileName() const
{
	boost::filesystem::path path( file_->filename() );
	path.replace_extension( L".adfs" );
    return QString::fromStdWString( path.normalize().wstring() );
}

bool
Dataprocessor::isSaveAsAllowed() const
{
    return true;
}

bool
Dataprocessor::create(const QString& filename )
{
    boost::filesystem::path path( qtwrapper::wstring::copy( filename ) );
    path.replace_extension( L".adfs" );
    portfolio_->create_with_fullpath( path.wstring() );

    auto file = adcontrols::datafile::create( path.wstring() );
    if ( file ) {
        file_.reset( file );
        file_->open( filename.toStdWString() );
        file->accept( *this );
        setDisplayName( filename );
        return true;
    }
    return false;
}

bool
Dataprocessor::open(const QString &filename, QString& emsg )
{
    emsg.clear();

    try {
        if ( adcontrols::datafile * file = adcontrols::datafile::open( filename.toStdWString(), false ) ) {
            file_.reset( file );
            try {
                file->accept( *this );
                setDisplayName( filename );
                setFilePath( filename );
                return true;
            } catch ( boost::exception& ex ) {
                emsg = QString::fromStdString( boost::diagnostic_information( ex ) );
                ADERROR() << boost::diagnostic_information( ex );
            } catch ( std::exception& ex ) {
                emsg = QString::fromStdString( ex.what() );
                ADERROR() << ex.what();
            } catch ( ... ) {
                emsg = QString::fromStdString( "Unidentified exception" );
                ADERROR() << "Unidentified exception received.";
            }
        }
    } catch ( boost::exception& ex ) {
        emsg = QString::fromStdString( boost::diagnostic_information( ex ) );
        ADERROR() << boost::diagnostic_information( ex );
    } catch ( ... ) {
        emsg = QString::fromStdString( boost::current_exception_diagnostic_information() );
        ADERROR() << "got an exception '...'";
    }
    return false;
}

adcontrols::datafile&
Dataprocessor::file()
{
    return *file_;// ifileimpl_->file();
}

const std::wstring&
Dataprocessor::filename() const
{
    return file_->filename();
}

bool
Dataprocessor::load( const std::wstring& path, const std::wstring& id )
{
    // this is used for reload 'acquire' when shanpshot spectrum was added.
    return file_->loadContents( path, id, *this );
}

const adcontrols::LCMSDataset *
Dataprocessor::getLCMSDataset()
{
    return rawDataset_;
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
	fetch( folium );
    idActiveFolium_ = folium.id();
    SessionManager::instance()->selectionChanged( this, folium );
}

portfolio::Folium
Dataprocessor::currentSelection() const
{
	return portfolio_->findFolium( idActiveFolium_ );
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
		} catch ( boost::exception& ex ) {
            ADTRACE() << boost::diagnostic_information( ex );
#if defined _DEBUG || DEBUG
            QMessageBox::information( 0, "Dataprocessor", QString::fromStdString( boost::diagnostic_information( ex ) ) );
#endif
		}
	}
	return true;
}

namespace dataproc {

    // dispatch method
    struct doSpectralProcess : public boost::static_visitor<bool> {
        //const adutils::MassSpectrumPtr& ptr_;
        std::shared_ptr< const adcontrols::MassSpectrum > ptr_;

        portfolio::Folium& folium;

        doSpectralProcess( const adutils::MassSpectrumPtr& p, portfolio::Folium& f ) : ptr_(p), folium(f) {
        }

        doSpectralProcess( std::shared_ptr< const adcontrols::MassSpectrum >& p, portfolio::Folium& f ) : ptr_(p), folium(f) {
        }

        template<typename T> bool operator () ( T& ) const {
            ADTRACE() << "doSpectraolProcess( " << typeid( T ).name() << ") -- ignored";
            return false;
        }

        bool operator () ( const adcontrols::CentroidMethod& m ) const {
            return DataprocessorImpl::applyMethod( folium, m, *ptr_ );
        }

        bool operator () ( const adcontrols::TargetingMethod& m ) const {
            return DataprocessorImpl::applyMethod( folium, m );
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
            // ADTRACE() << "doChromatogramProcess( " << typeid( T ).name() << ") -- ignored";
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
    ADTRACE() << "applyProcess: " << procType;

    portfolio::Folium folium = portfolio_->findFolium( idActiveFolium_ );
    if ( folium )
        applyProcess( folium, m, procType );
    setModified( true );
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
            selector.append< adcontrols::TargetingMethod >( method ); // always does 'targeting' when centroid
        }
        else if ( procType == TargetingProcess ) {
            if ( auto fCentroid = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& f ) {
                        return f.name() == Constants::F_CENTROID_SPECTRUM; } ) ) {
                selector.append< adcontrols::TargetingMethod >( method );
            } else {
                selector.append< adcontrols::CentroidMethod >( method );
                selector.append< adcontrols::TargetingMethod >( method );
            }
        }
        else if ( procType == CalibrationProcess ) {
            // should not be here
        }
        else if ( procType == PeakFindProcess ) {
            selector.append< adcontrols::PeakMethod >( method );
        }

        adutils::ProcessedData::value_type data = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );
     
        for ( auto it = method.begin(); it != method.end(); ++it )
            boost::apply_visitor( processIt(*it, folium), data );
        setModified( true );
        SessionManager::instance()->processed( this, folium );
    }
}

void
Dataprocessor::removeCheckedItems()
{
    for ( auto& folder: portfolio_->folders() ) {
        for ( auto& folium: folder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"false" ) {
                folder.removeFolium( folium );
                setModified( true );
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
                    profile->addDescription( adcontrols::description( L"create", folium.name() ) );

                addCalibration( *profile, method );
            }
        }
    }
    setModified( true );
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
				ptr->addDescription( adcontrols::description( L"create", folium.name() ) );

			addCalibration( * boost::get< adutils::MassSpectrumPtr >( data ), method );
		}
		setModified( true );
    }
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    const adcontrols::descriptions& descs = src.getDescriptions();
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
		setModified( true );
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
	setModified( true );
}

void
Dataprocessor::addCalibration( const adcontrols::MassSpectrum& profile
                               , const adcontrols::MassSpectrum& centroid
                               , const adcontrols::MSCalibrateMethod& calibMethod
                               , const adcontrols::MSAssignedMasses& assigned )
{
    portfolio::Folder folder = portfolio_->addFolder( L"MSCalibration" );
    std::wstring name;
    const adcontrols::descriptions& descs = centroid.getDescriptions();
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

	setModified( true );
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
        setModified( true );
    }
}

void
Dataprocessor::applyCalibration( const std::wstring& dataInterpreterClsid, const adcontrols::MSCalibrateResult& calibration )
{
    if ( portfolio::Folder folder = portfolio_->findFolder( L"Spectra" ) ) {

        setModified( true );

        for ( portfolio::Folium folium: folder.folio() ) {

			this->fetch( folium );
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
    setModified( true );
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
                setModified( true );
            }
        }
	}
}

void
Dataprocessor::formulaChanged()
{
    setModified( true );
}

portfolio::Folium
Dataprocessor::addSpectrum( const adcontrols::MassSpectrum& src, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectra" );

    // name from descriptions : exclude values which key has a pattern of "acquire.protocol.*" that is description for protocol/fcn related
    std::wstring name = src.getDescriptions().make_folder_name( L"^((?!acquire\\.protocol\\.).)*$" );

    if ( auto folium = folder.findFoliumByName( name ) )
        return folium; // already exists

    portfolio::Folium folium = folder.addFolium( name );
    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    folium.assign( ms, ms->dataClass() );

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( doSpectralProcess( ms, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
	return folium;
}

portfolio::Folium
Dataprocessor::addSpectrum( std::shared_ptr< const adcontrols::MassSpectrum >& ptr, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectra" );

    // name from descriptions : exclude values which key has a pattern of "acquire.protocol.*" that is description for protocol/fcn related
    std::wstring name = ptr->getDescriptions().make_folder_name( L"^((?!acquire\\.protocol\\.).)*$" );

    if ( auto folium = folder.findFoliumByName( name ) )
        return folium; // already exists

    portfolio::Folium folium = folder.addFolium( name );
    // adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
    folium.assign( ptr, ptr->dataClass() );

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( doSpectralProcess( ptr, folium ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );
	return folium;
}


portfolio::Folium
Dataprocessor::addChromatogram( const adcontrols::Chromatogram& src, const adcontrols::ProcessMethod& m, bool checked )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Chromatograms" );

    std::wstring name = adcontrols::Chromatogram::make_folder_name( src.getDescriptions() );

    portfolio::Folium folium = folder.addFolium( name );
    adutils::ChromatogramPtr c( new adcontrols::Chromatogram( src ) );  // profile, deep copy
	folium.assign( c, c->dataClass() );
    
    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
		boost::apply_visitor( doChromatogramProcess( c, folium ), *it );

    if ( checked )
        folium.setAttribute( L"isChecked", L"true" );

    SessionManager::instance()->updateDataprocessor( this, folium );

    setModified( true );

	return folium;
}

portfolio::Folium
Dataprocessor::addSpectrogram( std::shared_ptr< adcontrols::MassSpectra >& spectra )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectrograms" );
    portfolio::Folium folium = folder.addFolium( L"Spectrogram" );  // "Spectrograms/Spectrogram"
	folium.assign( spectra, spectra->dataClass() );

    SessionManager::instance()->updateDataprocessor( this, folium );
    setModified( true );

	return folium;
}

portfolio::Folium
Dataprocessor::addSpectrogramClusters( std::shared_ptr< adcontrols::SpectrogramClusters >& clusters )
{
    portfolio::Folder folder = portfolio_->addFolder( L"Spectrograms" );
    portfolio::Folium folium = folder.findFoliumByName( L"Spectrogram" );  // "Spectrograms/Spectrogram"
    if ( folium ) {
        portfolio::Folium att = folium.addAttachment( L"Clusters" );
        att.assign( clusters, clusters->dataClass() );

        SessionManager::instance()->updateDataprocessor( this, folium );
        setModified( true );
    }
	return folium;
}

void
Dataprocessor::createSpectrogram()
{
	DataprocessWorker::instance()->createSpectrogram( this );
    setModified( true );
}

void
Dataprocessor::clusterSpectrogram()
{
	DataprocessWorker::instance()->clusterSpectrogram( this );
    setModified( true );
}

void
Dataprocessor::findPeptide( const adprot::digestedPeptides& digested )
{
	DataprocessWorker::instance()->findPeptide( this, digested );
}

void
Dataprocessor::subtract( portfolio::Folium& base, portfolio::Folium& target )
{
    if ( auto background = portfolio::get< adutils::MassSpectrumPtr >( base ) ) {
        
        if ( auto profile = portfolio::get< adutils::MassSpectrumPtr >( target ) ) {

            if ( profile->isCentroid() || background->isCentroid() )
                return;

            adcontrols::MassSpectrum xms( *profile );
            for ( size_t i = 0; i < xms.size(); ++i )
                xms.setIntensity( i, xms.getIntensity( i ) - background->getIntensity( i ) );

			xms.addDescription( adcontrols::description( L"processed", ( boost::wformat( L"%1% - %2%" ) % target.name() % base.name() ).str() ) );
            addSpectrum( xms, adcontrols::ProcessMethod() );
            setModified( true );
        }
    }
}

///////////////////////////
bool
Dataprocessor::subscribe( const adcontrols::LCMSDataset& data )
{
    rawDataset_ = &data;
	return true;
}

bool
Dataprocessor::subscribe( const adcontrols::ProcessedDataset& processed )
{
    std::string xml = processed.xml();
    portfolio_.reset( new portfolio::Portfolio( xml ) );
    return true;
}

void
Dataprocessor::notify( adcontrols::dataSubscriber::idError, const wchar_t * text )
{
    QString msg( tr( "Instrument module(s) \"%1\" not installed." ).arg( QString::fromStdWString( text ) ) );
    emit onNotify( msg );
}

bool
Dataprocessor::onFileAdded( const std::wstring& path, adfs::file& file )
{
    // reload action for snapshot on acquire

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
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::TargetingMethod& m )
{
    if ( auto fCentroid = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& f ) { return f.name() == Constants::F_CENTROID_SPECTRUM; }) ) {

        if ( adcontrols::MassSpectrumPtr centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fCentroid ) ) {
            
            if ( auto targeting = std::make_shared< adcontrols::Targeting >(m) ) {
                
                if ( (*targeting)(*centroid) ) {
                    
                    fCentroid.removeAttachment( Constants::F_TARGETING );
                    portfolio::Folium att = fCentroid.addAttachment( Constants::F_TARGETING );

                    att.assign( targeting, adcontrols::Targeting::dataClass() );
                    
                    auto mptr = std::make_shared< adcontrols::ProcessMethod >( m );
                    att.addAttachment( L"Process Method" ).assign( mptr, mptr->dataClass() );
                    
                    return true;
                }                    
            }
        }

    }
    return false;
}

bool
DataprocessorImpl::applyMethod( portfolio::Folium& folium, const adcontrols::IsotopeMethod& m )
{
    adcontrols::MassSpectrumPtr prev = findAttachedMassSpectrum( folium );
    // copy centroid result if exist, for meta data copy
    if ( prev ) {
        adcontrols::MassSpectrumPtr pResult( std::make_shared< adcontrols::MassSpectrum >( *prev ) );
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
    auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium );
    std::string diClsid = profile->getMSProperty().dataInterpreterClsid();

    std::vector< std::wstring > models = adcontrols::MassSpectrometer::get_model_names();
    if ( models.empty() ) {
        if ( !diClsid.empty() )
            QMessageBox::warning( 0, QObject::tr( "Calibration" ), QObject::tr( "It has no mass spectrometer for %1 installed so that mass can't be assinged." ).arg( diClsid.c_str() ) );
        else
            QMessageBox::warning( 0, QObject::tr( "Calibration" ), QObject::tr( "It has no mass spectrometer installed so that mass can't be assigned." ) );
        return false;
    }

    std::string dataInterpreter = adportable::utf::to_utf8( models[ 0 ] );
    if ( !diClsid.empty() ) {
        QMessageBox::warning( 0, QObject::tr( "Calibration" ), QObject::tr( "No mass spectrometer class '%1' installed." ).arg( diClsid.c_str() ) );
        return false;
    }
    else {
        QMessageBox::warning( 0, QObject::tr( "Calibration" ), QObject::tr( "Data has no mass spectrometer information, assume %1" ).arg( dataInterpreter.c_str() ) );

        adcontrols::segment_wrapper<> segments( *profile );
        for ( auto& fms : segments ) {
            adcontrols::MSProperty prop( fms.getMSProperty() );
            prop.setDataInterpreterClsid( dataInterpreter.c_str() );
            fms.setMSProperty( prop );
        }

        portfolio::Folium::vector_type atts = folium.attachments();
        std::for_each( atts.begin(), atts.end(), [&] ( portfolio::Folium& att ){
            if ( portfolio::is_type< adcontrols::MassSpectrumPtr >( static_cast<boost::any&>(att) ) ) {
                auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( att );
                adcontrols::segment_wrapper<> segments( *centroid );
                for ( auto& fms : segments ) {
                    adcontrols::MSProperty prop( fms.getMSProperty() );
                    prop.setDataInterpreterClsid( dataInterpreter.c_str() );
                    fms.setMSProperty( prop );
                }
            }
        } );
        return true;
    }
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

            adcontrols::MSCalibrateResultPtr pCalibResult( std::make_shared< adcontrols::MSCalibrateResult >() );
            portfolio::Folium fCalibResult = folium.addAttachment( L"Calibrate Result" );

            if ( DataprocHandler::doMSCalibration( *pCalibResult, *pCentroid, m ) ) {
                fCalibResult.assign( pCalibResult, pCalibResult->dataClass() );
            } else {
                // set centroid result for user manual peak assign possible
                fCalibResult.assign( pCalibResult, pCalibResult->dataClass() );
            }

            adcontrols::ProcessMethodPtr method( std::make_shared< adcontrols::ProcessMethod >( m ) );
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
            adcontrols::waveform_filter::fft4c::lowpass_filter( ms, m.cutoffFreqHz() );
            double base(0), rms(0);
            const double * intens = ms.getIntensityArray();
            adportable::spectrum_processor::tic( uint32_t( ms.size() ), intens, base, rms );
            for ( size_t i = 0; i < ms.size(); ++i )
                ms.setIntensity( i, intens[i] - base );
        }
        portfolio::Folium filterd = folium.addAttachment( Constants::F_DFT_FILTERD );
        profile2->addDescription( adcontrols::description( L"process", Constants::F_DFT_FILTERD ) );
        filterd.assign( profile2, profile2->dataClass() );

        centroid = DataprocHandler::doCentroid( *pkInfo, *pCentroid, *profile2, m );

    } else {
        centroid = DataprocHandler::doCentroid( *pkInfo, *pCentroid, profile, m );
    }

    if ( centroid ) {
        pCentroid->addDescription( adcontrols::description( L"process", L"Centroid" ) );
        att.assign( pCentroid, pCentroid->dataClass() );

        auto mptr = std::make_shared< adcontrols::ProcessMethod >( m ); // Ptr ptr( new adcontrols::ProcessMethod() );
        att.addAttachment( L"Process Method" ).assign( mptr, mptr->dataClass() );

        att.addAttachment( L"MSPeakInfo" ).assign( pkInfo, pkInfo->dataClass() );

        return true;
    } else {
        pCentroid->addDescription( adcontrols::description( L"process", L"Centroid failed" ) );
        att.assign( pCentroid, pCentroid->dataClass() ); // attach it even no peak detected
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
        
        auto mptr = std::make_shared< adcontrols::ProcessMethod >( m );
        att.addAttachment( L"Process Method" ).assign( mptr, mptr->dataClass() );
        
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
        const auto pMethod = boost::any_cast< adcontrols::ProcessMethodPtr >( fMethod->data() );
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
#if 0
        // for debugging convension
        std::string xml;
        if ( adportable::xml_serializer< adcontrols::MSCalibrateResult >::serialize( *ptr, xml ) ) {
            fname.replace_extension( ".msclb.xml" );
            std::ofstream of( fname.string() );
            of << xml;
        }
#endif
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
    //if ( adportable::xml_serializer< adcontrols::MSCalibrateResult >::serialize( calibResult, xml ) ) {
    fname.replace_extension( ".msclb.xml" );
    std::wofstream of( fname.string() );
    adportable::xml::serialize<>()(calibResult, of);

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


void
Dataprocessor::exportXML() const
{
    boost::filesystem::path path( filename() );
    path += ".xml";
    portfolio_->save( path.wstring() );
}

// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2023 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2023 MS-Cheminformatics LLC
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
#include "dataprochandler.hpp"
#include "dataprocessworker.hpp"
#include "document.hpp"
#include "mainwindow.hpp"
#include "sessionmanager.hpp"
#include <adcontrols/annotation.hpp>
#include <adcontrols/annotations.hpp>
#include <adcontrols/baseline.hpp>
#include <adcontrols/baselines.hpp>
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/elementalcompositionmethod.hpp>
#include <adcontrols/histogram.hpp>
#include <adcontrols/isotopemethod.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/lockmass.hpp>
#include <adcontrols/massspectra.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msassignedmass.hpp>
#include <adcontrols/mscalibratemethod.hpp>
#include <adcontrols/mscalibrateresult.hpp>
#include <adcontrols/mscalibration.hpp>
#include <adcontrols/msfractuation.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/peak.hpp>
#include <adcontrols/peakmethod.hpp>
#include <adcontrols/peakresolution.hpp>
#include <adcontrols/peakresult.hpp>
#include <adcontrols/peaks.hpp>
#include <adcontrols/processeddataset.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/samplinginfo.hpp>
#include <adcontrols/segment_wrapper.hpp>
#include <adcontrols/spectrogram.hpp>
#include <adcontrols/targeting.hpp>
#include <adcontrols/targetingmethod.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adcontrols/waveform_filter.hpp>
#include <adfs/adfs.hpp>
#include <adfs/attributes.hpp>
#include <adfs/file.hpp>
#include <adfs/sqlite.hpp>
#include <adlog/logger.hpp>
#include <adportable/array_wrapper.hpp>
#include <adportable/debug.hpp>
#include <adportable/digital_filter.hpp>
#include <adportable/fft4g.hpp>
#include <adportable/float.hpp>
#include <adportable/is_same.hpp>
#include <adportable/json_helper.hpp>
#include <adportable/profile.hpp>
#include <adportable/spectrum_processor.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/utf.hpp>
#include <adprocessor/noise_filter.hpp>
#include <adportable/xml_serializer.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adportfolio/portfolio.hpp>
#include <adutils/acquiredconf_v3.hpp>
#include <adutils/fsio.hpp>
#include <adutils/processeddata.hpp>
#include <adutils/processeddata_t.hpp>
#include <qtwrapper/debug.hpp>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/documentmanager.h>
#include <utils/mimeutils.h>

#if QTC_VERSION <= 0x03'02'81
#include <coreplugin/id.h>
#else
#include <utils/id.h>
#endif
#include <coreplugin/idocument.h>
#include <qtwrapper/waitcursor.hpp>

#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/exception/all.hpp>
#include <chrono>
#include <stack>
#include <fstream>
#include <type_traits>
#include <QApplication>
#include <QFontMetrics>
#include <QMessageBox>

using namespace dataproc;

namespace dataproc {

    struct DataprocessorImpl {
        static bool applyMethod( Dataprocessor *, portfolio::Folium&, const adcontrols::IsotopeMethod& );
        static bool applyMethod( Dataprocessor *, portfolio::Folium&, const adcontrols::TargetingMethod& );

        // calibration
        static bool applyMethod( Dataprocessor *, portfolio::Folium&
                                 , const adcontrols::MSCalibrateMethod& );
        static bool applyMethod( Dataprocessor *, portfolio::Folium&
                                 , const adcontrols::MSCalibrateMethod&
                                 , const adcontrols::MSAssignedMasses&
                                 , std::shared_ptr< adcontrols::MassSpectrometer > );
        // centroid
        static bool applyMethod( Dataprocessor *, portfolio::Folium&
                                 , const adcontrols::CentroidMethod&, const adcontrols::MassSpectrum& );

        // chromatogram
        static bool applyPeakMethod( Dataprocessor *, portfolio::Folium&
                                     , const adcontrols::PeakMethod&
                                     , const adcontrols::Chromatogram& );

        static adcontrols::MassSpectrumPtr findAttachedMassSpectrum( portfolio::Folium& folium );
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

    class Dataprocessor::impl {
    public:
        impl() : modified_( false ) {
        }
        std::wstring idActiveFolium_;
        bool modified_;
    };
}

namespace {
    struct baseline_level_visitor : public boost::static_visitor< bool > {
        double& y0_;
        baseline_level_visitor( double& y0 ) : y0_( y0 ) {}
        template< typename T > bool operator () ( T ) const {
            ADDEBUG() << "unhandled attachment:";
            return false;
        }
        bool operator () ( std::shared_ptr< adcontrols::Chromatogram > ptr ) const {
            if ( ptr->size() > 0 && ( std::abs( ptr->intensity(0) ) > std::numeric_limits<double>::epsilon() ) ) {
                y0_ = ptr->intensity( 0 );
                for ( size_t i = 0; i < ptr->size(); ++i ) {
                    ptr->setIntensity( i, ptr->intensity( i ) - y0_ );
                }
                return true;
            }
            return false;
        }
        bool operator () ( std::shared_ptr< adcontrols::PeakResult > ptr ) const {
            std::for_each( ptr->baselines().begin(), ptr->baselines().end(), [&](auto& bs){ bs.yMove( y0_ ); } );
            std::for_each( ptr->peaks().begin(), ptr->peaks().end(), [&](auto& pk){ pk.yMove( y0_ ); } );
            return true;
        }
    };

    //////////////////////////////////////////////////////////////////////

    struct save_as {
        Dataprocessor& this_;
        QString *errorString_;
        save_as( Dataprocessor& t, QString*& sp ) : this_( t ), errorString_( sp ) {}

        bool rename_backup( const std::filesystem::path& path ) const {
            if ( std::filesystem::exists( path ) ) {
                // rename existing files
                int id = 1;
                do {
                    auto name = path.stem().generic_string() + (boost::format( "~%1%.adfs" ) % id++ ).str();
                    auto backup( path );
                    backup.replace_filename( name );
                    if ( ! std::filesystem::exists( backup ) ) {
                        std::error_code ec;
                        std::filesystem::rename( path, backup, ec );
                        if ( ec ) {
                            *errorString_ = QString::fromStdString( ec.message() );
                            return false;
                        }
                        break;
                    }
                } while ( true );
            }
            return true;
        }

        bool operator()( std::filesystem::path&& path ) const {

            path.replace_extension( L".adfs" );
            if ( rename_backup( path ) ) {
                if ( auto file = adcontrols::datafile::create( path.wstring() ) ) {
                    do {
                        auto fs = std::make_unique< adfs::filesystem >();
                        if ( fs->mount( path ) )
                            adutils::v3::AcquiredConf::create_table_v3( *fs->_ptr() );
                    } while ( 0 );

                    adfs::stmt sql( *this_.db() ); // source db
                    if ( sql.exec( ( boost::format( "ATTACH DATABASE '%1%' AS X" ) % path.string() ).str() ) ) {

                        sql.exec( "INSERT INTO X.ScanLaw SELECT * FROM ScanLaw" );
                        sql.exec( "INSERT INTO X.Spectrometer SELECT * FROM Spectrometer" );
                        sql.exec( "INSERT INTO X.MetaData SELECT * FROM MetaData" );
                        sql.exec( "INSERT INTO X.MSCalibration SELECT * FROM MSCalibration" );

                        sql.exec( "DETACH DATABASE X" );
                    }

                    if ( file->saveContents( L"/Processed", this_.portfolio(), *this_.file() ) ) {
                        this_.setModified( false );
                    }
                    // for debugging
#if 0
                    path.replace_extension( ".xml" );
                    boost::filesystem::remove( path );
                    pugi::xml_document dom;
                    dom.load( portfolio_->xml().c_str() );
                    dom.save_file( path.string().c_str() );
#endif
                    return true;
                }
            }
            return false;
        }
    };

    //////////////////////////////////////////////////////////////////////
}

Dataprocessor::~Dataprocessor()
{
    do {
        auto rpath = std::filesystem::proximate( filename(), adportable::profile::user_data_dir<char>() );
        ADDEBUG() << "## Dataprocessor::dtor closeing file: " << rpath << " ##";
    } while(0);
    disconnect( this, &Dataprocessor::onNotify, MainWindow::instance(), &MainWindow::handleWarningMessage );
}

Dataprocessor::Dataprocessor() : impl_( std::make_unique< impl >() )
{
    setId( Utils::Id( Constants::C_DATAPROCESSOR ) );
    connect( this, &Dataprocessor::onNotify, MainWindow::instance(), &MainWindow::handleWarningMessage );
}

void
Dataprocessor::setDisplayName( const QString& fullpath )
{
#if QTC_VERSION <= 0x03'02'81
    QFontMetrics fm( QApplication::fontMetrics() );
    IDocument::setDisplayName( fm.elidedText( fullpath, Qt::ElideLeft, 200 ) );
#endif
}

void
Dataprocessor::setModified( bool modified )
{
    impl_->modified_ = modified;
}

// IDocument

bool
Dataprocessor::isModified() const
{
    return impl_->modified_;
}

Core::IDocument::ReloadBehavior
Dataprocessor::reloadBehavior( ChangeTrigger state, ChangeType type ) const
{
    return IDocument::BehaviorSilent;
}

Core::IDocument::OpenResult
Dataprocessor::open( QString *errorString
                    , const Utils::FilePath &filePath
                    , const Utils::FilePath &realFilePath)
{
	qtwrapper::waitCursor wait;

    std::string emsg;
    if ( adprocessor::dataprocessor::open( std::filesystem::path( filePath.toString().toStdString() ),  emsg ) ) {
        SessionManager::instance()->addDataprocessor( std::static_pointer_cast<Dataprocessor>(shared_from_this()) );

        Core::DocumentManager::addDocument( this );
        Core::DocumentManager::addToRecentFiles( filePath );
        document::instance()->addToRecentFiles( filePath.toString() );

        setFilePath(filePath);
        setMimeType(Utils::mimeTypeForFile(filePath).name()); // application/vnd.sqlite3

        handleGlobalMSLockChanged();
        emit openFinished( true );

        return Core::IDocument::OpenResult::Success;
    }
    // following error message is being ignored by qt-creator."
    *errorString =
        QString( "file %1 could not be opend.\nReason: %2." ).arg( filePath.toString(), QString::fromStdString( emsg ) );
    qDebug() << "=============" << *errorString;
    emit openFinished( false );
    return Core::IDocument::OpenResult::ReadError;
}

bool
Dataprocessor::save( QString * errorString, const Utils::FilePath& filePath, bool autoSave )
{
    bool isSave = filePath.isEmpty(); // or isSaveAs
    if ( isSave ) {
        std::filesystem::path path( file()->filename() ); // adcontrols::datafile *
        if ( path.extension() == ".adfs" ) {
            if ( file()->saveContents( L"/Processed", portfolio() ) ) {
                setModified( false );
                return true;
            } else {
                *errorString = "Save contents failed.";
            }
        } else {
            *errorString = "Cannot save processed result into a file rather than .adfs file.";
        }
        return false;
    } else {
        // Save As
        return save_as( *this, errorString )( std::filesystem::path( filePath.toString().toStdString() ) );
    }
    return false;
}

bool
Dataprocessor::reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType )
{
    return true;
}

bool
Dataprocessor::isSaveAsAllowed() const
{
    return true;
}

bool
Dataprocessor::create(const QString& filename )
{
    boost::filesystem::path path( filename.toStdString() );
    path.replace_extension( L".adfs" );

    portfolio().create_with_fullpath( path.wstring() );

    std::unique_ptr< adcontrols::datafile > file( adcontrols::datafile::create( path.wstring() ) );
    if ( file ) {
        setFile( std::move( file ) );
        this->file()->accept( *this );
        setDisplayName( filename );
        return true;
    }
    return false;
}

bool
Dataprocessor::open(const std::filesystem::path& filename, std::string& emsg )
{
    emsg = std::string{};
    if ( adprocessor::dataprocessor::open( filename, emsg ) ) {
#if QTC_VERSION >= 0x08'00'00
        auto filePath = Utils::FilePath::fromString( QString::fromStdString( filename.string() ) );
        Core::IDocument::setFilePath( filePath );
#else
        Core::IDocument::setFilePath( QString::fromStdWString( filename ) );
        Core::DocumentManager::setCurrentFile( QString::fromStdWString( filename ) );
#endif
        return true;
    }
    return false;
}

bool
Dataprocessor::open(const QString &filename, QString& emsg )
{
    std::string msg;
    bool rcode = open( std::filesystem::path( filename.toStdString() ), msg );
    emsg = QString::fromStdString( msg );
    return rcode;
}

bool
Dataprocessor::load( const std::wstring& path, const std::wstring& id )
{
    ADDEBUG() << "## " << __FUNCTION__ << " path: " << path << " id: " << id;
    // this is used for reload 'acquire' when shanpshot spectrum was added.
    return this->file()->loadContents( path, id, *this );
}

portfolio::Portfolio
Dataprocessor::getPortfolio()
{
    return portfolio();
}

void
Dataprocessor::setCurrentSelection( portfolio::Folder& folder )
{
	impl_->idActiveFolium_ = folder.id();
}

void
Dataprocessor::setCurrentSelection( portfolio::Folium& folium )
{
    // ScopedDebug() << "## " << __FUNCTION__ << " ## " << folium.name();
	fetch( folium );
    impl_->idActiveFolium_ = folium.id();
    SessionManager::instance()->selectionChanged( this, folium );
}

portfolio::Folium
Dataprocessor::currentSelection() const
{
	return portfolio().findFolium( impl_->idActiveFolium_ );
}


namespace dataproc {

    // dispatch method
    struct doSpectralProcess : public boost::static_visitor<bool> {
        std::shared_ptr< const adcontrols::MassSpectrum > ptr_;
        Dataprocessor * dataprocessor_;

        portfolio::Folium& folium;

        doSpectralProcess( const adutils::MassSpectrumPtr& p
                           , portfolio::Folium& f
                           , Dataprocessor * dp ) : ptr_(p), dataprocessor_( dp ), folium(f) {
        }

        doSpectralProcess( std::shared_ptr< const adcontrols::MassSpectrum >& p
                           , portfolio::Folium& f
                           , Dataprocessor * dp ) : ptr_(p), dataprocessor_( dp ), folium(f) {
        }

        template<typename T> bool operator () ( T& ) const {
            ADTRACE() << "doSpectraolProcess( " << typeid( T ).name() << ") -- ignored";
            return false;
        }

        bool operator () ( const adcontrols::CentroidMethod& m ) const {
#ifndef NDEBUG
            if ( ptr_ && ptr_->isCentroid() )
                ADDEBUG() << "Apply centroid to histogram; converting to profile";
#endif
            return DataprocessorImpl::applyMethod( dataprocessor_, folium, m, *ptr_ );
        }

        bool operator () ( const adcontrols::TargetingMethod& m ) const {
            return DataprocessorImpl::applyMethod( dataprocessor_, folium, m );
        }

        bool operator () ( const adcontrols::IsotopeMethod& m ) const {
            return DataprocessorImpl::applyMethod( dataprocessor_, folium, m );
        }

        bool operator () ( const adcontrols::MSCalibrateMethod& m ) const {
            return DataprocessorImpl::applyMethod( dataprocessor_, folium, m );
        }
    };

    struct doChromatogramProcess : public boost::static_visitor<bool> {
        std::shared_ptr< adcontrols::Chromatogram > ptr_;
        portfolio::Folium& folium;
        Dataprocessor * dataprocessor_;
        adprocessor::noise_filter filter_;

        doChromatogramProcess( std::shared_ptr< adcontrols::Chromatogram > p
                               , portfolio::Folium& f
                               , Dataprocessor * dp ) : ptr_(p), folium(f), dataprocessor_( dp ) {
        }
        template<typename T> bool operator () ( T& ) const {
            return false;
        }

        bool operator () ( const adcontrols::PeakMethod& m ) const {
            using namespace adcontrols;
            auto [func,freq] = m.noise_filter();
            if ( func == chromatography::eDFTLowPassFilter ) {
                auto xptr = filter_( *ptr_, freq );
                folium.addAttachment( constants::F_DFT_CHROMATOGRAM ).assign( xptr, xptr->dataClass() );
                emit dataprocessor_->invalidateFolium( dataprocessor_, folium );
                return DataprocessorImpl::applyPeakMethod( dataprocessor_, folium, m, *xptr );
            } else {
                folium.erase_attachment( constants::F_DFT_CHROMATOGRAM,[](auto t) { ADDEBUG() << ">> erase_attachment: " << t; });
                emit dataprocessor_->invalidateFolium( dataprocessor_, folium );
                return DataprocessorImpl::applyPeakMethod( dataprocessor_, folium, m, *ptr_ );
            }
        }
    };

    // dispatch data type
    struct processIt : public boost::static_visitor<bool> {

        const adcontrols::ProcessMethod::value_type& m_;
        portfolio::Folium& folium_;
        Dataprocessor * dataprocessor_;

        processIt( const adcontrols::ProcessMethod::value_type& m, portfolio::Folium& f, Dataprocessor * dp )
            : m_(m)
            , folium_(f)
            , dataprocessor_( dp ) {
        }

        template<typename T> bool operator ()( T& ) const {
            return false;
        }

        bool operator () ( adutils::MassSpectrumPtr& ptr ) const {
            return boost::apply_visitor( doSpectralProcess(ptr, folium_, dataprocessor_ ), m_ );
        }

        bool operator () ( adutils::ChromatogramPtr& ptr ) const {
            return boost::apply_visitor( doChromatogramProcess(ptr, folium_, dataprocessor_ ), m_ );
        }
    };
    //-----
}

portfolio::Folium
Dataprocessor::findProfiledHistogram( const portfolio::Folium& folium )
{
    if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

        return portfolio::find_first_of( folium.attachments()
                                         , []( const portfolio::Folium& a ){
                                             return a.name() == Constants::F_PROFILED_HISTOGRAM;
                                         } );
    }
    return portfolio::Folium();
}

portfolio::Folium
Dataprocessor::addProfiledHistogram( portfolio::Folium& folium )
{
    if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

        auto att = findProfiledHistogram( folium );

        if ( !att && ptr->isCentroid() ) {
            if ( auto spectrometer = massSpectrometer() ) { // implemented in base class 'adprocessor::dataprocessor'
                att = folium.addAttachment( Constants::F_PROFILED_HISTOGRAM );
                auto ms = adcontrols::histogram::make_profile( *ptr, *spectrometer );
                att.assign( ms, ms->dataClass() );
                emit SessionManager::instance()->foliumChanged( this, folium );
                return att;
            }
        }
    }
    return portfolio::Folium();
}

void
Dataprocessor::applyProcess( const adcontrols::ProcessMethod& m, ProcessType procType )
{
    // ADDEBUG() << "################### " << __FUNCTION__ << " ##";
    portfolio::Folium folium = portfolio().findFolium( impl_->idActiveFolium_ );
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
            selector.append< adcontrols::TargetingMethod >( method ); // always do 'targeting' when centroid

        } else if ( procType == TargetingProcess ) {
            if ( auto fCentroid = portfolio::find_first_of( folium.attachments(), []( portfolio::Folium& f ) {
                        return f.name() == Constants::F_CENTROID_SPECTRUM; } ) ) {
                selector.append< adcontrols::TargetingMethod >( method );
            } else {
                selector.append< adcontrols::CentroidMethod >( method );
                selector.append< adcontrols::TargetingMethod >( method );
            }
        } else if ( procType == CalibrationProcess ) {
            // should not be here
        } else if ( procType == PeakFindProcess ) {
            selector.append< adcontrols::PeakMethod >( method );
        } else if ( procType == MSChromatogrProcess ) {
            // this is done in MainWindow::handleProcess
        }

        adutils::ProcessedData::value_type data
            = adutils::ProcessedData::toVariant( static_cast<boost::any&>( folium ) );

        for ( auto it = method.begin(); it != method.end(); ++it )
            boost::apply_visitor( processIt(*it, folium, this ), data );

        // post processing -- update annotation etc.
        if ( adportable::a_type< std::shared_ptr< adcontrols::MassSpectrum > >::is_a( folium.data() ) ) {
            if ( auto ms = boost::any_cast< std::shared_ptr< adcontrols::MassSpectrum > >( folium.data() ) ) {
                if ( ms->isHistogram() || !ms->isCentroid() ) {
                    auto atts = folium.attachments();
                    auto itCentroid = std::find_if( atts.begin(), atts.end(), []( auto& f ){ return f.name() == Constants::F_CENTROID_SPECTRUM; });
                    if ( itCentroid != atts.end() ) {
                        if ( adportable::a_type< std::shared_ptr< adcontrols::MassSpectrum > >::is_a( itCentroid->data() ) ) {
                            if ( auto processed = boost::any_cast< std::shared_ptr< adcontrols::MassSpectrum > >( itCentroid->data() ) ) {
                                // if has targeting...
                                size_t i(0);
                                for ( auto& xms: adcontrols::segment_wrapper<>( *processed ) ) {
                                    auto& tms = adcontrols::segment_wrapper<>( *ms )[ i ];
                                    for ( const auto& a: xms.get_annotations() ) {
                                        if ( ( a.index() >= 0 ) && a.index() < processed->size() ) {
                                            double mass = processed->mass( a.index() );
                                            adcontrols::annotation anno( a );
                                            // ADDEBUG() << a.text();
                                            anno.index( ms->getIndexFromMass( mass, true ) );
                                            anno.x( mass );
                                            anno.y( tms.intensity( anno.index() ) );
                                            tms.get_annotations() << anno;
                                        }
                                    }
                                    ++i;
                                }
                            }
                        }
                    }
                }
            }
        }

        // if folium == profile spectrum && process == centroid|targeting, then copy annotation to profile
        // ADDEBUG() << "################### " << __FUNCTION__ << " ##";
        setModified( true );

        SessionManager::instance()->processed( this, folium );
    }
}

void
Dataprocessor::remove( portfolio::Folium folium )
{
}

void
Dataprocessor::handleSetGlobalMSLock( portfolio::Folium folium )
{
    if ( folium.empty() )
        fetch( folium );

    if ( folium.parentFolder().name<char>() == "Spectra" ) {

        auto folder = portfolio().addFolder( L"MSLock" );

        auto dst = folder.addFolium( folium.name() ).assign( folium.data(), folium.dataClass().c_str() );
        dst.appendAttributes( folium.attributes() );
        for ( const auto& a: folium.attachments() ) {
            auto att = dst.addAttachment( a.name() ).assign( a.data(), a.dataClass().c_str() );
            att.appendAttributes( a.attributes(), false );
        }
        SessionManager::instance()->updateDataprocessor( this, dst );

    } else if ( folium.parentFolder().name<char>() == "MSLock" ) {
        folium.setAttribute( "mslock", "true" );
    }
    adprocessor::dataprocessor::handleGlobalMSLockChanged();
    setModified( true );
}

void
Dataprocessor::handleRemoveGlobalMSLock( portfolio::Folium folium )
{
    ADDEBUG() << "## " << __FUNCTION__ << " ## " << folium.name();
    if ( folium.parentFolder().name<char>() == "MSLock" ) {
        folium.setAttribute( "mslock", "false" );
    }
    adprocessor::dataprocessor::handleGlobalMSLockChanged();
    setModified( true );
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
    portfolio::Folder calibFolder = portfolio().addFolder( L"MSCalibration" );

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
    portfolio::Folium folium = portfolio().findFolium( impl_->idActiveFolium_ );
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
        name += descs[i].text<wchar_t>();

    portfolio::Folder folder = portfolio().addFolder( L"MSCalibration" );
    portfolio::Folium folium = folder.addFolium( name );

	SessionManager::instance()->updateDataprocessor( this, folium );

	if ( const adcontrols::MSCalibrateMethod * pCalibMethod = m.find< adcontrols::MSCalibrateMethod >() ) {
		std::pair<double, double> range = std::make_pair( pCalibMethod->lowMass(), pCalibMethod->highMass() );

		adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( src ) );  // profile, deep copy
		const adcontrols::MassSpectrum& tail = ms->numSegments() == 0 ? *ms : ms->getSegment( ms->numSegments() - 1 );
		double lMass = ms->mass( 0 );
		double hMass = tail.mass( tail.size() - 1 );
		// workaround for unsure acquired mass range (before calibration)
		range.first = std::min( range.first, lMass );
		range.second = std::max( range.second, hMass );
		ms->setAcquisitionMassRange( range.first, range.second );
        //
		folium.assign( ms, ms->dataClass() );

		for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
			boost::apply_visitor( doSpectralProcess( ms, folium, this ), *it );

		SessionManager::instance()->updateDataprocessor( this, folium );
		setModified( true );
	}
}

void
Dataprocessor::applyCalibration( const adcontrols::ProcessMethod& m
                                 , const adcontrols::MSAssignedMasses& assigned )
{
    portfolio::Folium folium = portfolio().findFolium( impl_->idActiveFolium_ );

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
    portfolio::Folder folder = portfolio().addFolder( L"MSCalibration" );
    std::wstring name;
    const adcontrols::descriptions& descs = centroid.getDescriptions();
    for ( size_t i = 0; i < descs.size(); ++i )
        name += descs[ i ].text<wchar_t>();
    name += L", manually assigned";

    portfolio::Folium folium = folder.addFolium( name );

    adutils::MassSpectrumPtr ms( new adcontrols::MassSpectrum( profile ) );  // deep copy
    folium.assign( ms, ms->dataClass() );

    // copy centroid spectrum, which manually assigned by user and result stored in 'assined'
    portfolio::Folium att = folium.addAttachment( L"Centroid Spectrum" );
    adcontrols::MassSpectrumPtr pCentroid( new adcontrols::MassSpectrum( centroid ) ); // deep copy
    att.assign( pCentroid, pCentroid->dataClass() );

    // todo: process method to be added

    if ( DataprocessorImpl::applyMethod( this, folium, calibMethod, assigned, massSpectrometer() ) ) {
        SessionManager::instance()->updateDataprocessor( this, folium );
    }

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
		if ( DataprocessorImpl::applyMethod( this, folium, *mcalib, assigned, this->massSpectrometer() ) )
			SessionManager::instance()->updateDataprocessor( this, folium );

        setModified( true );
    }
}

void
Dataprocessor::applyCalibration( const adcontrols::MSCalibrateResult& calibration )
{
    // ADDEBUG() << "applyCalibration: ";

    if ( portfolio::Folder folder = portfolio().findFolder( L"Spectra" ) ) {

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

    ADTRACE() << "applyCalibration " << calibration.calibration().calibrationUuid()
              << " for " << calibration.calibration().massSpectrometerClsid()
              << " to file: " << this->filename();

    if ( !adprocessor::dataprocessor::applyCalibration( calibration ) ) {
        ADDEBUG() << "applyCalibration faild";
    }
}

adcontrols::lockmass::mslock
Dataprocessor::doMSLock( portfolio::Folium& folium
                         , std::shared_ptr< adcontrols::MassSpectrum > ms
                         , const std::vector< std::pair< int, int > >& indecies ) // idx, fcn
{
    if ( auto sp = this->massSpectrometer() ) {
        sp->assignMasses( *ms, 0 ); // clear lock mass if already been applied
    }

    ADDEBUG() << "############## doMSLock ####################";

    adcontrols::lockmass::mslock mslock;
    std::for_each( indecies.begin(), indecies.end(), [&]( const auto& a ){
        adcontrols::lockmass::mslock::findReferences( mslock, *ms, a.first, a.second );
    });

    // apply profile and peakinfo contained in the folium
    if ( mslock.fit() ) {
        if ( mslock( *ms ) ) {
            ms->addDescription( adcontrols::description( L"process", L"mslock" ) );

            setModified( true );

            if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {
                mslock( *profile );
            }

            portfolio::Folio atts = folium.attachments();
            for ( auto& a: atts ) {
                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( a ) ) {
                    if ( ptr == ms ) {  // 'ptr' is mass locked source, don't apply lock mass twince
                        // update attached peakinfo
                        if ( auto fchild = portfolio::find_first_of( a.attachments()
                                                                     , []( const auto& child ){
                                                                         return portfolio::is_type< adcontrols::MSPeakInfoPtr >( child ); })) {
                            if ( auto pkinfo = portfolio::get< adcontrols::MSPeakInfoPtr >( fchild ) ) {
                                mslock( *pkinfo );
                                adcontrols::MSPeakInfo::setReferences( *pkinfo, indecies );
                            }
                        }
                    } else {
                        mslock( *ptr );
                    }
                }
            }
            auto a = folium.addAttachment( adcontrols::constants::F_MSLOCK );
            a.assign( std::make_shared< adcontrols::lockmass::mslock >( mslock ), adcontrols::lockmass::mslock::dataClass() );
        }
        return mslock;
    }
    return {};
}

void
Dataprocessor::lockMassHandled( const std::wstring& foliumId
                                , const adcontrols::MassSpectrumPtr& ms
                                , const adcontrols::lockmass::mslock& lockmass )
{
    using portfolio::Folium;

	if ( Folium folium = this->portfolio().findFolium( foliumId ) ) {

        if ( auto profile = portfolio::get< adcontrols::MassSpectrumPtr >( folium ) ) {

            lockmass( *profile );

            portfolio::Folio atts = folium.attachments();
            for ( auto& a: atts ) {
                if ( auto ptr = portfolio::get< adcontrols::MassSpectrumPtr >( a ) ) {

                    if ( ptr == ms ) {
                        // 'ptr' is mass locked source, don't apply lock mass twince

                        // update attached peakinfo
                        if ( auto fchild = portfolio::find_first_of( a.attachments(), []( Folium& child ){
                                    return portfolio::is_type< adcontrols::MSPeakInfoPtr >( child );
                                } ) )   {

                            if ( auto pkinfo = portfolio::get< adcontrols::MSPeakInfoPtr >( fchild ) )
                                DataprocHandler::reverse_copy( *pkinfo, *ptr );
                        }

                    } else {
                        // ADDEBUG() << "apply lockmass to: " << a.name();
                        lockmass( *ptr );
                    }
                }
            }

            setModified( true );
        }
    }
}

void
Dataprocessor::formulaChanged()
{
    setModified( true );
}

portfolio::Folium
Dataprocessor::addSpectrum( std::shared_ptr< adcontrols::MassSpectrum > ptr, const adcontrols::ProcessMethod& m )
{
    portfolio::Folder folder = portfolio().addFolder( L"Spectra" );

    // name from descriptions : exclude values which key has a pattern of "acquire.protocol.*" that is description for protocol/fcn related
    // std::wstring name = ptr->getDescriptions().make_folder_name( L"^((?!acquire\\.protocol\\.).)*$" );
    std::wstring name = ptr->getDescriptions().make_folder_name( L"(^folium.create$)|(^create$)" );

    bool mslocked( false );
    if ( auto lkms = dataGlobalMSLock() )
        mslocked = mslock( *ptr, *lkms );

    if ( mslocked )
        name += L",mslk";

    portfolio::Folium folium = folder.addFolium( name );
    folium.assign( ptr, ptr->dataClass() );

    if ( mslocked )
        folium.setAttribute( "mslock_external", "true" );

    for ( adcontrols::ProcessMethod::vector_type::const_iterator it = m.begin(); it != m.end(); ++it )
        boost::apply_visitor( doSpectralProcess( ptr, folium, this ), *it );

    SessionManager::instance()->updateDataprocessor( this, folium );

    setModified( true );

	return folium;
}

portfolio::Folium
Dataprocessor::addChromatogram( std::shared_ptr< adcontrols::Chromatogram > cptr
                                , const adcontrols::ProcessMethod& m
                                , std::shared_ptr< adprocessor::noise_filter > filter )
{
    portfolio::Folder folder = portfolio().addFolder( L"Chromatograms" );

    std::wstring name = adcontrols::Chromatogram::make_folder_name( cptr->getDescriptions() );

    portfolio::Folium folium = folder.addFolium( name );
	folium.assign( cptr, cptr->dataClass() );

    if ( auto peakm = m.find< adcontrols::PeakMethod >() ) {
        using namespace adcontrols;
        auto [func,freq] = peakm->noise_filter();
        if ( func == chromatography::eDFTLowPassFilter && filter ) {
            auto pptr = (*filter)( *cptr, freq ); // chromatogram to be processed
            folium.addAttachment( constants::F_DFT_CHROMATOGRAM ).assign( pptr, pptr->dataClass() );
            DataprocessorImpl::applyPeakMethod( this, folium, *peakm, *pptr );
        } else {
            folium.erase_attachment( constants::F_DFT_CHROMATOGRAM,[](auto t) { ADDEBUG() << ">>>>>> erase_attachment: " << t; });
            DataprocessorImpl::applyPeakMethod( this, folium, *peakm, *cptr );
        }
    }

    // copy peak result into chromatogram (for annotation)
    portfolio::Folio attachments = folium.attachments();
    auto it = std::find_if( attachments.begin(), attachments.end()
                            , []( const auto& a ){ return a.data().type()
                                    == typeid( std::shared_ptr< adcontrols::PeakResult > ); } );
    if ( it != attachments.end() ) {
        if ( auto pkres = boost::any_cast< std::shared_ptr< adcontrols::PeakResult > >( it->data() ) ) {
            cptr->setBaselines( pkres->baselines() );
            cptr->setPeaks( pkres->peaks() );
        }
    }

    setModified( true );

	return folium;
}

portfolio::Folium
Dataprocessor::addContour( std::shared_ptr< adcontrols::MassSpectra > spectra )
{
    portfolio::Folder folder = portfolio().addFolder( L"Contours" );

    const auto& desc = spectra->getDescriptions();
    std::wstring name =
        std::accumulate( desc.begin(), desc.end(), std::wstring { L"Contour " },
                         [] ( const std::wstring& a, const adcontrols::description& b ) { return a + b.text<wchar_t>(); } );

    portfolio::Folium folium = folder.addFolium( name );  // "Contours/Contour"
	folium.assign( spectra, spectra->dataClass() );

    SessionManager::instance()->updateDataprocessor( this, folium );
    setModified( true );

	return folium;
}

portfolio::Folium
Dataprocessor::addContourClusters( std::shared_ptr< adcontrols::SpectrogramClusters > clusters )
{
    portfolio::Folder folder = portfolio().addFolder( L"Contours" );
    portfolio::Folium folium = folder.findFoliumByName( L"Contour" );  // "Contours/Contour"
    if ( folium ) {
        portfolio::Folium att = folium.addAttachment( L"Clusters" );
        att.assign( clusters, clusters->dataClass() );

        SessionManager::instance()->updateDataprocessor( this, folium );
        setModified( true );
    }
	return folium;
}

void
Dataprocessor::findSinglePeak( portfolio::Folium folium, std::pair< double, double > trange )
{
    if ( auto chro = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {

        auto res = chro->find_single_peak( trange.first, trange.second );

        if ( res.first && res.second ) {
            if ( auto pkres = std::make_shared< adcontrols::PeakResult >() ) {
                *pkres << std::move( res );

                portfolio::Folium att = folium.addAttachment( Constants::F_PEAKRESULT ); // L"Peak Result" // unique by default
                att.assign( pkres, pkres->dataClass() );

                SessionManager::instance()->updateDataprocessor( this, folium );
                setModified( true );
            }
        }
    }
}

void
Dataprocessor::baselineCollection( portfolio::Folium folium )
{
    using dataTuple = std::tuple< std::shared_ptr< adcontrols::PeakResult >
                                  , std::shared_ptr< adcontrols::Chromatogram >
                                  , std::shared_ptr< adcontrols::MassSpectrum >
                                  >;
    if ( auto var = adutils::to_variant< dataTuple >()( static_cast< const boost::any& >( folium ) ) ) {

        double y0 = 0;
        if ( boost::apply_visitor( baseline_level_visitor(y0), *var ) ) {
            for ( auto& a: folium.attachments() ) {
                if ( auto var = adutils::to_variant< dataTuple >()(static_cast< boost::any& >( a )) ) {
                    boost::apply_visitor( baseline_level_visitor(y0), *var );
                }
            }
            SessionManager::instance()->updateDataprocessor( this, folium );
            setModified( true );
        }
    } else {
        ADDEBUG() << "####### no data found for " << __FUNCTION__ << " #########";
    }
}

void
Dataprocessor::dftFilter( portfolio::Folium folium
                          , std::shared_ptr< adcontrols::ProcessMethod > pm )
{
    ADDEBUG() << "## " << __FUNCTION__ << " ## ";
    double freq = 10.0;
    if ( auto peakm = pm->find< adcontrols::PeakMethod >() ) {
        std::tie(std::ignore, freq) = peakm->noise_filter();
    }
    using namespace adcontrols::constants;

    if ( auto chr = portfolio::get< adcontrols::ChromatogramPtr >( folium ) ) {

        auto xchr = adprocessor::noise_filter()( *chr, freq );
        folium.addAttachment( F_DFT_CHROMATOGRAM ).assign( xchr, xchr->dataClass() );

        emit SessionManager::instance()->foliumChanged( this, folium );
        SessionManager::instance()->updateDataprocessor( this, folium );
        setModified( true );
    }
}

void
Dataprocessor::setAttribute( portfolio::Folium folium, std::pair< std::string, std::string >&& keyValue)
{
    folium.setAttribute( keyValue.first, keyValue.second );
    if ( keyValue.first == "remove" && keyValue.second == "true" ) {
        folium.setAttribute( "isChecked", "false" );
    }
    setModified( true );
}

void
Dataprocessor::deleteRemovedItems()
{
    for ( auto folder: portfolio().folders() ) {
        for ( auto& folium: folder.folio() ) {
            if ( folium.attribute( L"remove" ) == L"true" ) {
                folder.erase( folium, []( const auto& t){ /* ADDEBUG() << "deleteRemovedItems: " << t; */} );
                setModified( true );
            }
        }
    }
}

void
Dataprocessor::createContour()
{
	DataprocessWorker::instance()->createContour( this );
    setModified( true );
}

void
Dataprocessor::clusterContour()
{
	DataprocessWorker::instance()->clusterContour( this );
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

            auto xms = std::make_shared< adcontrols::MassSpectrum >( *profile );
            // adcontrols::MassSpectrum xms( *profile );
            for ( size_t i = 0; i < xms->size(); ++i )
                xms->setIntensity( i, xms->intensity( i ) - background->intensity( i ) );

			xms->addDescription( adcontrols::description( L"processed", ( boost::wformat( L"%1% - %2%" ) % target.name() % base.name() ).str() ) );
            addSpectrum( xms, adcontrols::ProcessMethod() );
            setModified( true );
        }
    }
}

////////////////////////////////////
// dataSubscriber implementation

void
Dataprocessor::notify( adcontrols::dataSubscriber::idError, const std::string& json )
{
    QString msg( tr( "Instrument module(s) \"%1\" not installed." ).arg( QString::fromStdString( json ) ) );
    emit onNotify( msg );
}

//

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

	portfolio::Folder folder = portfolio().addFolder( foldername );
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

/////////////// targetting /////////////
bool
DataprocessorImpl::applyMethod( Dataprocessor *
                                , portfolio::Folium& folium
                                , const adcontrols::TargetingMethod& m )
{
    if ( auto fCentroid = portfolio::find_first_of(
             folium.attachments()
             , []( portfolio::Folium& f ) { return f.name() == Constants::F_CENTROID_SPECTRUM; }) ) {

        if ( adcontrols::MassSpectrumPtr centroid = portfolio::get< adcontrols::MassSpectrumPtr >( fCentroid ) ) {

            if ( auto targeting = std::make_shared< adcontrols::Targeting >(m) ) {

                // ADDEBUG() << "doSpectraolProcess -- Targeting";

                if ( (*targeting)(*centroid) ) {

                    fCentroid.erase_attachment( Constants::F_TARGETING, [](auto t){ ADDEBUG() << "erase attachment: " << t; } );
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
DataprocessorImpl::applyMethod( Dataprocessor *
                                , portfolio::Folium& folium, const adcontrols::IsotopeMethod& m )
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

bool
DataprocessorImpl::applyMethod( Dataprocessor * dp
                                , portfolio::Folium& folium
                                , const adcontrols::MSCalibrateMethod& m )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr pProfile = boost::any_cast< adcontrols::MassSpectrumPtr >( folium );

    auto spectrometer = dp->massSpectrometer();
    // auto spectrometer = adcontrols::MassSpectrometer::create( pProfile->getMSProperty().dataInterpreterClsid() );
    if ( !spectrometer ) {
        // adcontrols::TimeDigitalHistogram
        adfs::stmt sql ( *dp->db() );
        sql.prepare( "SELECT clsidSpectrometer FROM ScanLaw WHERE objuuid=?" );
        sql.bind( 1 ) = boost::uuids::uuid{{ 0 }};
        if ( sql.step() == adfs::sqlite_row ) {
            auto clsid = sql.get_column_value< boost::uuids::uuid >( 0 );
            if ( ( spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( clsid ) ) )
                spectrometer->initialSetup( *dp->db(), {{ 0 }} ); // load existing calibration if any
        }
    } else {
        ADDEBUG() << "MassSpectrometer: " << spectrometer->massSpectrometerClsid();
    }

    if ( !spectrometer ) {
        ADTRACE() << "## Error: No mass spectrometer has determined";
        return false;
    }

    ADDEBUG() << "### " << __FUNCTION__ << " TBD";

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
					if ( ms.intensity( i ) < y_threshold )
						ms.setColor( i, 16 ); // transparent
				}
			}

            if ( auto pCalibResult = std::make_shared< adcontrols::MSCalibrateResult >() ) {

                portfolio::Folium fCalibResult = folium.addAttachment( L"Calibrate Result" );

                if ( DataprocHandler::doMSCalibration( *pCalibResult, *pCentroid, m, spectrometer->massSpectrometerClsid() ) ) {

                    assert( pCalibResult->calibration().massSpectrometerClsid() != boost::uuids::uuid{{0}} );

                    pCalibResult->calibration().setMassSpectrometerClsid( spectrometer->massSpectrometerClsid() );
                    fCalibResult.assign( pCalibResult, pCalibResult->dataClass() );
                } else {
                    // set centroid result for user manual peak assign possible
                    pCalibResult->calibration().setMassSpectrometerClsid( spectrometer->massSpectrometerClsid() );
                    fCalibResult.assign( pCalibResult, pCalibResult->dataClass() );
                }

                adcontrols::ProcessMethodPtr method( std::make_shared< adcontrols::ProcessMethod >( m ) );
                fCalibResult.addAttachment( L"Process Method" ).assign( method, method->dataClass() );

                return true;
            }
        }
    }
    return false;
}

bool
DataprocessorImpl::applyMethod( Dataprocessor *
                                , portfolio::Folium& folium
                                , const adcontrols::MSCalibrateMethod& m
                                , const adcontrols::MSAssignedMasses& assigned
                                , std::shared_ptr< adcontrols::MassSpectrometer > massSpectrometer )
{
    using namespace portfolio;

    adcontrols::MassSpectrumPtr pProfile = boost::any_cast< adcontrols::MassSpectrumPtr >( folium );

    Folium::vector_type atts = folium.attachments();
    Folium::vector_type::iterator it = Folium::find< adcontrols::MassSpectrumPtr >( atts.begin(), atts.end() );

    if ( it != atts.end() ) {

        if ( auto pCentroid = boost::any_cast< adcontrols::MassSpectrumPtr >( static_cast<boost::any&>( *it ) ) ) {

            if ( auto pResult = std::make_shared< adcontrols::MSCalibrateResult >() ) {

                if ( DataprocHandler::doMSCalibration( *pResult, *pCentroid, m, assigned, massSpectrometer->massSpectrometerClsid() ) ) {

                    assert( pResult->calibration().massSpectrometerClsid() != boost::uuids::uuid{{0}} );

                    portfolio::Folium att = folium.addAttachment( L"Calibrate Result" );
                    att.assign( pResult, pResult->dataClass() );

                    // rewrite calibration := change m/z asssing on the spectrum
                    pCentroid->setCalibration( pResult->calibration(), true );

                    // update profile mass array
                    if ( pProfile )
                        pProfile->setCalibration( pResult->calibration(), true );

                    return true;
                }
            }
        }
    }
    return false;
}

bool
DataprocessorImpl::applyMethod( Dataprocessor * dataprocessor
                                , portfolio::Folium& folium
                                , const adcontrols::CentroidMethod& m
                                , const adcontrols::MassSpectrum& profile )
{
    portfolio::Folium att = folium.addAttachment( Constants::F_CENTROID_SPECTRUM );
    std::shared_ptr< adcontrols::MassSpectrum > pCentroid = std::make_shared< adcontrols::MassSpectrum >();
    bool centroid;

	// make sure no 'processed profile' data exist
	folium.erase_attachment( Constants::F_DFT_FILTERD,[](auto t) { ADDEBUG() << "erase_attachment: " << t; });
    folium.erase_attachment( Constants::F_MSPEAK_INFO,[](auto t) { ADDEBUG() << "erase_attachment: " << t; });

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

        centroid = adprocessor::dataprocessor::doCentroid( *pkInfo, *pCentroid, *profile2, m );

    } else {
        centroid = adprocessor::dataprocessor::doCentroid( *pkInfo, *pCentroid, profile, m );
    }

    if ( centroid ) {
        pCentroid->addDescription( adcontrols::description( L"process", L"Centroid" ) );
        att.assign( pCentroid, pCentroid->dataClass() );

        auto mptr = std::make_shared< adcontrols::ProcessMethod >( m ); // Ptr ptr( new adcontrols::ProcessMethod() );
        att.addAttachment( L"Process Method" ).assign( mptr, mptr->dataClass() );

        att.addAttachment( L"MSPeakInfo" ).assign( pkInfo, pkInfo->dataClass() );

        emit SessionManager::instance()->foliumChanged( dataprocessor, folium );

        return true;
    } else {
        pCentroid->addDescription( adcontrols::description( L"process", L"Centroid failed" ) );
        att.assign( pCentroid, pCentroid->dataClass() ); // attach it even no peak detected
    }
    return false;
}

// FindPeak -- chromatographic peak find
// static
bool
DataprocessorImpl::applyPeakMethod( Dataprocessor *
                                    , portfolio::Folium& folium
                                    , const adcontrols::PeakMethod& m
                                    , const adcontrols::Chromatogram& c )
{
    if ( auto pResult = std::make_shared< adcontrols::PeakResult >() ) {
        if ( DataprocHandler::doFindPeaks( *pResult, c, m ) ) {
            auto mptr = std::make_shared< adcontrols::ProcessMethod >( m );
            auto att = folium.addAttachment( adcontrols::constants::F_PEAKRESULT ).assign( pResult, pResult->dataClass() );
            att.addAttachment( L"Process Method" ).assign( mptr, mptr->dataClass() );
            return true;
        }
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
Dataprocessor::MSCalibrationSave( portfolio::Folium& folium, const QString& file )
{
    if ( file.isEmpty() )
        return false;

    boost::filesystem::path fname( file.toStdString() );
    fname.replace_extension( ".msclb" );

    adfs::filesystem dbf;
    if ( !adutils::fsio::create( dbf, fname.wstring() ) )
        return false;

    portfolio::Folio atts = folium.attachments();
    auto it = portfolio::Folium::find< adcontrols::MSCalibrateResultPtr >( atts.begin(), atts.end() );
    if ( it != atts.end() ) {
        const adcontrols::MSCalibrateResultPtr ptr = boost::any_cast< adcontrols::MSCalibrateResultPtr >( it->data() );
        adutils::fsio::save_mscalibfile( dbf, *ptr );
        //
        fname.replace_extension( ".msclb.xml" );
        std::wofstream of( fname.string() );
        adportable::xml::serialize<>()( *ptr, of );
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
Dataprocessor::MSCalibrationLoad( const QString& filename
                                  , adcontrols::MSCalibrateResult& r, adcontrols::MassSpectrum& ms )
{
    boost::filesystem::path path( filename.toStdString() );
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
    boost::filesystem::path path( this->file()->filename() );
    path += ".xml";
    portfolio().save( path.wstring() );
}

void
Dataprocessor::applyLockMass( std::shared_ptr< adcontrols::MassSpectra > spectra )
{
    if ( spectra->size() == 0 )
        return;

    if ( auto rawfile = rawdata() ) {

        if ( auto msfractuation = rawfile->msFractuation() ) {

            bool interporate( false );

            if ( !msfractuation->has_a( (*spectra->begin())->rowid() ) ) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                int result = QMessageBox::question( MainWindow::instance()
                                                    , QObject::tr("Lock mass")
                                                    , QObject::tr( "Blacketing ?" )
                                                    , QMessageBox::Yes
                                                    , QMessageBox::No|QMessageBox::Default|QMessageBox::Escape );
#else
                QMessageBox mbox;
                mbox.setText( "Lock mass" );
                mbox.setInformativeText( "Blacketing?" );
                mbox.setStandardButtons( QMessageBox::Yes | QMessageBox::No ); // |QMessageBox::Default|QMessageBox::Escape );
                mbox.setDefaultButton( QMessageBox::No ); //|QMessageBox::Default|QMessageBox::Escape );
                int result = mbox.exec();
#endif
                if ( result == QMessageBox::Yes )
                    interporate = true;
            }

            for ( auto& ms : *spectra ) {
                auto fitter = msfractuation->find( ms->rowid(), interporate );
                fitter( *ms );
            }
        } else {
            QMessageBox::information( MainWindow::instance()
                                      , QObject::tr( "QtPlatz" )
                                      , QObject::tr( "No MS-Fractuation instance exists in this data file" ) );
        }
    }
}

void
Dataprocessor::exportMatchedMasses( std::shared_ptr< adcontrols::MassSpectra > spectra
                                    , const std::wstring& foliumId )
{
    DataprocessWorker::instance()->exportMatchedMasses( this, spectra, foliumId );
}

void
Dataprocessor::xicSelectedMassPeaks( adcontrols::MSPeakInfo&& info )
{
    auto ms = std::make_shared< adcontrols::MassSpectrum >();
    ms->resize( info.size() );
    ms->setCentroid( adcontrols::CentroidPeakAreaWaitedMass );
    bool is_area = true;

    size_t idx = 0;
    for ( const auto& pk: info ) {
        ms->setIntensity( idx, is_area ? pk.area() : pk.height() );
        ms->setMass( idx, pk.mass() );
        ms->setTime( idx, pk.time() );
        idx++;
    }
    ms->setAcquisitionMassRange( 0, 1000 );
    auto prop = ms->getMSProperty();
    prop.setInstMassRange( std::make_pair( 0, 1000 ) );
    ms->setMSProperty( prop );

    auto folder = portfolio().addFolder( L"Spectra" );
    std::wstring name = L"XIC";

    auto folium = folder.addFolium( name );
    folium.assign( ms, ms->dataClass() );
    SessionManager::instance()->updateDataprocessor( this, folium );
}

void
Dataprocessor::markupMassesFromChromatograms( portfolio::Folium&& folium )
{
    if ( auto ms = portfolio::get< std::shared_ptr< adcontrols::MassSpectrum > >( folium ) ) {

        adcontrols::MSPeakInfo info;
        if ( auto folder = portfolio().findFolder( L"Chromatograms" ) ) {
            for ( auto& f: folder.folio() ) {
                // ADDEBUG() << "Chromatogram: " << f.attribute( L"isChecked" ) << ", " << f.name();
                if ( f.attribute( L"isChecked" ) == L"true" ) {
                    fetch( f );
                    if ( auto chro = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( f ) ) {
                        auto jv = adportable::json_helper::find( chro->generatorProperty(), "generator.extract_by_peak_info.pkinfo" );
                        if ( jv.is_object() ) {
                            if ( auto pk = adcontrols::MSPeakInfoItem::fromJson( jv ) ) {
                                info << *pk;
                            }
                        }
                    }
                }
            }
        }
        if ( info.size() ) {
            if ( auto att = portfolio::find_first_of( folium.attachments()
                                                      , []( const portfolio::Folium& a ){
                                                          return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
                if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( att ) ) {
                    for ( const auto& pk: info ) {
                        size_t idx(0);
                        if ( ( idx = centroid->find( pk.mass(), pk.widthHH() ) ) != adcontrols::MassSpectrum::npos ) {
                            centroid->setColor( idx, 15 ); // magenta
                        } else {
                            ADDEBUG() << "missed idx: " << idx << ", mass: " << pk.mass() << ", " << centroid->mass( idx );
                        }
                    }
                }
            }
        }
        setModified( true );
        SessionManager::instance()->updateDataprocessor( this, folium );
    }
}

void
Dataprocessor::clearMarkup( portfolio::Folium&& folium )
{
    if ( portfolio::is_type< adutils::MassSpectrumPtr >( folium ) ) {
        if ( auto att = portfolio::find_first_of( folium.attachments()
                                                  , []( const portfolio::Folium& a ){
                                                      return a.name() == Constants::F_CENTROID_SPECTRUM; }) ) {
            if ( auto centroid = portfolio::get< adcontrols::MassSpectrumPtr >( att ) ) {
                centroid->setColorArray( {} ); // clear
            }
        }
        setModified( true );
        SessionManager::instance()->updateDataprocessor( this, folium );
    }
}


namespace dataproc
{
#if QTC_VERSION <= 0x03'02'81
    QString
    Dataprocessor::filepath() const
    {
        return this->filePath();
    }
#endif
}

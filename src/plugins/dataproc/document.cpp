/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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

#include "document.hpp"
#include "dataprocessor.hpp"
#include "sessionmanager.hpp"
#include "mainwindow.hpp"
#include "dataproceditor.hpp"
#include <adcontrols/axis.hpp>
#include <adcontrols/chromatogram.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/description.hpp>
#include <adcontrols/descriptions.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/msproperty.hpp>
#include <adcontrols/msqpeaks.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/tofprotocol.hpp>
#include <adfs/adfs.hpp>
#include <adfs/filesystem.hpp>
#include <adfs/file.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/scoped_debug.hpp>
#include <adportable/float.hpp>
#include <adportable/profile.hpp>
#include <adportable/utf.hpp>
#include <adportfolio/portfolio.hpp>
#include <adportfolio/folder.hpp>
#include <adportfolio/folium.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/settings.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <pugixml.hpp>
#include <app/app_version.h>
#include <coreplugin/icore.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/editormanager/editormanager.h>
#include <QFileInfo>
#include <QSettings>
#include <QMessageBox>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/all.hpp>
#include <boost/archive/archive_exception.hpp>
#include <atomic>
#include <cctype>
#include <thread>

namespace dataproc {

    struct user_preference {
        static std::filesystem::path path( QSettings * settings ) {
            std::filesystem::path dir( settings->fileName().toStdWString() );
            return dir.remove_filename() / "dataproc";
        }
    };

}

using namespace dataproc;

std::atomic<document * > document::instance_(0);
std::mutex document::mutex_;

document::document(QObject *parent)
    : QObject(parent)
    , quant_( std::make_shared< adcontrols::MSQPeaks >() )
    , settings_( std::make_shared< QSettings >( QSettings::IniFormat, QSettings::UserScope
                                                , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                , QLatin1String( "dataproc" ) ) )
    , horAxis_{
            { PlotChromatogram, adcontrols::axis::Seconds }
            ,{ PlotSpectrum, adcontrols::axis::MassToCharge }}
{
}

document *
document::instance()
{
    static document __instance;
    return &__instance;
}

std::shared_ptr< adcontrols::ProcessMethod >
document::processMethod() const
{
    std::lock_guard< std::mutex > lock( mutex_ );
    return pm_;
}

void
document::setProcessMethod( const adcontrols::ProcessMethod& m, const QString& filename )
{
    do {
        std::lock_guard< std::mutex > lock( mutex_ );
        pm_ = std::make_shared< adcontrols::ProcessMethod >( m );
    } while(0);

    if ( ! filename.isEmpty() ) {
        procmethod_filename_ = filename;
        qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, filename );
    }

    emit onProcessMethodChanged( filename );
}

void
document::addToRecentFiles( const QString& filename )
{
    qtwrapper::settings(*settings_).addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, filename );
}

void
document::addToRecentFiles( const QString& filename, const char * const GRP )
{
    qtwrapper::settings(*settings_).addRecentFiles( GRP, Constants::KEY_FILES, filename );
}


void
document::initialSetup()
{
    std::filesystem::path dir = user_preference::path( settings_.get() );

    if ( !std::filesystem::exists( dir ) ) {
        if ( !std::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
        }
    }

    QString path = recentFile( Constants::GRP_DATA_FILES, false );
    if ( path.isEmpty() ) {
        path = QString::fromStdWString( ( std::filesystem::path( adportable::profile::user_data_dir< char >() ) / "data" ).generic_wstring() );
    } else {
        path = QFileInfo( path ).path();
    }
    // fake project directory for help initial openfiledialog location
    Core::DocumentManager::setProjectsDirectory( Utils::FilePath::fromString( path ) );
    Core::DocumentManager::setUseProjectsDirectory( true );

    std::filesystem::path mfile( dir / "default.pmth" );
    adcontrols::ProcessMethod pm;
    if ( load( QString::fromStdWString( mfile.wstring() ), pm ) )
        setProcessMethod( pm, QString() ); // don't save default name
}

void
document::finalClose()
{
    std::filesystem::path dir = user_preference::path( settings_.get() );
    if ( !std::filesystem::exists( dir ) ) {
        if ( !std::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "dataproc::document"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return;
        }
    }

    // update data from UI
    adcontrols::ProcessMethod pm;
    MainWindow::instance()->getProcessMethod( pm );

    std::filesystem::path fname( dir / "default.pmth" );
    save( QString::fromStdWString( fname.wstring() ), pm );
}

adcontrols::MSQPeaks *
document::msQuanTable()
{
    return quant_.get();
}

const adcontrols::MSQPeaks *
document::msQuanTable() const
{
    return quant_.get();
}

void
document::setMSQuanTable( const adcontrols::MSQPeaks& v )
{
    quant_ = std::make_shared< adcontrols::MSQPeaks >( v );
}

QString
document::recentFile( const char * group, bool dir_on_fail )
{
    if ( group == 0 )
        group = Constants::GRP_DATA_FILES;

    QString file = qtwrapper::settings( *settings_ ).recentFile( group, Constants::KEY_FILES );
    if ( !file.isEmpty() )
        return file;

    if ( dir_on_fail ) {
#if QTC_VERSION <= 0x03'02'81
        file = Core::DocumentManager::currentFile();
        if ( file.isEmpty() )
            file = qtwrapper::settings( *settings_ ).recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
#endif
        if ( !file.isEmpty() ) {
            QFileInfo fi( file );
            return fi.path();
        }
        return QString::fromStdWString( adportable::profile::user_data_dir< wchar_t >() );
    }
    return QString();
}

// static
size_t
document::findCheckedTICs( Dataprocessor * dp, std::set< int >& vfcn )
{
    vfcn.clear();
    if ( dp ) {
        auto cfolder = dp->portfolio().findFolder( L"Chromatograms" );
        for ( auto& folium: cfolder.folio() ) {
            if ( folium.attribute( L"isChecked" ) == L"true" ) {
                const std::wstring& name = folium.name();
                auto found = name.find( std::wstring( L"TIC/TIC.") );
                if ( found != std::wstring::npos ) {
                    auto dot = name.find_last_of( L'.' );
                    if ( dot != std::wstring::npos ) {
                        int fcn = std::stoi( name.substr( dot + 1 ) );
                        vfcn.insert( fcn - 1 );
                    }
                }
            }
        }
    }
    return vfcn.size();
}

//static
const std::shared_ptr< adcontrols::Chromatogram >
document::findTIC( Dataprocessor * dp, int fcn )
{
    if ( dp ) {
        auto cfolder = dp->portfolio().findFolder( L"Chromatograms" );
        std::wstring name = ( boost::wformat( L"TIC/TIC.%d" ) % ( fcn + 1 ) ).str();
        if ( auto folium = cfolder.findFoliumByName( name ) ) {
            auto cptr = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium );
            return cptr;
        }

        // query should match both 'pkd.1.u5303a/TIC.1' 'TIC.1/pkd.1.u5303a
        std::ostringstream query;
        query << boost::format( "./folium[contains(@name,'TIC.%d')]" ) % ( fcn + 1 );
        if ( auto folium = cfolder.findFoliumByRegex( query.str() ) ) {
            auto cptr = portfolio::get< std::shared_ptr< adcontrols::Chromatogram > >( folium );
            return cptr;
        }

    }
    ADDEBUG() << "## " << __FUNCTION__ << " ## " << "TIC for protocol " << fcn + 1 << " cannot be found";
    return 0;
}

bool
document::load( const QString& filename, adcontrols::ProcessMethod& pm )
{
    QFileInfo fi( filename );

    if ( fi.exists() ) {
        adfs::filesystem fs;
        if ( fs.mount( filename.toStdWString().c_str() ) ) {
            adfs::folder folder = fs.findFolder( L"/ProcessMethod" );

            try {
                auto files = folder.files();
                if ( !files.empty() ) {
                    auto file = files.back();
                    try {
                        file.fetch( pm );
                    } catch ( std::exception& ex ) {
                        QMessageBox::information( 0, "dataproc -- Open default process method"
                                                  , ( boost::format( "Failed to open last used process method file: %1% by reason of %2% @ %3% L%4%" )
                                                      % filename.toStdString() % ex.what() % __FILE__ % __LINE__ ).str().c_str() );
                        return false;
                    }
                    return true;
                }
            } catch ( std::exception& ex ) { //boost::archive::archive_exception& ex ) {
                QMessageBox::information( 0, "dataproc -- Open default process method"
                                          , ( boost::format( "Failed to open last used process method file: %1% by reason of %2% @ %3% L%4%" )
                                              % filename.toStdString() % ex.what() % __FILE__ % __LINE__ ).str().c_str() );
            }
        }
    }
    return false;
}

bool
document::save( const QString& filename, const adcontrols::ProcessMethod& pm )
{
    std::filesystem::path name( filename.toStdWString() );
    name.replace_extension( ".pmth" );

    adfs::filesystem fs;

    if ( !fs.create( name.wstring().c_str() ) ) {
        ADDEBUG() << "Error: \"" << filename.toStdString() << "\" can't be created";
        return false;
    }

    adfs::folder folder = fs.addFolder( L"/ProcessMethod" );
    adfs::file adfile = folder.addFile( name.wstring(), name.wstring() );
    try {
        adfile.dataClass( adcontrols::ProcessMethod::dataClass() );
        adfile.save( pm );
    } catch ( std::exception& ex ) {
        ADDEBUG() << "Exception: " << boost::diagnostic_information( ex );
        return false;
    }
    adfile.commit();

    name.replace_extension( ".pmth.xml" );
    if ( std::filesystem::exists( name ) )
        std::filesystem::remove( name );

    try {
        std::wofstream of( name.string() );
        adcontrols::ProcessMethod::xml_archive( of, pm );
    } catch ( std::exception& ex ) {
        ADDEBUG() << boost::diagnostic_information( ex );
    }

    return true;
}

void
document::saveScanLaw( const QString& model, double flength, double accv, double tdelay, double mass, const QString& formula )
{
    if ( settings_ ) {
        settings_->beginGroup( "ScanLaws" );
        settings_->setValue( model + "/fLength", flength );
        settings_->setValue( model + "/acceleratorVoltage", accv );
        settings_->setValue( model + "/tDelay", tdelay );
        settings_->setValue( model + "/mass", mass );
        settings_->setValue( model + "/formula", formula );
        settings_->endGroup();
    }
    emit scanLawChanged( flength, accv, tdelay );
}

bool
document::findScanLaw( const QString& model, double& flength, double& accv, double& tdelay, double& mass, QString& formula )
{
    bool result( false );

    if ( settings_ ) {
        settings_->beginGroup( "ScanLaws" );
        QVariant vLength = settings_->value( model + "/fLength", flength );
        if ( vLength.isValid() ) {
            QVariant vAcc = settings_->value( model + "/acceleratorVoltage", accv );
            if ( vAcc.isValid() ) {
                QVariant vDelay = settings_->value( model + "/tDelay" );
                flength = vLength.toDouble();
                accv = vAcc.toDouble();
                tdelay = vDelay.toDouble();
                mass = settings_->value( model + "/mass" ).toDouble();
                formula = settings_->value( model + "/formula" ).toString();
                result = true;
            }
        }
        settings_->endGroup();
    }
    return result;
}

void
document::handleSelectTimeRangeOnChromatogram( double x1, double x2 )
{
	qtwrapper::waitCursor w;

	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
	if ( dp ) {
		if ( const adcontrols::LCMSDataset * dset = dp->rawdata() ) {
            {
                auto cptr = document::findTIC( dp, 0 );
            }

            if ( dset->dataformat_version() >= 3 ) {

                handleSelectTimeRangeOnChromatogram_v3( dp, dset, x1, x2 );

            } else {

                handleSelectTimeRangeOnChromatogram_v2( dp, dset, x1, x2 );

            }
        }
    }
}

void
document::handleSelectTimeRangeOnChromatogram_v2( Dataprocessor * dp, const adcontrols::LCMSDataset * dset, double x1, double x2 )
{
    // v2 data not supported
    QMessageBox::warning( 0, "DataprocPlugin", boost::current_exception_diagnostic_information().c_str() );
}

void
document::handleSelectTimeRangeOnChromatogram_v3( Dataprocessor * dp, const adcontrols::LCMSDataset * dset, double x1, double x2 )
{
    using adcontrols::DataReader;

    double t1 = (horAxis( PlotChromatogram ) == adcontrols::axis::Seconds) ? x1 : double( adcontrols::Chromatogram::toSeconds( x1 ) );
    double t2 = (horAxis( PlotChromatogram ) == adcontrols::axis::Seconds) ? x2 : double( adcontrols::Chromatogram::toSeconds( x2 ) );

    for ( auto reader: dset->dataReaders() ) {
        auto it1 = reader->findPos( t1 );
        auto it2 = reader->findPos( t2 );
        auto range = std::make_pair( it1->time_since_inject(), it2->time_since_inject() );
        ADDEBUG() << "------ time range: " << range;
        if ( auto ms = reader->coaddSpectrum( std::move(it1), std::move( it2 ) ) ) {
            std::ostringstream text;
            text << reader->abbreviated_display_name() << boost::format( " %.1f-%.1fs" ) % range.first % range.second;
            adcontrols::ProcessMethod m;
            ms->addDescription( adcontrols::description({"folium.create", text.str()}) );
            dp->mslock( *ms, t1 );
            portfolio::Folium folium = dp->addSpectrum( ms, m );
        }
    }
}


void
document::onSelectSpectrum_v3( Dataprocessor * dp, double /*time*/, adcontrols::DataReader_iterator iterator )
{
    using adcontrols::DataReader;

    // read from v3 format data
    if ( auto reader = iterator.dataReader() ) {

        if ( auto ms = reader->readSpectrum( iterator ) ) {
            std::ostringstream text;
            if ( iterator._fcn() < 0 ) {
                text << reader->abbreviated_display_name() << boost::format ( " %.3fs" ) % iterator->time_since_inject();
            } else {
                text << reader->abbreviated_display_name()
                     << boost::format ( " %.3fs p%d.%d " ) % iterator->time_since_inject() % ms->protocolId() % ms->nProtocols() ;
            }
            adcontrols::ProcessMethod m;
            ms->addDescription( adcontrols::description( {"folium.create", text.str() } ) );
	        if ( Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor() ) {
                dp->mslock( *ms, 0 );
                portfolio::Folium folium = dp->addSpectrum( ms, m );
            }
        } else {
            ADDEBUG() << "### " << __FUNCTION__ << " readSpectrum return nullptr ###";
        }

    } else {
        ADDEBUG() << "### " << __FUNCTION__ << " null dataReader returned from iterator ###";
    }
}

void
document::onSelectSpectrum_v2( double /*minutes*/, size_t pos, int fcn )
{
    ScopedDebug(__t);
	qtwrapper::waitCursor w;

	Dataprocessor * dp = SessionManager::instance()->getActiveDataprocessor();
	if ( dp ) {
		if ( const adcontrols::LCMSDataset * dset = dp->rawdata() ) {
			auto ms = std::shared_ptr< adcontrols::MassSpectrum >();
            try {
                std::wostringstream text;
                // size_t pos = dset->find_scan( index, fcn );
                if ( dset->getSpectrum( fcn, pos, *ms ) ) {
                    double t = dset->timeFromPos( pos );
                    if ( !adportable::compare<double>::approximatelyEqual( ms->getMSProperty().timeSinceInjection(), 0.0 ) )
                        t = ms->getMSProperty().timeSinceInjection();
                    text << boost::wformat( L"Spectrum %d fcn:%d/%d @ %.3lfs" ) % pos % ms->protocolId() % ms->nProtocols() % t;
                    adcontrols::ProcessMethod m;
                    ms->addDescription( adcontrols::description( L"create", text.str() ) );
                    dp->mslock( *ms, 0 );
                    portfolio::Folium folium = dp->addSpectrum( ms, m );
                }
            }
            catch ( ... ) {
                ADTRACE() << boost::current_exception_diagnostic_information();
                QMessageBox::warning( 0, "DataprocPlugin", boost::current_exception_diagnostic_information().c_str() );
            }
        }
    }
}

void
document::handle_folium_added( const QString& fname, const QString& path, const QString& id )
{
    qtwrapper::waitCursorBlocker block;

	std::wstring filename = fname.toStdWString();

    SessionManager::vector_type::iterator it = SessionManager::instance()->find( filename );
    if ( it == SessionManager::instance()->end() ) {
        it = SessionManager::instance()->find( filename );
    }

    if ( it != SessionManager::instance()->end() ) {
		if ( auto processor = it->processor() )
            processor->load( path.toStdWString(), id.toStdWString() );
    }
}

void
document::handle_portfolio_created( const QString& filename )
{
    Core::EditorManager::openEditor( Utils::FilePath::fromString( filename ) );
}

adcontrols::axis::AxisH
document::horAxis( Plot id ) const
{
    auto it = horAxis_.find( id );
    if ( it != horAxis_.end() )
        return it->second;

    return adcontrols::axis::Seconds;
}

void
document::setHorAxis( Plot id, adcontrols::axis::AxisH value )
{
    horAxis_[ id ] = value;
}

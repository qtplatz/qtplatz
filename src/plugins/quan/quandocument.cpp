/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "quandocument.hpp"
#include "quanconnection.hpp"
#include "quanconstants.hpp"
#include "paneldata.hpp"
#include "quandatawriter.hpp"
#include "quansampleprocessor.hpp"
#include "quanprocessor.hpp"
#include "quanprogress.hpp"
#include "quanpublisher.hpp"
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quancalibration.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#include <adfs/filesystem.hpp>
#include <adlog/logger.hpp>
#include <adportable/profile.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <app/app_version.h>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QFuture>

#include <algorithm>

namespace quan {
    namespace detail {
        struct user_preference {
            static boost::filesystem::path path( QSettings * settings ) {
                boost::filesystem::path dir( settings->fileName().toStdWString() );
                return dir.remove_filename() / "Quan";                
            }
        };
    }
}

using namespace quan;

std::atomic< QuanDocument * > QuanDocument::instance_( 0 );
std::mutex QuanDocument::mutex_;

QuanDocument::~QuanDocument()
{
}

QuanDocument::QuanDocument() : settings_( new QSettings(QSettings::IniFormat, QSettings::UserScope
                                                        , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                        , QLatin1String( "Quan" ) ) )
                             , quanSequence_( std::make_shared< adcontrols::QuanSequence >() )
                             , pm_(std::make_shared< adcontrols::ProcessMethod >())
                             , postCount_( 0 )
{
    (*pm_) << adcontrols::QuanMethod() << adcontrols::QuanCompounds();
    std::fill( dirty_flags_.begin(), dirty_flags_.end(), true );
    connect( this, &QuanDocument::onProcessed, this, &QuanDocument::handle_processed );
}

QuanDocument *
QuanDocument::instance()
{
    QuanDocument * tmp = instance_.load( std::memory_order_relaxed );
    std::atomic_thread_fence( std::memory_order_acquire );
    if ( tmp == nullptr ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        tmp = instance_.load( std::memory_order_relaxed );
        if ( tmp == nullptr ) {
            tmp = new QuanDocument();
            std::atomic_thread_fence( std::memory_order_release );
            instance_.store( tmp, std::memory_order_relaxed );
        }
    }
    return tmp;
}

void
QuanDocument::register_dataChanged( std::function< void( int, bool ) > f )
{
    clients_.push_back( f );
}

PanelData *
QuanDocument::addPanel( int idx, int subIdx, std::shared_ptr< PanelData >& section )
{
    auto& a_chapter = book_[ idx ];
    auto& a_page = a_chapter[ subIdx ];
    a_page.push_back( section );
    return findPanel( idx, subIdx, int( a_page.size() - 1 ) );
}

PanelData *
QuanDocument::findPanel( int idx, int subIdx, int pos )
{
    auto chapter = book_.find( idx );
    if ( chapter != book_.end() ) {
        auto page = chapter->second.find( subIdx );
        if ( page != chapter->second.end() && page->second.size() > size_t( pos ) )
            return page->second[ pos ].get();
    }
    return 0;
}

bool
QuanDocument::save_default_methods()
{
    boost::filesystem::path dir = detail::user_preference::path( settings_.get() );

    if ( !boost::filesystem::exists( dir ) ) {
        if ( !boost::filesystem::create_directories( dir ) ) {
            QMessageBox::information( 0, "QuanDocument"
                                      , QString( "Work directory '%1' can not be created" ).arg( dir.string().c_str() ) );
            return false;
        }
    }

    return save( dir / "QuanSequence.xml", *quanSequence_, false ) && save( dir / "QuanMethod.xml", *pm_ );
}

bool
QuanDocument::load_default_methods()
{
    QString name = recentFile( Constants::GRP_METHOD_FILES, Constants::KEY_FILES );
    boost::filesystem::path filepath( name.toStdWString() );
    
    if ( boost::filesystem::exists( filepath ) && load( filepath, *pm_ ) ) {
        auto qm = pm_->find< adcontrols::QuanMethod >();
        if ( !qm ) {
            *pm_ << adcontrols::QuanMethod();
            qm = pm_->find< adcontrols::QuanMethod >();
        }
        qm->quanMethodFilename( filepath.generic_wstring().c_str() ); // update filename with actual path
        dirty_flags_[ idMethodComplex ] = false;
    }

    // recovery from backup
    if ( dirty_flags_[ idMethodComplex ] ) {
        boost::filesystem::path dir = detail::user_preference::path( settings_.get() );
        boost::filesystem::path backup = dir / L"QuanMethod.xml";
        if ( boost::filesystem::exists( backup ) && load( backup, *pm_ ) )
            dirty_flags_[ idMethodComplex ] = false; // don't update filename
    }

    name = recentFile( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES );
    filepath = name.toStdWString();

    if ( boost::filesystem::exists( filepath ) && load( filepath, *quanSequence_ ) ) {
        dirty_flags_[ idQuanSequence ] = true;
        quanSequence_->filename( filepath.generic_wstring().c_str() ); // just make sure it has correct filename
    }

    // recovery from backup
    if ( dirty_flags_[ idQuanSequence ] ) {
        boost::filesystem::path dir = detail::user_preference::path( settings_.get() );
        boost::filesystem::path backup = dir / L"QuanSequence.xml";
        if ( boost::filesystem::exists( backup ) && load( backup, *quanSequence_ ) ) {
            dirty_flags_[ idQuanSequence ] = false;
            // stay with original filename
        }
    }
    return !(dirty_flags_[ idQuanSequence ] | dirty_flags_[ idMethodComplex ]);
}

void
QuanDocument::docTemplate( std::shared_ptr< adpublisher::document >& doc )
{
    docTemplate_ = doc;
}

std::shared_ptr< adpublisher::document >
QuanDocument::docTemplate() const
{
    return docTemplate_;
}

const adcontrols::QuanMethod&
QuanDocument::quanMethod() const
{
    if ( auto qm = pm_->find< adcontrols::QuanMethod >() )
        return *qm;
    else {
        *pm_ << adcontrols::QuanMethod();
        if ( qm = pm_->find< adcontrols::QuanMethod >() )
            return *qm;
    }
    BOOST_THROW_EXCEPTION( std::runtime_error( "adcontrols::ProcessMethod has a bug with respect to adcontrols::QuanMethod" ) );
}

void
QuanDocument::quanMethod( const adcontrols::QuanMethod& t )
{
    *pm_ *= t;
    dirty_flags_[ idQuanMethod ] = true;
    for ( auto& client: clients_ )
        client( idQuanMethod, false );
}

const adcontrols::QuanCompounds&
QuanDocument::quanCompounds() const
{
    auto qc = pm_->find< adcontrols::QuanCompounds >();
    if ( !qc ) {
        *pm_ << adcontrols::QuanCompounds();
        qc = pm_->find< adcontrols::QuanCompounds >();
    }
    return *qc;
}

void
QuanDocument::quanCompounds( const adcontrols::QuanCompounds& t )
{
    *pm_ *= t;
}

void
QuanDocument::quanSequence( std::shared_ptr< adcontrols::QuanSequence >& ptr )
{
    quanSequence_ = ptr;
    dirty_flags_[ idQuanSequence ] = true;
    for ( auto& client: clients_ )
        client( idQuanSequence, false );
    addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( ptr->filename() ) );
}

std::shared_ptr< adcontrols::QuanSequence >
QuanDocument::quanSequence()
{
    return quanSequence_;
}

const adcontrols::ProcessMethod& 
QuanDocument::pm() const
{
    return *pm_;
}

adcontrols::ProcessMethod&
QuanDocument::pm()
{
    return *pm_;
}

const adcontrols::ProcessMethod&
QuanDocument::procMethod() const
{
    return *pm_;
}

void
QuanDocument::setProcMethod( adcontrols::ProcessMethod& m )
{
    *pm_ *= m;
    dirty_flags_[ idProcMethod ] = true;
}

std::shared_ptr< QuanPublisher >
QuanDocument::publisher() const
{
    return publisher_;
}

void
QuanDocument::publisher( std::shared_ptr< QuanPublisher >& ptr )
{
    publisher_ = ptr;
}

void
QuanDocument::run()
{
    qtwrapper::waitCursor wait;

    if ( quanSequence_ && quanSequence_->size() > 0 ) {

        if ( auto writer = std::make_shared< QuanDataWriter >( quanSequence_->outfile() ) ) {

            if ( writer->open() ) {
                
                if ( !writer->create_table() ) {
                    QMessageBox::information( 0, "QuanDocument", "Create result table failed" );
                    return;
                }
                
                // deep copy which prepare for a long background process (e.g. chromatogram search...)
                auto dup = std::make_shared< adcontrols::ProcessMethod >( *pm_ );
                //dup->appendMethod( *method_->quanMethod() );      // write data into QtPlatz filesystem region (for C++)
                //dup->appendMethod( *method_->quanCompounds() );   // ibid

                auto que = std::make_shared< QuanProcessor >( quanSequence_, dup );
                exec_.push_back( que );
                
                writer->write( *quanSequence_ );         // save into global space in a result file
                writer->write( *dup );                   // ibid
                
                writer->insert_table( *pm_->find< adcontrols::QuanMethod >() );    // write data into sql table for user query
                writer->insert_table( *pm_->find< adcontrols::QuanCompounds >() ); // write data into sql table for user query
                writer->insert_table( *quanSequence_ );  // ibid
                
                for ( auto it = que->begin(); it != que->end(); ++it ) {
                    ++postCount_;
                    threads_.push_back( std::thread( [que,it,writer] () { QuanSampleProcessor( que.get(), it->second )(writer); } ) );
                }

                // update result outfile name on sequence for next run
                for ( auto& client : clients_ )
                    client( idQuanSequence, true );
            }
        }
    }
}

void
QuanDocument::stop()
{
    std::lock_guard< std::mutex > lock( mutex_ );
    
//    if ( progress_ )
//        progress_->progress.cancel();
    //adwidgets::ProgressWnd::instance()->stop();
}

void
QuanDocument::sample_processed( QuanSampleProcessor * p )
{
    // hear is in a sample processing thread; p is a pointer on thread's local stack (not a heap!!)
    auto processor = p->processor();
    emit onProcessed( processor );
}

void
QuanDocument::handle_processed( QuanProcessor * processor )
{
    std::lock_guard< std::mutex > lock( mutex_ );
    if ( postCount_ && ( --postCount_ == 0 ) ) {

        std::for_each( threads_.begin(), threads_.end(), [] ( std::thread& t ){ t.join(); } );
        threads_.clear();

        if ( auto sequence = processor->sequence() ) {
            boost::filesystem::path database( sequence->outfile() );
            if ( boost::filesystem::exists( database ) ) {
                adfs::filesystem fs;
                if ( fs.mount( database.wstring().c_str() ) ) {
                    processor->doCalibration( fs.db() );
                    processor->doQuantification( fs.db() );
                }
            }
        }

        if ( auto sequence = processor->sequence() ) {
            if ( auto connection = std::make_shared< QuanConnection >() ) {
                if ( connection->connect( sequence->outfile() ) )
                    setConnection( connection.get() );
            }
        }

        //adwidgets::ProgressWnd::instance()->hide();

        auto shp = processor->shared_from_this();
        exec_.erase( std::remove( exec_.begin(), exec_.end(), shp ) );

        emit onSequenceCompleted();
    }
}

void
QuanDocument::onInitialUpdate()
{
    if ( !load_default_methods() )
        ADERROR() << "default method load failed";

    for ( auto& client: clients_ ) {
        client( idQuanMethod, true );
        client( idQuanCompounds, true );
        client( idQuanSequence, true );
        client( idProcMethod, true );
    }
}

void
QuanDocument::onFinalClose()
{
    save_default_methods();
}

void
QuanDocument::setMethodFilename( int idx, const std::wstring& filename )
{
    if ( auto qm = pm_->find< adcontrols::QuanMethod >() ) {
        switch ( idx )  {
        case idQuanMethod:
            qm->quanMethodFilename( filename.c_str() );
            dirty_flags_[ idQuanMethod ] = true;
            break;
        case idQuanSequence:
            qm->quanMethodFilename( filename.c_str() );
            dirty_flags_[ idQuanSequence ] = true;
            break;
        }
    }
}

#if 0
void
QuanDocument::method( const QuanMethodComplex& t )
{
    //*method_ = t;

    boost::filesystem::path path( t.filename() );
    settings_->setValue( "MethodFilename", QString::fromStdWString( path.normalize().wstring() ) );

    for ( auto& client: clients_ ) {
        client( idQuanMethod, true );
        client( idQuanCompounds, true );
        client( idProcMethod, true );
    }
}
#endif

void
QuanDocument::replace_method( const adcontrols::QuanMethod& d )
{
    *pm_ *= d;
}

void
QuanDocument::replace_method( const adcontrols::QuanCompounds& d )
{
    *pm_ *= d;
}

void
QuanDocument::replace_method( const adcontrols::ProcessMethod& d )
{
    *pm_ *= d;
    for ( auto& client: clients_ ) {
        client( idQuanMethod, true );
        client( idQuanCompounds, true );
        //client( idQuanSequence, true );
        client( idProcMethod, true );
    }
}

void
QuanDocument::setConnection( QuanConnection * conn )
{
    quanConnection_ = conn->shared_from_this();

    ProgressHandler handler( 0, 5 );
    qtwrapper::waitCursor w;

    if ( ( publisher_ = std::make_shared< QuanPublisher >() ) ) {
        
        Core::ProgressManager::addTask( handler.progress.future(), "Quan connecting database...", Constants::QUAN_TASK_OPEN );
        
        std::thread work( [&] () { (*publisher_)(conn, handler); } );
        
        work.join();

        emit onConnectionChanged();
    }

    addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, QString::fromStdWString( conn->filepath() ) );
}

QuanConnection *
QuanDocument::connection()
{
    return quanConnection_.get();
}

void
QuanDocument::mslock_enabled( bool checked )
{
    emit onMSLockEnabled( checked );
}

QString
QuanDocument::lastMethodDir() const
{
    QString value = recentFile( Constants::GRP_METHOD_FILES, Constants::KEY_REFERENCE );
    if ( value.isEmpty() )
        value = recentFile( Constants::GRP_METHOD_FILES, Constants::KEY_FILES );
    return value;
}

QString
QuanDocument::lastSequenceDir() const
{
    return recentFile( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES );
}

QString
QuanDocument::lastDataDir() const
{
    return recentFile( Constants::GRP_DATA_FILES, Constants::KEY_FILES );
}

void
QuanDocument::addRecentFiles( const QString& group, const QString& key, const QString& value )
{
    std::vector< QString > list;
    getRecentFiles( group, key, list );

    boost::filesystem::path path = boost::filesystem::path( value.toStdWString() ).generic_wstring();
    auto it = std::remove_if( list.begin(), list.end(), [path] ( const QString& a ){ return path == a.toStdWString(); } );
    if ( it != list.end() )
        list.erase( it, list.end() );

    settings_->beginGroup( group );

    settings_->beginWriteArray( key );
    settings_->setArrayIndex( 0 );
    settings_->setValue( "File", QString::fromStdWString( path.generic_wstring() ) );
    for ( size_t i = 0; i < list.size() && i < 7; ++i ) {
        settings_->setArrayIndex( int(i + 1) );
        settings_->setValue( "File", list[ i ] );
    }
    settings_->endArray();

    settings_->endGroup();
}

void
QuanDocument::getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const
{
    settings_->beginGroup( group );

    int size = settings_->beginReadArray( key );
    for ( int i = 0; i < size; ++i ) {
        settings_->setArrayIndex( i );
        list.push_back( settings_->value( "File" ).toString() );
    }
    settings_->endArray();

    settings_->endGroup();
}

QString
QuanDocument::recentFile( const QString& group, const QString& key ) const
{
    QString value;

    settings_->beginGroup( group );
    
    if ( int size = settings_->beginReadArray( key ) ) {
        (void)size;
        settings_->setArrayIndex( 0 );
        value = settings_->value( "File" ).toString();
    }
    settings_->endArray();
    
    settings_->endGroup();

    return value;
}

bool
QuanDocument::load( const boost::filesystem::path& path, adcontrols::QuanSequence& t )
{
    if ( path.extension() == ".xml" ) {

        boost::filesystem::wifstream fi( path );
        try {
            if ( adcontrols::QuanSequence::xml_restore( fi, t ) ) {
                addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_REFERENCE, QString::fromStdWString( path.generic_wstring() ) );
                return true;
            }
        }
        catch ( std::exception& ex ) {
            ADWARN() << boost::diagnostic_information( ex );
        }
        return false;

    } else {

        adfs::filesystem fs;
        if ( !fs.mount( path.wstring().c_str() ) )
            return false;

        if ( auto folder = fs.findFolder( L"/QuanSequence" ) ) {
            if ( auto file = folder.files().back() ) {
                try {
                    if ( file.fetch( t ) ) {
                        addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
                        return true;
                    }
                }
                catch ( std::exception& ex ) {
                    ADWARN() << boost::diagnostic_information( ex );
                }
            }
        }
    }
    return false;
}

bool
QuanDocument::save( const boost::filesystem::path& path, const adcontrols::QuanSequence& t, bool updateSettings )
{
    if ( path.extension() == ".xml" ) {

        boost::filesystem::wofstream fo( path );
        if ( adcontrols::QuanSequence::xml_archive( fo, t ) ) {
            if ( updateSettings )
                addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
            return true;
        }
        return false;

    } else {

        adfs::filesystem fs;
        if ( !fs.create( path.wstring().c_str() ) )
            return false;

        if ( auto folder = fs.addFolder( L"/QuanSequence" ) ) {
            if ( auto file = folder.addFile( adfs::create_uuid(), L"QuanSequence" ) ) {
                file.dataClass( t.dataClass() );
                file.save( t );
                if ( updateSettings )
                    addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
            }
            boost::filesystem::path xmlfile( path );
            xmlfile.replace_extension( path.extension().string() + ".xml" );
            return save( xmlfile, t, false );
        }
    }
    return false;
}

//static
bool
QuanDocument::load( const boost::filesystem::path& path, adcontrols::ProcessMethod& pm )
{
    if ( path.extension() == ".xml" ) {

        try {
            boost::filesystem::wifstream is( path );
            return adcontrols::ProcessMethod::xml_restore( is, pm );
        } catch ( std::exception& ex ) {
            ADWARN() << boost::diagnostic_information( ex );
        }

        return false;

    } else  {

        adfs::filesystem fs;
        if ( !fs.mount( path.wstring().c_str() ) )
            return false;

        if ( auto folder = fs.findFolder( L"/QuanMethod" ) ) {
            if ( auto file = folder.files().back() ) {
                try {
                    if ( file.fetch( pm ) )
                        return true;
                } catch ( std::exception& ex ) {
                    ADWARN() << boost::diagnostic_information( ex );
                }
            }
        }
    }
    return false;
}

//static
bool
QuanDocument::save( const boost::filesystem::path& path, const adcontrols::ProcessMethod& pm )
{
    if ( path.extension() == ".xml" ) {

        try {
            boost::filesystem::wofstream os( path );
            return adcontrols::ProcessMethod::xml_archive( os, pm );
        }
        catch ( std::exception& ex ) {
            ADWARN() << boost::diagnostic_information( ex );
        }
        return false;

    } else {

        adfs::filesystem fs;
        if ( !fs.create( path.wstring().c_str() ) )
            return false;

        if ( auto folder = fs.addFolder( L"/QuanMethod" ) ) {
            if ( auto file = folder.addFile( adfs::create_uuid(), L"QuanMethod" ) ) {
                file.dataClass( pm.dataClass() );
                file.save( pm );
            }
            boost::filesystem::path xmlfile( path );
            xmlfile.replace_extension( path.extension().string() + ".xml" );
            return save( xmlfile, pm );
        }

    }
    return false;
}


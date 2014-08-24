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
#include "quanmethodcomplex.hpp"
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
#include <adportable/portable_binary_oarchive.hpp>
#include <adportable/portable_binary_iarchive.hpp>
#include <adportable/profile.hpp>
#include <adpublisher/document.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <app/app_version.h>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QFuture>

#include <algorithm>

namespace quan {
    namespace detail {

        template< class T > struct method_writer {
            const char * nvp_;
            std::string error_code_;
            method_writer( const char * nvp ) : nvp_( nvp ) {}
            bool operator()( const boost::filesystem::path& file, const T& m ) {
                try {
                    boost::filesystem::wofstream outf( file );
                    boost::archive::xml_woarchive ar( outf );
                    ar << boost::serialization::make_nvp( nvp_, m );
                } catch ( std::exception& ex ) {
                    error_code_ = boost::diagnostic_information( ex );
                    ADERROR() << error_code_;
                    return false;
                }
                return true;
            }
        };

        template< class T > struct method_reader {
            std::string error_code_;
            bool operator()( const boost::filesystem::path& file, T& m ) {
                try {
                    boost::filesystem::wifstream inf( file );
                    boost::archive::xml_wiarchive ar( inf );
                    ar >> BOOST_SERIALIZATION_NVP( m );
                } catch ( std::exception& ex ) {
                    error_code_ = boost::diagnostic_information( ex );
                    ADERROR() << error_code_;
                    return false;
                }
                return true;
            }
        };

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
                             , method_( std::make_shared< QuanMethodComplex >() )
                             , quanSequence_( std::make_shared< adcontrols::QuanSequence >() )
                             , postCount_( 0 )
{
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

    bool a = detail::method_writer<adcontrols::QuanSequence>( "QuanSequence" )( dir / L"QuanSequence.xml", *quanSequence_ );

    bool b = detail::method_writer<QuanMethodComplex>( "QuanMethodComplex" )( dir / L"QuanMethod.xml", *method_ );

    return a && b;
}

bool
QuanDocument::load_default_methods()
{
    QString name = recentFile( Constants::GRP_METHOD_FILES, Constants::KEY_FILES );
    boost::filesystem::path filepath( name.toStdWString() );
    
    if ( boost::filesystem::exists( filepath ) && load( filepath, *method_ ) ) {
        method_->setFilename( filepath.generic_wstring().c_str() );  // just make sure file name is correct
        dirty_flags_[ idMethodComplex ] = false;
    }

    // recovery from backup
    if ( dirty_flags_[ idMethodComplex ] ) {
        boost::filesystem::path dir = detail::user_preference::path( settings_.get() );
        boost::filesystem::path backup = dir / L"QuanMethod.xml";
        if ( boost::filesystem::exists( backup ) && detail::method_reader<QuanMethodComplex>()(backup, *method_) )
            dirty_flags_[ idMethodComplex ] = false;
        // don't update filename
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

const adcontrols::QuanMethod&
QuanDocument::quanMethod() const
{
    return *method_->quanMethod();
}

void
QuanDocument::quanMethod( const adcontrols::QuanMethod& t )
{
    *method_->quanMethod() = t;
    dirty_flags_[ idQuanMethod ] = true;
    for ( auto& client: clients_ )
        client( idQuanMethod, false );
}

const adcontrols::QuanCompounds&
QuanDocument::quanCompounds() const
{
    return *method_->quanCompounds();
}

void
QuanDocument::quanCompounds( const adcontrols::QuanCompounds& t )
{
    *method_->quanCompounds() = t;
    dirty_flags_[ idQuanCompounds ] = true;
    for ( auto& client: clients_ )
        client( idQuanCompounds, false );
}

void
QuanDocument::quanSequence( std::shared_ptr< adcontrols::QuanSequence >& ptr )
{
    quanSequence_ = ptr;
    dirty_flags_[ idQuanSequence ] = true;

    addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( ptr->filename() ) );
}

const adpublisher::document&
QuanDocument::docTemplate() const
{
    return *method_->docTemplate();
}

void
QuanDocument::docTemplate( adpublisher::document& t )
{
    *method_->docTemplate() = t;
}

std::shared_ptr< adcontrols::QuanSequence >
QuanDocument::quanSequence()
{
    return quanSequence_;
}

const adcontrols::ProcessMethod&
QuanDocument::procMethod() const
{
    return *method_->procMethod();
}

void
QuanDocument::setProcMethod( adcontrols::ProcessMethod& m )
{
    *method_->procMethod() = m;
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

bool
QuanDocument::save( const boost::filesystem::path& filepath, const QuanMethodComplex& m )
{
    if ( filepath.extension() == ".xml" ) {

        if ( detail::method_writer<QuanMethodComplex>( "QuanMethodComplex" )(filepath, m) ) {
            addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, QString::fromStdWString( filepath.wstring() ) );
            return true;
        }
        return false;

    } else {
        adfs::filesystem fs;
        try {
            if ( !fs.create( filepath.wstring().c_str() ) )
                return false;
        } catch ( adfs::exception& ex ) {
            QMessageBox::warning( 0, tr( "Save Quan Method" ), QString( ex.message.c_str() ) );
            return false;
        }
        auto folder = fs.addFolder( L"/QuanMethod" );
        auto file = folder.addFile( boost::lexical_cast<std::wstring>(m.ident().uuid()).c_str() );
        file.dataClass( m.dataClass() );
        try {
            if ( file.save<QuanMethodComplex>( m, [] ( std::ostream& os, const QuanMethodComplex& t ){
                        portable_binary_oarchive ar( os );
                        ar << t;
                        return true;
                    } ) ) {
                settings_->setValue( "MethodFiles/Files", QString::fromStdWString( filepath.wstring() ) );
                return true;
            }
        } catch ( std::exception& ex ) {
            QMessageBox::warning( 0, tr( "Save Quan Method" ), boost::diagnostic_information( ex ).c_str() );
        }
    }
    return false;
}

bool
QuanDocument::load( const boost::filesystem::path& filepath, QuanMethodComplex& m )
{
    if ( ! boost::filesystem::exists( filepath ) )
        return false;

    if ( filepath.extension() == ".xml" ) {
        
        if ( !detail::method_reader<QuanMethodComplex>()(filepath, m) )
            return false;

    } else {

        adfs::filesystem fs;
        if ( !fs.mount( filepath.wstring().c_str() ) )
            return false;
        auto folder = fs.findFolder( L"/QuanMethod" );
        auto files = folder.files();
        auto file = files.back();
        try {
            if ( !file.fetch( m ) ) 
                return false;
        } catch ( std::exception& ex ) {
            QMessageBox::warning( 0, tr("Quan loading method file"), boost::diagnostic_information( ex ).c_str() );
            return false;
        }
    }
    boost::filesystem::path normalized( filepath );
    m.setFilename( normalized.wstring().c_str() );
    addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_REFERENCE, QString::fromStdWString( normalized.normalize().wstring() ) );
    return true;
}

bool
QuanDocument::load( const boost::filesystem::path& file, adcontrols::QuanSequence& t )
{
    if ( detail::method_reader<adcontrols::QuanSequence>()(file, t) ) {
        addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_REFERENCE, QString::fromStdWString( file.wstring() ) );
        return true;
    }
    return false;
}

bool
QuanDocument::save( const boost::filesystem::path& file, const adcontrols::QuanSequence& t, bool updateSettings )
{
    if ( detail::method_writer<adcontrols::QuanSequence>( "QuanSequence" )(file, t) ) {
        if ( updateSettings )
            addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( file.wstring() ) );

        return true;
    }
    return false;
}

void
QuanDocument::run()
{
    qtwrapper::waitCursor wait;

    if ( quanSequence_ && quanSequence_->size() > 0 ) {

        // adwidgets::ProgressWnd::instance()->show();
        // adwidgets::ProgressWnd::instance()->raise();
        
        if ( auto writer = std::make_shared< QuanDataWriter >( quanSequence_->outfile() ) ) {

            if ( writer->open() ) {
                
                if ( !writer->create_table() ) {
                    QMessageBox::information( 0, "QuanDocument", "Create result table failed" );
                    return;
                }
                
                // deep copy which prepare for a long background process (e.g. chromatogram search...)
                auto dup = std::make_shared< adcontrols::ProcessMethod >( *method_->procMethod() );
                dup->appendMethod( *method_->quanMethod() );      // write data into QtPlatz filesystem region (for C++)
                dup->appendMethod( *method_->quanCompounds() );   // ibid

                auto que = std::make_shared< QuanProcessor >( quanSequence_, dup );
                exec_.push_back( que );
                
                writer->write( *quanSequence_ );         // save into global space in a result file
                writer->write( *dup );                   // ibid
                
                writer->insert_table( *method_->quanMethod() );    // write data into sql table for user query
                writer->insert_table( *method_->quanCompounds() ); // write data into sql table for user query
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
    switch ( idx )  {
    case idQuanMethod:
        method_->quanMethod()->quanMethodFilename( filename.c_str() );
        dirty_flags_[ idQuanMethod ] = true;
        break;
    case idQuanSequence:
        method_->quanMethod()->quanSequenceFilename( filename.c_str() );
        dirty_flags_[ idQuanSequence ] = true;
        break;
    }
}

const QuanMethodComplex&
QuanDocument::method() const
{
    return *method_;
}

void
QuanDocument::method( const QuanMethodComplex& t )
{
    *method_ = t;

    boost::filesystem::path path( t.filename() );
    settings_->setValue( "MethodFilename", QString::fromStdWString( path.normalize().wstring() ) );

    for ( auto& client: clients_ ) {
        client( idQuanMethod, true );
        client( idQuanCompounds, true );
        client( idProcMethod, true );
    }
}

void
QuanDocument::method( std::shared_ptr< adcontrols::QuanMethod >& ptr )
{
    *method_ = ptr;
}

void
QuanDocument::method( std::shared_ptr< adcontrols::QuanCompounds >& ptr )
{
    *method_ = ptr;
}

void
QuanDocument::method( std::shared_ptr< adcontrols::ProcessMethod >& ptr )
{
    *method_ = ptr;
}

void
QuanDocument::method( std::shared_ptr< adpublisher::document >& ptr )
{
    *method_ = ptr;
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


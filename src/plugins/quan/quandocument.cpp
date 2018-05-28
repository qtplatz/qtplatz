/**************************************************************************
** Copyright (C) 2010-2018 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2018 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include "quancountingprocessor.hpp"
#include "quandatawriter.hpp"
#include "quansampleprocessor.hpp"
#include "quanprocessor.hpp"
#include "quanpublisher.hpp"
#include <adcontrols/centroidmethod.hpp>
#include <adcontrols/centroidprocess.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/mspeakinfo.hpp>
#include <adcontrols/mspeakinfoitem.hpp>
#include <adcontrols/msreference.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/processmethod.hpp>
#include <adcontrols/quancalibration.hpp>
#include <adcontrols/quancompounds.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adfs/filesystem.hpp>
#include <adlog/logger.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/semaphore.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adpublisher/document.hpp>
#include <adwidgets/progressinterface.hpp>
#include <adwidgets/progresswnd.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <coreplugin/progressmanager/progressmanager.h>
#include <boost/exception/all.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <app/app_version.h>
#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QFuture>
#include <algorithm>
#include <future>
#if !defined NDEBUG
#include <workaround/boost/archive/xml_woarchive.hpp>
#endif

namespace quan {
    namespace detail {
        struct user_preference {
            static boost::filesystem::path path( QSettings * settings ) {
                boost::filesystem::path dir( settings->fileName().toStdWString() );
                return dir.remove_filename() / "Quan";                
            }
        };
    }

    template< class... T > struct item_list {};

    typedef item_list< adcontrols::CentroidMethod
                       , adcontrols::MSChromatogramMethod
                       , adcontrols::MSLockMethod  // mslock method to be copied to MSChromatogramMethod
                       , adcontrols::PeakMethod
                       , adcontrols::QuanCompounds
                       , adcontrols::QuanMethod
                       , adcontrols::TargetingMethod
                       , adcontrols::CountingMethod > method_list;

    template< typename last >
    struct item_list< last > {
        static void create( adcontrols::ProcessMethod& p ) { p *= last();  }
    };

    template< typename first, typename ...args >
    struct item_list< first, args...> {
        static void create( adcontrols::ProcessMethod& p ) {
            p *= first();
            item_list< args... >::create( p );
        }
    };
    
}

using namespace quan;

std::mutex QuanDocument::mutex_;

QuanDocument::~QuanDocument()
{
}

QuanDocument::QuanDocument() : settings_( std::make_shared< QSettings >(QSettings::IniFormat, QSettings::UserScope
                                                                        , QLatin1String( Core::Constants::IDE_SETTINGSVARIANT_STR )
                                                                        , QLatin1String( "Quan" ) ) )
                             , postCount_( 0 )
                             , procm_( std::make_unique< adcontrols::ProcessMethod >())
                             , quanSequence_( std::make_shared< adcontrols::QuanSequence >() )
                             , semaphore_( 16 )
{
    method_list::create( *procm_ );

    std::fill( dirty_flags_.begin(), dirty_flags_.end(), true );
    connect( this, &QuanDocument::onProcessed, this, &QuanDocument::handle_processed );
}

QuanDocument *
QuanDocument::instance()
{
    static QuanDocument __instance;
    return &__instance;
}

boost::signals2::connection
QuanDocument::connectDataChanged( const notify_update_t::slot_type& subscriber )
{
    return notify_update_.connect( subscriber );
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

    if ( docTemplate_ )
        save( dir / "QuanDocTemplate.xml", *docTemplate_ );
    
    save( dir / "QuanSequence.xml", *quanSequence_, false ) && save( dir / "QuanMethod.xml", *procm_, false );

    // check if method file in user file space
    QString name = recentFile( Constants::GRP_METHOD_FILES, Constants::KEY_FILES );
    if ( ! name.isEmpty() ) {
        boost::filesystem::path filepath( name.toStdWString() );
        save( filepath, *procm_, false );
    }
    return true;
}

bool
QuanDocument::load_default_methods()
{
    do {
        // .qmth file
        QString name = recentFile( Constants::GRP_METHOD_FILES, Constants::KEY_FILES );

        if ( ! name.isEmpty() ) {
            boost::filesystem::path filepath( name.toStdWString() );
    
            if ( boost::filesystem::exists( filepath ) && load( filepath, *procm_, true ) ) {
                auto qm = procm_->find< adcontrols::QuanMethod >();
                if ( !qm ) {
                    *procm_ << adcontrols::QuanMethod();
                    qm = procm_->find< adcontrols::QuanMethod >();
                }
                qm->quanMethodFilename( filepath.generic_wstring().c_str() ); // update filename with actual path
                dirty_flags_[ idMethodComplex ] = false;
            }
        }
        
        // recovery from backup
        if ( dirty_flags_[ idMethodComplex ] ) {
            boost::filesystem::path dir = detail::user_preference::path( settings_.get() );
            boost::filesystem::path backup = dir / L"QuanMethod.xml";
            if ( boost::filesystem::exists( backup ) && load( backup, *procm_, false ) )
                dirty_flags_[ idMethodComplex ] = false; // don't update filename
        }

    } while ( 0 );

    do {
        // .sequ file
        QString name = recentFile( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES );
        if ( ! name.isEmpty() ) {
            boost::filesystem::path filepath( name.toStdWString() );

            if ( boost::filesystem::exists( filepath ) && load( filepath, *quanSequence_ ) ) {
                dirty_flags_[ idQuanSequence ] = true;
                quanSequence_->filename( filepath.generic_wstring().c_str() ); // just make sure it has correct filename
            }
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
    } while ( 0 );
    
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

void
QuanDocument::replace_method( const adcontrols::QuanMethod& d )
{
    *procm_ *= d;
}

void
QuanDocument::replace_method( const adcontrols::QuanCompounds& d )
{
    *procm_ *= d;
}

void
QuanDocument::replace_method( const adcontrols::ProcessMethod& d )
{
    *procm_ *= d;
    notify_update_( idQuanMethod, true );
    notify_update_( idQuanCompounds, true );
    notify_update_( idProcMethod, true );
}

void
QuanDocument::quanSequence( std::shared_ptr< adcontrols::QuanSequence >& ptr )
{
    quanSequence_ = ptr;
    dirty_flags_[ idQuanSequence ] = true;
    notify_update_( idQuanSequence, false );
    addRecentFiles( Constants::GRP_SEQUENCE_FILES, Constants::KEY_FILES, QString::fromStdWString( ptr->filename() ) );
}

std::shared_ptr< adcontrols::QuanSequence >
QuanDocument::quanSequence()
{
    return quanSequence_;
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
    if ( postCount_ )
        return;
    
    qtwrapper::waitCursor wait;

    if ( quanSequence_ && quanSequence_->size() > 0 ) {

        if ( auto writer = std::make_shared< QuanDataWriter >( quanSequence_->outfile() ) ) {

            if ( writer->open() ) {
                
                if ( !writer->create_table() ) {
                    QMessageBox::information( 0, "QuanDocument", "Create result table failed" );
                    return;
                }
                
                // deep copy which prepare for a long background process (e.g. chromatogram search...)
                auto dup = std::make_shared< adcontrols::ProcessMethod >( *procm_ );

                unsigned int concurrency = std::max( std::thread::hardware_concurrency(), 4u );
                auto que = std::make_shared< QuanProcessor >( quanSequence_, dup, concurrency );
                //auto que = std::make_shared< QuanProcessor >( quanSequence_, dup  );
                exec_.push_back( que );
                
                writer->write( *quanSequence_ );         // save into global space in a result file
                writer->write( *dup );                   // ibid
                
                writer->insert_table( *procm_->find< adcontrols::QuanMethod >() );    // write data into sql table for user query
                writer->insert_table( *procm_->find< adcontrols::QuanCompounds >() ); // write data into sql table for user query
                writer->insert_table( *quanSequence_ );  // ibid
                
                for ( auto it = que->begin(); it != que->end(); ++it ) {
                    ++postCount_;
                    auto p = std::make_shared< adwidgets::ProgressInterface >();
                    Core::ProgressManager::addTask( p->progress.future(), "Processing...", Constants::QUAN_TASK_CALIB );
                    futures_.emplace_back( std::async( std::launch::async, [que,it,writer,p]() {
                                QuanSampleProcessor( que.get(), it->second, p )(writer);
                            } )
                        );
                }
            }
        }
    }
}

void
QuanDocument::execute_counting()
{
    qtwrapper::waitCursor wait;
    
    if ( quanSequence_ && quanSequence_->size() > 0 ) {
        
        // deep copy which prepare for a long background process (e.g. chromatogram search...)
        auto dup = std::make_shared< adcontrols::ProcessMethod >( *procm_ );
        unsigned int concurrency = std::min( std::thread::hardware_concurrency(), 4u );
        auto que = std::make_shared< QuanProcessor >( quanSequence_, dup, concurrency );

        if ( auto writer = std::make_shared< QuanDataWriter >( quanSequence_->outfile() ) ) {
            
            if ( writer->open() ) {
                
                if ( !writer->create_table() ) {
                    QMessageBox::information( 0, "QuanDocument", "Create result table failed" );
                    return;
                }
                
                if ( !writer->create_counting_tables() ) {
                    QMessageBox::information( 0, "QuanDocument", "Create counting table failed" );
                    return;
                }
                
                exec_.push_back( que );

                writer->write( *quanSequence_ ); // save into global space in a result file
                writer->write( *dup );           // ibid
                
                writer->insert_table( *procm_->find< adcontrols::QuanMethod >() );    // write data into sql table for user query
                writer->insert_table( *procm_->find< adcontrols::QuanCompounds >() ); // write data into sql table for user query
                writer->insert_table( *quanSequence_ );  // ibid

                for ( auto it = que->begin(); it != que->end(); ++it ) {
                    ++postCount_;
                    auto p = std::make_shared< adwidgets::ProgressInterface >();
                    Core::ProgressManager::addTask( p->progress.future(), "Processing...", Constants::QUAN_TASK_CALIB );
                    futures_.emplace_back( std::async( std::launch::async, [que,it,writer,p]() {
                                QuanCountingProcessor( que.get(), it->second, p )(writer);
                            } )
                        );
                }
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
QuanDocument::sample_processed( QuanCountingProcessor * p )
{
    // hear is in a sample processing thread; p is a pointer on thread's local stack (not a heap!!)
    auto processor = p->processor();
    emit onProcessed( processor );
}

void
QuanDocument::handle_processed( QuanProcessor * processor )
{
    std::lock_guard< std::mutex > lock( mutex_ );

    ADDEBUG() << "handle_processed(" << postCount_ << ")";
    
    if ( postCount_ && ( --postCount_ == 0 ) ) {

        std::for_each( futures_.begin(), futures_.end(), []( auto& t ){ t.get(); } );
        futures_.clear();

        // std::for_each( threads_.begin(), threads_.end(), [] ( std::thread& t ){ t.join(); } );
        // threads_.clear();

        adwidgets::ProgressInterface p1, p2;
        Core::ProgressManager::addTask( p1.progress.future(), "Calibrating...", Constants::QUAN_TASK_CALIB );
        Core::ProgressManager::addTask( p2.progress.future(), "Determinating...", Constants::QUAN_TASK_QUAN );
        
        auto future = std::async( std::launch::async, [&](){
                if ( auto sequence = processor->sequence() ) {
                    boost::filesystem::path database( sequence->outfile() );
                    if ( boost::filesystem::exists( database ) ) {
                        adfs::filesystem fs;
                        if ( fs.mount( database.wstring().c_str() ) ) {
                            processor->doCalibration( fs.db(), [&](size_t _1, size_t _2){
                                    p1( int(_1), int(_2) ); return false;
                                } );
                            processor->doQuantification( fs.db(), [&](size_t _1, size_t _2){
                                    p2( int(_1), int(_2) ); return false;
                                } );
                        }
                    }
                }

                if ( auto sequence = processor->sequence() ) {
                    if ( auto connection = std::make_shared< QuanConnection >() ) {
                        if ( connection->connect( sequence->outfile() ) )
                            setConnection( connection.get() );
                    }
                }
                
            } );

        while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
            QCoreApplication::instance()->processEvents();

        auto shp = processor->shared_from_this();
        exec_.erase( std::remove( exec_.begin(), exec_.end(), shp ) );

        // ADDEBUG() << "<------------------- Sequence completed -----------------------";
        emit onSequenceCompleted();

        // update result outfile name on sequence for next run
        notify_update_( idQuanSequence, true );
    }
}

void
QuanDocument::onInitialUpdate()
{
    if ( !load_default_methods() )
        ADERROR() << "default method load failed";

    notify_update_( idQuanMethod, true );
    notify_update_( idQuanCompounds, true );
    notify_update_( idQuanSequence, true );
    notify_update_( idProcMethod, true );
}

void
QuanDocument::onFinalClose()
{
    save_default_methods();
}

void
QuanDocument::setMethodFilename( int idx, const std::wstring& filename )
{
    if ( auto qm = procm_->find< adcontrols::QuanMethod >() ) {
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

void
QuanDocument::setConnection( QuanConnection * conn )
{
    quanConnection_ = conn->shared_from_this();

    adwidgets::ProgressInterface handler( 0, 5 );

    if ( ( publisher_ = std::make_shared< QuanPublisher >() ) ) {
        
        Core::ProgressManager::addTask( handler.progress.future(), "Quan connecting database...", Constants::QUAN_TASK_OPEN );

        auto future = std::async( std::launch::async, [&](){ (*publisher_)(conn, handler); } );

        while ( std::future_status::ready != future.wait_for( std::chrono::milliseconds( 100 ) ) )
            QCoreApplication::instance()->processEvents();
        
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
QuanDocument::addRecentDataDir( const QString& dir )
{
    addRecentFiles( Constants::GRP_DATA_FILES, Constants::KEY_FILES, dir );
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
QuanDocument::load( const boost::filesystem::path& path, adcontrols::ProcessMethod& pm, bool updateSettings )
{
    if ( path.extension() == ".xml" ) {

    	boost::system::error_code ec;
    	if ( boost::filesystem::exists( path, ec ) ) {
    		try {
    			boost::filesystem::wifstream is( path );
    			if ( adcontrols::ProcessMethod::xml_restore( is, pm ) ) {
                    if ( updateSettings )
                        addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
                    return true;
                }
    		} catch ( std::exception& ex ) {
    			ADWARN() << boost::diagnostic_information( ex ) << " while loading: " << path;
    		}
    	}
    	return false;

    } else  {

        adfs::filesystem fs;
        if ( !fs.mount( path.wstring().c_str() ) )
            return false;

        if ( auto folder = fs.findFolder( L"/QuanMethod" ) ) {
            if ( auto file = folder.files().back() ) {
                try {
                    if ( file.fetch( pm ) ) {
                        if ( updateSettings )
                            addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
                        return true;
                    }
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
QuanDocument::save( const boost::filesystem::path& path, const adcontrols::ProcessMethod& pm, bool updateSettings )
{
    if ( path.extension() == ".xml" ) {

    	boost::system::error_code ec;
    	if ( boost::filesystem::exists( path, ec ) ) {
    		boost::filesystem::path backup(path);
    		backup.replace_extension( ".old.xml" );
    		boost::filesystem::rename( path, backup, ec );
    	}
        try {
            boost::filesystem::wofstream fo( path );
            if ( adcontrols::ProcessMethod::xml_archive( fo, pm ) ) {
                if ( updateSettings )
                    addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
                return true;
            }
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
                if ( file.save( pm ) ) {
                    if ( updateSettings )
                        addRecentFiles( Constants::GRP_METHOD_FILES, Constants::KEY_FILES, QString::fromStdWString( path.generic_wstring() ) );
                }
            }
            boost::filesystem::path xmlfile( path );
            xmlfile.replace_extension( path.extension().string() + ".xml" );
            return save( xmlfile, pm, false );
        }

    }
    return false;
}

bool
QuanDocument::load( const boost::filesystem::path& path, adpublisher::document& doc )
{
    return doc.load_file( path.string().c_str() );
}

bool
QuanDocument::save( const boost::filesystem::path& path, const adpublisher::document& doc )
{
    return doc.save_file( path.string().c_str() );
}

namespace quan {

    template<> const adcontrols::QuanMethod *
    QuanDocument::getm() const
    {
        return procm_->find< adcontrols::QuanMethod >();
    }

    template<> const adcontrols::QuanCompounds *
    QuanDocument::getm() const
    {
        return procm_->find< adcontrols::QuanCompounds >();
    }

    template<> const adcontrols::MSLockMethod *
    QuanDocument::getm() const
    {
        return procm_->find< adcontrols::MSLockMethod >();
    }
    
    template<> const adcontrols::ProcessMethod *
    QuanDocument::getm() const
    {
        return procm_.get();
    }
    
    template<> void
    QuanDocument::setm< adcontrols::QuanMethod >( const adcontrols::QuanMethod& t )
    {
        *procm_ *= t;

        dirty_flags_[ idQuanMethod ] = true;
        notify_update_( idQuanMethod, false );
    }

    template<> void
    QuanDocument::setm< adcontrols::QuanCompounds >( const adcontrols::QuanCompounds& t )
    {
        *procm_ *= t;
    }

    template<> void
    QuanDocument::setm<>( const adcontrols::ProcessMethod& t )
    {
        *procm_ *= t;
    }
}

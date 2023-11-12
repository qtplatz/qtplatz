/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <adportable/semaphore.hpp>
#include <QObject>
#include <boost/signals2/signal.hpp>
#include <array>
#include <atomic>
#include <filesystem>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>



namespace adcontrols {
    class QuanMethod; class QuanCompounds;
    class QuanSequence; class QuanSample;
    class ProcessMethod; class QuanProcessor;
}

namespace adpublisher { class document; }
namespace boost { namespace filesystem { class path; } }

class QSettings;

namespace quan {

    namespace detail { class dataWriter; }

    class PanelData;
    class QuanSampleProcessor;
    class QuanCountingProcessor;
    class QuanExportProcessor;
    class QuanProcessor;
    class QuanConnection;
    class QuanPublisher;

    enum idMethod { idMethodComplex, idQuanMethod, idQuanCompounds, idProcMethod, idQuanSequence, idSize };

    class document : public QObject {
        Q_OBJECT
    private:
        ~document();
        document();
        static std::mutex mutex_;
    public:
        static document * instance();

        PanelData * addPanel( int idx, int subIdx, std::shared_ptr< PanelData >& );
        PanelData * findPanel( int idx, int subIdx, int pos );

        void setMethodFilename( int idx, const std::wstring& filename );

        template< typename T >  const T* getm() const;
        template< typename T >  void setm( const T& t );

        void quanSequence( std::shared_ptr< adcontrols::QuanSequence >& );
        std::shared_ptr< adcontrols::QuanSequence > quanSequence();

        void publisher( std::shared_ptr< QuanPublisher >& );
        std::shared_ptr< QuanPublisher > publisher() const;

        std::shared_ptr< adpublisher::document > docTemplate() const;
        void docTemplate( std::shared_ptr< adpublisher::document >& );

        typedef boost::signals2::signal< void( int, bool ) > notify_update_t;

        boost::signals2::connection connectDataChanged( const notify_update_t::slot_type& );

        void setResultFile( const std::wstring& );
        void mslock_enabled( bool );

        void setConnection( QuanConnection * );
        QuanConnection * connection();

        void replace_method( const adcontrols::QuanMethod& );
        void replace_method( const adcontrols::QuanCompounds& );
        void replace_method( const adpublisher::document& );
        void replace_method( const adcontrols::ProcessMethod& );

        void execute_spectrogram_export();
        void execute_counting();
        void run();
        void stop();

        void onInitialUpdate();
        void onFinalClose();

        void handle_processed( QuanProcessor * );  // UI thread
        void sample_processed( QuanSampleProcessor * ); // within a sample process thread
        void sample_processed( QuanCountingProcessor * ); // within a sample process thread
        void sample_processed( QuanExportProcessor * );

        QSettings * settings() { return settings_.get(); }
        std::shared_ptr< QSettings > settings_ptr() { return settings_; }
        QString lastMethodDir() const;
        QString lastSequenceDir() const;
        QString lastDataDir() const;
        void addRecentDataDir( const QString& );

        bool load( const std::filesystem::path&, adcontrols::QuanSequence& );
        bool save( const std::filesystem::path&, const adcontrols::QuanSequence&, bool updateSettings );

        bool load( const std::filesystem::path&, adcontrols::ProcessMethod&, bool );
        bool save( const std::filesystem::path&, const adcontrols::ProcessMethod&, bool );

        static bool load( const std::filesystem::path&, adpublisher::document& );
        static bool save( const std::filesystem::path&, const adpublisher::document& );

    private:
        typedef std::vector< std::shared_ptr< PanelData > > page_type;
        typedef std::map< int, page_type > chapter_type;
        std::map< int, chapter_type > book_;
        std::shared_ptr< QSettings > settings_;
        std::shared_ptr< QuanConnection > quanConnection_;
        std::unique_ptr< adcontrols::ProcessMethod > procm_;
        std::shared_ptr< adcontrols::QuanSequence > quanSequence_;
        notify_update_t notify_update_;

        std::array< bool, idSize > dirty_flags_;

        std::atomic< size_t > postCount_;
        std::vector< std::future<void > > futures_;

        std::shared_ptr< QuanPublisher > publisher_;
        std::shared_ptr< adpublisher::document > docTemplate_;

        std::vector< std::shared_ptr< QuanProcessor > > exec_;
        adportable::semaphore semaphore_;

        bool processed( adcontrols::QuanSample& );

        bool save_default_methods();
        bool load_default_methods();

        void addRecentFiles( const QString& group, const QString& key, const QString& value );
        void getRecentFiles( const QString& group, const QString& key, std::vector<QString>& list ) const;
        QString recentFile( const QString& group, const QString& key ) const;

    signals:
        void onProcessed( QuanProcessor * );
        void onMSLockEnabled( bool );
        void onSequenceCompleted();
        void onConnectionChanged();
    };
}

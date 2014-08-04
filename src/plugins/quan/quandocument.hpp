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

#ifndef QUANDOCUMENT_HPP
#define QUANDOCUMENT_HPP

#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <QObject>

namespace adcontrols { class QuanMethod; class QuanCompounds; class QuanSequence; class QuanSample; class ProcessMethod; class QuanProcessor; }
namespace adwidgets { class ProgressWnd; }
namespace boost { namespace filesystem { class path; } }

namespace quan {
    
    namespace detail { class dataWriter; }

    class PanelData;
    class QuanSampleProcessor;
    class QuanProcessor;
    class QuanConnection;

    enum idMethod { idQuanMethod, idQuanCompounds, idQuanSequence, idProcMethod, idSize };

    class QuanDocument : public QObject {
        Q_OBJECT
    private:
        ~QuanDocument();
        QuanDocument();
        static std::atomic< QuanDocument * > instance_;
        static std::mutex mutex_;
    public:
        static QuanDocument * instance();

        PanelData * addPanel( int idx, int subIdx, std::shared_ptr< PanelData >& );
        PanelData * findPanel( int idx, int subIdx, int pos );

        void setMethodFilename( int idx, const std::wstring& filename );

        const adcontrols::QuanMethod& quanMethod() const;
        void quanMethod( const adcontrols::QuanMethod & );

        const adcontrols::QuanCompounds& quanCompounds() const;
        void quanCompounds( const adcontrols::QuanCompounds& );

        void quanSequence( std::shared_ptr< adcontrols::QuanSequence >& );
        std::shared_ptr< adcontrols::QuanSequence > quanSequence();

        const adcontrols::ProcessMethod& procMethod() const;
        void setProcMethod( adcontrols::ProcessMethod& );

        void register_dataChanged( std::function< void( int, bool ) > );
        void setResultFile( const std::wstring& );
        void mslock_enabled( bool );

        void setConnection( QuanConnection * );
        QuanConnection * connection();

        void run();
        void stop();

        bool load( const boost::filesystem::path&, adcontrols::QuanMethod& );
        bool save( const boost::filesystem::path&, const adcontrols::QuanMethod& );

        bool load( const boost::filesystem::path&, adcontrols::QuanCompounds& );
        bool save( const boost::filesystem::path&, const adcontrols::QuanCompounds& );

        bool load( const boost::filesystem::path&, adcontrols::QuanSequence& );
        bool save( const boost::filesystem::path&, const adcontrols::QuanSequence& );

        bool load( const boost::filesystem::path&, adcontrols::ProcessMethod& );
        bool save( const boost::filesystem::path&, const adcontrols::ProcessMethod& );

        void onInitialUpdate();
        void onFinalClose();

        void handle_processed( QuanProcessor * );  // UI thread
        void sample_processed( QuanSampleProcessor * ); // within a sample process thread

    private:
        typedef std::vector< std::shared_ptr< PanelData > > page_type;
        typedef std::map< int, page_type > chapter_type;
        std::map< int, chapter_type > book_;

        std::shared_ptr< adcontrols::QuanMethod > quanMethod_;
        std::shared_ptr< adcontrols::QuanCompounds > quanCompounds_;
        std::shared_ptr< adcontrols::QuanSequence > quanSequence_;
        std::shared_ptr< adcontrols::ProcessMethod > procMethod_;
        std::shared_ptr< QuanConnection > quanConnection_;

        std::vector< std::function< void( int, bool ) > > clients_;

        std::array< bool, idSize > dirty_flags_;

        std::vector< std::thread > threads_;
        std::atomic< size_t > postCount_;

        std::vector< std::shared_ptr< QuanProcessor > > exec_;

        bool processed( adcontrols::QuanSample& );

        bool save_default_methods();
        bool load_default_methods();
    signals:
        void onProcessed( QuanProcessor * );
        void onReportTriggered( const QString& );
        void onMSLockEnabled( bool );
        void onSequenceCompleted();
        void onConnectionChanged();
    };
}

#endif // QUANDOCUMENT_HPP

/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2015 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <QObject>
#include <atomic>
#include <memory>
#include <set>
#include <mutex>

class QSettings;

namespace adcontrols {
    namespace ControlMethod { class Method; }
    class SampleRun;
}

namespace acquire {

    class MainWindow; 

    class document : public QObject {
        Q_OBJECT
        explicit document(QObject *parent = 0);
        static std::atomic< document * > instance_;
        static std::mutex mutex_;
    public:
        static document * instance();
        ~document();
        
        void initialSetup();
        void finalClose( MainWindow * );
        QSettings * settings() { return settings_.get(); }
        void addToRecentFiles( const QString& );
        QString recentFile( const char * group = 0, bool dir_on_fail = false );

        std::shared_ptr< adcontrols::ControlMethod::Method > controlMethod() const;
        void setControlMethod( const adcontrols::ControlMethod::Method&, const QString& filename = QString() );

        std::shared_ptr< adcontrols::SampleRun > sampleRun() const;
        void setSampleRun( const adcontrols::SampleRun&, const QString& filename = QString() );

        static bool load( const QString& filename, adcontrols::ControlMethod::Method& );
        static bool save( const QString& filename, const adcontrols::ControlMethod::Method& );
        static bool load( const QString& filename, adcontrols::SampleRun& );
        static bool save( const QString& filename, const adcontrols::SampleRun& );

        // fsm actions
        void fsmStart();
        void fsmStop();
        void fsmSetMainWindow( MainWindow * );
        void fsmActPrepareForRun();
        void fsmActRun();
        void fsmActStop();
        void fsmActInject();
        void notify_ready_for_run( const char * xml );
    private:
        std::shared_ptr< QSettings > settings_;  // user scope settings
        std::shared_ptr< adcontrols::ControlMethod::Method > cm_;
        std::shared_ptr< adcontrols::SampleRun > sampleRun_;
        QString ctrlmethod_filename_;
        QString samplerun_filename_;

        class impl;
        impl * impl_;
        
    signals:
        void onControlMethodChanged( const QString& );
        void onSampleRunChanged( const QString&, const QString& );
        void onSampleRunLength( const QString& );
        void instStateChanged( int );
        //void requestCommitMethods();
        void sampleRunChanged();

    public slots:
        
    };

}


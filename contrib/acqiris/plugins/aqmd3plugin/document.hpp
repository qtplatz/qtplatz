/**************************************************************************
** Copyright (C) 2010-2020 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2020 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include <QObject>
#include <deque>
#include <mutex>
#include <memory>
#include <vector>

class QSettings;

namespace adextension { class iController; class iSequenceImpl; }

namespace adcontrols {
    namespace ControlMethod { class Method; }
    struct seconds_t;
    class MappedImage;
    class MassSpectrum;
    class MassSpectrometer;
    class TraceAccessor;
    class Trace;
    class SampleRun;
    class threshold_method;
    class TofChromatogramsMethod;
}

namespace aqmd3controls { class method; class waveform; class threshold_result; class tdcdoc; }
namespace boost { namespace uuids { struct uuid; } namespace filesystem { class path; } }

namespace aqmd3 {

    class iAQMD3Impl;

    namespace detail { struct remover; }

    class document : public QObject {

        Q_OBJECT
        document();

    public:
        ~document();

        static document * instance();

        void initialSetup();
        void finalClose();

        bool isRecording() const;
        void actionSyncTrig();
        void actionRun();
        void actionStop();
        void actionRec( bool );
        void actionConnect();
        void actionInject();

        void prepare_for_run();
        void start_run();
        void stop();

        int32_t device_status() const;

        static bool appendOnFile( const boost::filesystem::path& path
                                  , const QString& title, const adcontrols::MassSpectrum&, QString& id );

        void addToRecentFiles( const QString& );
        QString recentFile( const char * group = 0, bool dir_on_fail = false );
        std::shared_ptr< const adcontrols::ControlMethod::Method > controlMethod() const;

        void setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename );
        void setControlMethod( std::shared_ptr< adcontrols::ControlMethod::Method > m, const QString& filename = QString() );

        void addInstController( adextension::iController * p );

        //void setControllerState( const QString& module, bool );
        bool isControllerEnabled( const QString& module ) const;

        std::shared_ptr< const aqmd3controls::method > method() const;

        double triggers_per_second() const;
        size_t unprocessed_trigger_counts() const;
        void save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist );

        void set_threshold_method( int ch, const adcontrols::threshold_method& );
        void set_method( const aqmd3controls::method& );

        adextension::iSequenceImpl * iSequence();
        // [[deprecated]] aqmd3::iAQMD3Impl * iController();
        std::vector< adextension::iController * > iControllers() const;

        std::shared_ptr< const adcontrols::SampleRun > sampleRun() const;
        std::shared_ptr< adcontrols::SampleRun > sampleRun();
        void setSampleRun( std::shared_ptr< adcontrols::SampleRun > );

        void setData( const boost::uuids::uuid& objid, std::shared_ptr< adcontrols::MassSpectrum >, unsigned int idx );
        void commitData();

        std::shared_ptr< adcontrols::MassSpectrum > recentSpectrum( const boost::uuids::uuid& uuid, int idx );

        void takeSnapshot();
        void applyTriggered();
        void setMethod( const adcontrols::TofChromatogramsMethod& );
        void getTraces( std::vector< std::shared_ptr< adcontrols::Trace > >& );
        void progress( double elapsed_time, std::shared_ptr< const adcontrols::SampleRun >&& );

        aqmd3controls::tdcdoc * tdc();
        QSettings * settings();

        void addCountingChromatogramsPoint( uint64_t timeSinceEpoch, uint32_t serialnumber, const std::vector<uint32_t>& );
        // std::shared_ptr< const adcontrols::TofChromatogramsMethod > tofChromatogramsMethod() const;

        // tentative solution -- will be removed
        void result_to_file( std::shared_ptr< aqmd3controls::threshold_result > ch1 );
        // <---

        std::shared_ptr< const adcontrols::MassSpectrometer > massSpectrometer() const;

        static bool load( const QString& filename, adcontrols::ControlMethod::Method& );
        static bool load( const QString& filename, aqmd3controls::method& );
        static bool save( const QString& filename, const adcontrols::ControlMethod::Method& );
        static bool save( const QString& filename, const aqmd3controls::method& );


    private:
        void prepare_next_sample( std::shared_ptr< adcontrols::SampleRun > run, const adcontrols::ControlMethod::Method& cm );

    private slots:
        void handleMessage( adextension::iController *, uint32_t code, uint32_t value );

    private:
        class impl;
        impl * impl_;

    signals:
        void on_reply( const QString&, const QString& );
        void on_waveform_received();
        void onControlMethodChanged( const QString& );
        void on_threshold_method_changed( int );
        void sampleRunChanged();

        void dataChanged( const boost::uuids::uuid&, int );
        void instStateChanged( int );
        void onModulesFailed( const QStringList& );
    };

}

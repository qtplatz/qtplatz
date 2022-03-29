/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef DOCUMENT_HPP
#define DOCUMENT_HPP

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
    class threshold_action;
    class TofChromatogramsMethod; // deprecated
    class XChromatogramsMethod;
    class TimeDigitalHistogram;
}

namespace acqrscontrols { namespace u5303a { class method; class waveform; class threshold_result; class tdcdoc; } }
namespace boost { namespace uuids { struct uuid; } namespace filesystem { class path; } }

namespace accutof {
    namespace acquire {

        class iControllerImpl;

        namespace detail { struct remover; }

        typedef std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > pkdavg_waveforms_t;

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

            void save_defaults();

            static bool appendOnFile( const boost::filesystem::path& path
                                      , const QString& title, const adcontrols::MassSpectrum&, QString& id );

            void addToRecentFiles( const QString& );
            QString recentFile( const char * group = 0, bool dir_on_fail = false );
            std::shared_ptr< const adcontrols::ControlMethod::Method > controlMethod() const;

            void setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename );
            void setControlMethod( std::shared_ptr< adcontrols::ControlMethod::Method > m, const QString& filename = QString() );

            void addInstController( adextension::iController * p );

            QVector< QPair< QString, bool > > controllerSettings() const;
            void setControllerSettings( const QString& module, bool );
            bool isControllerEnabled( const QString& module ) const;
            bool isControllerBlocked( const QString& module ) const;

            std::shared_ptr< const acqrscontrols::u5303a::method > method() const;

            double triggers_per_second() const;
            size_t unprocessed_trigger_counts() const;
            void save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist );

            void set_threshold_method( int ch, const adcontrols::threshold_method& );
            void set_threshold_action( const adcontrols::threshold_action& );
            void set_method( const acqrscontrols::u5303a::method& );

            adextension::iSequenceImpl * iSequence();
            std::vector< adextension::iController * > iControllers();

            std::shared_ptr< const adcontrols::SampleRun > activeSampleRun() const;
            std::shared_ptr< const adcontrols::SampleRun > sampleRun() const;
            std::shared_ptr< adcontrols::SampleRun > sampleRun();
            void setSampleRun( std::shared_ptr< adcontrols::SampleRun > );

            void setData( const boost::uuids::uuid& objid, std::shared_ptr< adcontrols::MassSpectrum >, unsigned int idx );
            void commitData();

            std::shared_ptr< adcontrols::MassSpectrum > recentSpectrum( const boost::uuids::uuid& uuid, int idx );

            void takeSnapshot();
            void applyTriggered();
            void setMethod( const adcontrols::TofChromatogramsMethod& );
            void setMethod( std::shared_ptr< const adcontrols::XChromatogramsMethod > );
            std::shared_ptr< const adcontrols::XChromatogramsMethod > xChromatogramsMethod() const;
            void getTraces( std::vector< std::shared_ptr< adcontrols::Trace > >& );
            void progress( double elapsed_time, std::shared_ptr< const adcontrols::SampleRun >&& );

            static acqrscontrols::u5303a::tdcdoc * tdc();
            QSettings * settings();

#if XCHROMATOGRAMSMETHOD
            void addChromatogramsPoint( std::shared_ptr< const adcontrols::XChromatogramsMethod >
                                        , pkdavg_waveforms_t );
            void addCountingChromatogramPoints( std::shared_ptr< const adcontrols::XChromatogramsMethod >
                                                , const std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& );
#endif
#if TOFCHROMATOGRAMSMETHOD
            void addCountingChromatogramPoints( const adcontrols::TofChromatogramsMethod&
                                                , const std::vector< std::shared_ptr< const adcontrols::TimeDigitalHistogram > >& );
            void addChromatogramsPoint( const adcontrols::TofChromatogramsMethod&, pkdavg_waveforms_t );
#endif
            size_t enqueue( pkdavg_waveforms_t&& );
            size_t dequeue( std::vector< pkdavg_waveforms_t >& );

            // tentative solution -- will be removed
            void result_to_file( std::shared_ptr< acqrscontrols::u5303a::threshold_result > );
            void waveforms_to_file( std::array< std::shared_ptr< const acqrscontrols::u5303a::waveform >, 2 > );
            // <---

            // copy from infitof2 document
            void onChromatogramChanged();
            // <--

            std::shared_ptr< const adcontrols::MassSpectrometer > massSpectrometer() const;
            double countRate( int idx ) const;

            // pkd display (draw) settings
            void setPKDSpectrumEnabled( bool );
            void setLongTermHistogramEnabled( bool );
            bool pkdSpectrumEnabled() const;
            bool longTermHistogramEnabled() const;

            bool hasDark() const;
            void clearDark();
            void acquireDark();

            // this method store niter massspectrometer nor calibration into document;
            std::shared_ptr< adcontrols::MassSpectrometer > setMSCalibFile( const QString& );
            QString msCalibFile() const;

            void handleDefferedWrite( const std::string& stem, size_t remain, size_t progress );

            static bool load( const QString& filename, adcontrols::ControlMethod::Method& );
            static bool load( const QString& filename, acqrscontrols::u5303a::method& );
            static bool save( const QString& filename, const adcontrols::ControlMethod::Method& );
            static bool save( const QString& filename, const acqrscontrols::u5303a::method& );

        private:
            void prepare_next_sample( std::shared_ptr< adcontrols::SampleRun > run, const adcontrols::ControlMethod::Method& cm );

        public slots:
            void handleAutoZeroXICs();
            void handleClearXICs();
            void handleSampleRun();
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
            void on_threshold_action_changed();
            void sampleRunChanged();
            // void sampleProgress( double elapsed_time, double method_time, const QString& runName, int, int );
            void dataChanged( const boost::uuids::uuid&, int );
            void traceChanged( const boost::uuids::uuid& );
            void instStateChanged( int );
            void onModulesFailed( const QStringList& );
            void moduleConfigChanged();
            void drawSettingChanged(); // <- setPKDSpectrumEnabled, setLongTermHistogramEnabled
            void traceSettingChanged( int id, bool );
            void onTick( const QByteArray );
            void onDelayPulseData( const QByteArray );
            void darkStateChanged( int );
            void msCalibrationLoaded( const QString& );
            void onDefferedWrite( const QString&, int, int );
        };
    }
}

#endif // DOCUMENT_HPP

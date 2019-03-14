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
#include <ostream>
//#include <ads54j/constants.hpp>
#include <adportable/mass_assign_t.hpp>

class QSettings;
class QByteArray;

namespace adacquire { class SampleProcessor; }
namespace map { struct trigger_data; }

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
    class TimeDigitalHistogram;
}

namespace boost { namespace uuids { struct uuid; } namespace filesystem { class path; } }

namespace socfpga { namespace dgmod { struct advalue; } }

namespace acquire {

    class iControllerImpl;

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

        //void setControllerState( const QString& module, bool );
        bool isControllerEnabled( const QString& module ) const;
        bool isControllerBlocked( const QString& module ) const;
        void setControllerSettings( const QString& module, bool );
        QVector< QPair< QString, bool > > controllerSettings() const;

        std::shared_ptr< const adcontrols::SampleRun > sampleRun() const;
        std::shared_ptr< adcontrols::SampleRun > sampleRun();
        void setSampleRun( std::shared_ptr< adcontrols::SampleRun > );

        QSettings * settings();

        void acquire_ip_addr( const QString& host, const QString& port );
        void addInstController( adextension::iController * p );
        void addInstController( std::shared_ptr< adextension::iController > p );
        std::vector< adextension::iController * > iControllers() const;
        adextension::iSequenceImpl * iSequence();

        std::ostream& console();
        void setConsole( std::ostream& p );
        void acquire_apply( const QByteArray& );

        bool poll();

        void set_pkd_threshold( double d );

        void set_threshold_method( const QByteArray& json, int ch );
        void set_threshold_action( const QByteArray& json );
        const QJsonDocument& threshold_method() const;
        const QJsonDocument& threshold_action() const;
        QByteArray acquire_method() const;

        void set_tof_chromatograms_method( const QByteArray& json );
        QByteArray tof_chromatograms_method() const;

        void commitData(); // call from task

        void progress( double elapsed_time, std::shared_ptr< const adcontrols::SampleRun >&& sampleRun ) const;

        // Action handlers
        void takeSnapshot();

        // copy from pkdavgacquire
        std::pair< QString, QString > http_addr() const;
        void set_http_addr( const QString&, const QString& );

        static bool write( const adacquire::SampleProcessor& sp, std::unique_ptr< map::trigger_data >&& );
        static void debug_write( const std::vector< std::pair< std::string, std::string > >& headers, const map::trigger_data& );
        // locally added for acquire debugging
        void debug_sse( const std::vector< std::pair< std::string, std::string> >& headers, const std::string& body );
        void debug_data( const std::vector< socfpga::dgmod::advalue >& );
    private:
        void prepare_next_sample( std::shared_ptr< adcontrols::SampleRun > run, const adcontrols::ControlMethod::Method& cm );
        bool prepareStorage( const boost::uuids::uuid& uuid, adacquire::SampleProcessor& sp ) const;
        bool closingStorage( const boost::uuids::uuid&, adacquire::SampleProcessor& ) const;
        void loadControllerSettings();

    private slots:
        void handleMessage( adextension::iController *, uint32_t code, uint32_t value );
        void handleConnected( adextension::iController * controller );
    public slots:
        void handleConsoleIn( const QString& line );

    private:
        class impl;
        impl * impl_;

    signals:
        void onTick( const QByteArray );
        void onDelayPulseData( const QByteArray );

        void on_reply( const QString&, const QString& );
        void on_waveform_received();
        void onControlMethodChanged( const QString& );
        void sampleRunChanged();

        void dataChanged( const boost::uuids::uuid&, int );
        void instStateChanged( int );
        void onModulesFailed( const QStringList& );
        void moduleConfigChanged();

        void on_threshold_action_changed( const QJsonDocument& );
        void on_threshold_method_changed( const QJsonDocument& );
        void on_threshold_level_changed( double );
    };

}

#endif // DOCUMENT_HPP

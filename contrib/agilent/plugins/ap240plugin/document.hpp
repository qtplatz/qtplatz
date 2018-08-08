/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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
#include <memory>

class QSettings;

namespace boost { namespace uuids { struct uuid; } }

namespace adcontrols {
    class MassSpectrum;
    class threshold_method;
    namespace ControlMethod { class Method; }
}

namespace adextension { class iSequenceImpl; }
namespace acqrscontrols {
    template< typename T > class threshold_result_;
    namespace ap240 {
        class waveform;
        class method;
    }
    namespace aqdrv4 {
        class waveform;
    }
}


namespace ap240 {

    typedef acqrscontrols::threshold_result_< acqrscontrols::ap240::waveform > ap240_threshold_result;
    typedef std::shared_ptr< ap240_threshold_result > ap240_threshold_result_ptr;

    class digitizer;
    class iControllerImpl;
    class tdcdoc;
    
    namespace detail { struct remover; }

    class document : public QObject {
        Q_OBJECT
        document();
        ~document();
    public:
        static document * instance();

        void initialSetup();
        void finalClose();

        void actionConnect();
        void ap240_start_run();
        void ap240_stop();
        void ap240_trigger_inject();
        void prepare_for_run();

        tdcdoc * tdc();

        typedef std::pair< ap240_threshold_result_ptr, ap240_threshold_result_ptr > waveforms_t;

        waveforms_t findWaveform( uint32_t serialnumber = (-1) );
        std::shared_ptr< adcontrols::MassSpectrum > getHistogram( int channel, double rs ) const;

        std::shared_ptr< const acqrscontrols::aqdrv4::waveform > findAqDrv4Waveform() const;
        
        int32_t device_status() const;

        static bool toMassSpectrum( adcontrols::MassSpectrum&, const acqrscontrols::ap240::waveform& );
        static bool appendOnFile( const std::wstring& path, const std::wstring& title, const adcontrols::MassSpectrum&, std::wstring& id );

        QSettings * settings() { return settings_.get(); }
        void addToRecentFiles( const QString& );
        QString recentFile( const char * group = 0, bool dir_on_fail = false );
        std::shared_ptr< acqrscontrols::ap240::method> controlMethod() const;
        void setControlMethod( const acqrscontrols::ap240::method& m, const QString& filename );

        std::shared_ptr< const adcontrols::threshold_method> threshold_method( int ch ) const;
        void set_threshold_method( int ch, const adcontrols::threshold_method& );
        
        void save_histogram( size_t tick, const adcontrols::MassSpectrum& );
        void waveform_drawn(); // <<-- GUI round trip mesurement purpose

        double triggers_per_second() const;
        size_t unprocessed_trigger_counts() const;

        adextension::iSequenceImpl * iSequence();
        ap240::iControllerImpl * iController();

        bool isControllerEnabled( const QString& module_name ) const;

        static bool load( const QString& filename, acqrscontrols::ap240::method& );
        static bool save( const QString& filename, const acqrscontrols::ap240::method& );

        void setData( const boost::uuids::uuid& objid, std::shared_ptr< adcontrols::MassSpectrum >, unsigned int idx );
        void commitData();
        
    private:
        class impl;
        impl * impl_;
        
        ap240::digitizer * digitizer_;
        
        std::shared_ptr< acqrscontrols::ap240::method > method_;

        int32_t device_status_;
        std::shared_ptr< QSettings > settings_;  // user scope settings
        QString ctrlmethod_filename_;

        void reply_handler( const std::string&, const std::string& );
        bool waveform_handler( const acqrscontrols::ap240::waveform *, const acqrscontrols::ap240::waveform *, acqrscontrols::ap240::method& );
        void handle_blob( const std::vector< std::pair< std::string, std::string > >& headers, const std::string& blob );
        void handle_sse( const std::vector< std::pair< std::string, std::string > >& headers, const std::string& body );
        void handle_aqdrv4_waveforms( const std::vector< std::shared_ptr< acqrscontrols::aqdrv4::waveform > >& vec );
        void worker_thread();
    signals:
        void on_reply( const QString&, const QString& );
        void on_waveform_received();
        void on_status( int );
        void onControlMethodChanged( const QString& );
        void on_threshold_method_changed( int ch );
        void sampleRunChanged();
        void on_aqdrv4_waveforms();
    };

}

#endif // DOCUMENT_HPP

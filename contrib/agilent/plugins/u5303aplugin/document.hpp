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
#include <mutex>
#include <memory>
#include <deque>

class QSettings;

namespace adcontrols { class MassSpectrum; namespace ControlMethod { class Method; } class threshold_method; }

namespace acqrscontrols { namespace u5303a { class method; } }

namespace u5303a {

    class digitizer;
	class method;
	class waveform;

    namespace detail { struct remover; }

    class document : public QObject {
        Q_OBJECT
        document();
        ~document();
    public:
        static document * instance();

        void initialSetup();
        void finalClose();

        void u5303a_connect();
        void u5303a_start_run();
        void u5303a_stop();
        void u5303a_trigger_inject();
        void prepare_for_run();
        std::shared_ptr< const waveform > findWaveform( uint32_t serialnumber = (-1) );
        int32_t device_status() const;

        static bool toMassSpectrum( adcontrols::MassSpectrum&, const waveform& );
        static bool appendOnFile( const std::wstring& path, const std::wstring& title, const adcontrols::MassSpectrum&, std::wstring& id );

        QSettings * settings() { return settings_.get(); }
        void addToRecentFiles( const QString& );
        QString recentFile( const char * group = 0, bool dir_on_fail = false );
        std::shared_ptr< adcontrols::ControlMethod::Method > controlMethod() const;
        void setControlMethod( const adcontrols::ControlMethod::Method& m, const QString& filename );

        std::shared_ptr< const acqrscontrols::u5303a::method > method() const;
        std::shared_ptr< adcontrols::MassSpectrum > getHistogram( double rs = 0.0 ) const;

        double triggers_per_second() const;
        size_t unprocessed_trigger_counts() const;
        void save_histogram( size_t tickCount, const adcontrols::MassSpectrum& hist );

        void set_threshold_method( int ch, const adcontrols::threshold_method& );

        static bool load( const QString& filename, adcontrols::ControlMethod::Method& );
        static bool load( const QString& filename, acqrscontrols::u5303a::method& );
        static bool save( const QString& filename, const adcontrols::ControlMethod::Method& );
        static bool save( const QString& filename, const acqrscontrols::u5303a::method& );
        
    private:
        friend struct detail::remover;
        class exec;

        class impl;
        impl * impl_;

        static std::mutex mutex_;
        static document * instance_;

        u5303a::digitizer * digitizer_;
        std::deque< std::shared_ptr< const waveform > > que_;
        std::shared_ptr< adcontrols::ControlMethod::Method > cm_;
        std::shared_ptr< acqrscontrols::u5303a::method > method_;
        std::unique_ptr< exec > exec_;

        int32_t device_status_;
        std::shared_ptr< QSettings > settings_;  // user scope settings
        QString ctrlmethod_filename_;

        void reply_handler( const std::string&, const std::string& );
        bool waveform_handler( const waveform *, u5303a::method& );
    signals:
        void on_reply( const QString&, const QString& );
        void on_waveform_received();
        void on_status( int );
        void onControlMethodChanged( const QString& );
        void on_threshold_method_changed( int );
    };

}

#endif // DOCUMENT_HPP

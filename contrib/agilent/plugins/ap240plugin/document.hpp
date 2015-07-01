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

namespace adcontrols { class MassSpectrum; class ControlMethod; }

namespace ap240 {

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

        void ap240_connect();
        void ap240_start_run();
        void ap240_stop();
        void ap240_trigger_inject();
        void prepare_for_run();
        std::shared_ptr< const waveform > findWaveform( uint32_t serialnumber = (-1) );
        int32_t device_status() const;

        static bool toMassSpectrum( adcontrols::MassSpectrum&, const waveform& );
        static bool appendOnFile( const std::wstring& path, const std::wstring& title, const adcontrols::MassSpectrum&, std::wstring& id );

        QSettings * settings() { return settings_.get(); }
        void addToRecentFiles( const QString& );
        QString recentFile( const char * group = 0, bool dir_on_fail = false );
        std::shared_ptr< adcontrols::ControlMethod > controlMethod() const;
        void setControlMethod( const adcontrols::ControlMethod& m, const QString& filename );

        static bool load( const QString& filename, adcontrols::ControlMethod& );
        static bool save( const QString& filename, const adcontrols::ControlMethod& );
        
    private:
        friend struct detail::remover;
        class exec;

        static std::mutex mutex_;
        static document * instance_;

        ap240::digitizer * digitizer_;
        std::deque< std::shared_ptr< const waveform > > que_;
        std::shared_ptr< adcontrols::ControlMethod > cm_;
        std::unique_ptr< exec > exec_;

        int32_t device_status_;
        std::shared_ptr< QSettings > settings_;  // user scope settings
        QString ctrlmethod_filename_;

        void reply_handler( const std::string&, const std::string& );
        bool waveform_handler( const waveform *, ap240::method& );
    signals:
        void on_reply( const QString&, const QString& );
        void on_waveform_received();
        void on_status( int );
        void onControlMethodChanged( const QString& );
    };

}

#endif // DOCUMENT_HPP

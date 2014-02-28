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

namespace adcontrols { class MassSpectrum; }

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

        void u5303a_connect();
        void u5303a_start_run();
        void u5303a_stop();
        void u5303a_trigger_inject();
        void prepare_for_run();
        void prepare_for_run( const u5303a::method& );
        std::shared_ptr< const waveform > findWaveform( uint32_t serialnumber = (-1) );
        const u5303a::method& method() const;
        int32_t device_status() const;

        static bool toMassSpectrum( adcontrols::MassSpectrum&, const waveform& );
        static bool appendOnFile( const std::wstring& path, const std::wstring& title, const adcontrols::MassSpectrum&, std::wstring& id );
        
    private:
        friend struct detail::remover;

        static std::mutex mutex_;
        static document * instance_;

        u5303a::digitizer * digitizer_;
        std::deque< std::shared_ptr< const waveform > > que_;
        std::shared_ptr< u5303a::method > method_;
        int32_t device_status_;

        void reply_handler( const std::string&, const std::string& );
        void waveform_handler( const waveform * );
    signals:
        void on_reply( const QString&, const QString& );
        void on_waveform_received();
        void on_status( int );
    };

}

#endif // DOCUMENT_HPP

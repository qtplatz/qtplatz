/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <QObject>

class QSettings;
namespace adextension  { class iSessionManager; }
namespace adprocessor  { class dataprocessor; }
namespace adcontrols   { class MappedImage; }
namespace adcontrols { namespace adcv { class ContoursMethod; } }
namespace portfolio    { class Folium; }
namespace boost {
    namespace filesystem { class path; }
    namespace uuids { struct uuid; }
}

namespace video {

    class Player;
    class processor;

    class document : public QObject {
        Q_OBJECT
    private:
        ~document();
        document();
        static std::mutex mutex_;
    public:
        static document * instance();

        void initialSetup();
        void finalClose();

        QSettings& settings();
        QString lastDataDir() const;

        void setCurrentFile( const QString& filename );

        void setRectOnImage( const QRect& );
        const QRect rectOnImage() const;

        bool openFile( const QString& filename, QString& errorMessage );

        Player * player();
        Player * camera();

        void captureCamera();

        std::shared_ptr< processor > currentProcessor();
        void setContoursMethod( QString&& json );
        const adcontrols::adcv::ContoursMethod& contoursMethod() const;

        bool zScaleAutoEnabled() const;
        double zScale() const;

    public slots:
        void setZScaleAutoEnabled( bool );
        void setZScale( double );

    private:
        std::unique_ptr< QSettings > settings_;
        std::unique_ptr< Player > player_;
        std::unique_ptr< Player > camera_;

    signals:
        void fileChanged( const QString& );
        void cameraChanged();
    private:
        class impl;
        impl * impl_;
    };
}

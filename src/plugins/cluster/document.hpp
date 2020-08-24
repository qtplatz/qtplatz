/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MALPIXDOCUMENT_HPP
#define MALPIXDOCUMENT_HPP

#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <QObject>

class QSettings;
// namespace mpxprocessor { class processor; }
// namespace mpxcontrols  { class HistogramMethod; }
namespace adextension  { class iSessionManager; }
namespace adprocessor  { class dataprocessor; }
namespace adcontrols   { class MappedImage; class MappedSpectra; class ContoursMethod; }
namespace portfolio    { class Folium; }
namespace boost {
    namespace filesystem { class path; }
    namespace uuids { struct uuid; }
}

class ContoursMethod;

namespace cluster {

    class Player;

    enum ZMapId : int;

    class document : public QObject {
        Q_OBJECT
    private:
        ~document();
        document();
        static std::mutex mutex_;
    public:
        static document * instance();

        std::shared_ptr< adprocessor::dataprocessor > currentProcessor();

        void initialSetup();
        void finalClose();

        QSettings& settings();
        QString lastDataDir() const;

        void setCurrentFile( const QString& filename );

        void setRectOnImage( const QRect& );
        const QRect rectOnImage() const;

        //
        void attach_file( const QString& dbfile, const QString& attach_filepath );

        enum plotType { spectrum, timetrace };
        enum axisType { axis_mz, axis_time };

        axisType axis( plotType ) const;
        void setAxis( plotType, axisType );

        //
        void setCellSelection( const QRect& );
        void setCellSelectionEnabled( bool );
        void setHistogramWindowEnabled( bool );
        bool histogramWindowEnabled() const;
        void setHistogramWindow( double tof, double width );
        std::pair< double, double > histogramWindow() const;

        //const mpxcontrols::HistogramMethod& histogramMethod() const;

        void setMappedImage( std::shared_ptr< const adcontrols::MappedImage >
                             , std::shared_ptr< const adcontrols::MappedSpectra >
                             , const std::pair< double, double >& trig
                             , const std::pair< double, double>& tof );

        std::shared_ptr< const adcontrols::MappedImage > mappedImage() const;
        std::shared_ptr< const adcontrols::MappedSpectra > mappedSpectra() const;

        Player * player();

        // Non 'HistogramMethod' values
        void setZAutoScaleEnable( ZMapId id, bool );
        bool zAutoScaleEnable( ZMapId id ) const;

        void setZScale( ZMapId id, uint32_t );
        uint32_t zScale( ZMapId id ) const;

        uint32_t zAutoScale( ZMapId id ) const;
        adcontrols::ContoursMethod& contoursMethod();
        const adcontrols::ContoursMethod& contoursMethod() const;
        void updateContoursMethod();
        void setMomentsEnabled( bool );
        bool momentsEnabled() const;

    public slots:
        // Link with dataproc navigator
        void handleAddProcessor( adextension::iSessionManager *, const QString& file );
        void handleSelectionChanged( adextension::iSessionManager *, const QString& file, const portfolio::Folium& );  // change node (folium) selection
        void handleProcessed( adextension::iSessionManager *, const QString& file, const portfolio::Folium& );         // data contents changed
        void handleCheckStateChanged( adextension::iSessionManager *, const QString& file, const portfolio::Folium&, bool );

    private:
        class impl;
        impl * impl_;

    signals:
        void currentProcessorChanged();
        void dataChanged( const portfolio::Folium& );
        void checkStateChanged( const portfolio::Folium& );
        void selRangeOnSpectrum( const QRectF& );
        void axisChanged();
        void enableMapRect( bool );
        // void setEnabled( int id, bool ); // id 0 = Map rect, 1 = tof range (check box on group boxes)
        void mappedImageChanged();
        void tofWindowChanged( double, double );
        void tofWindowEnabled( bool );
        void contoursMethodChanged();
    };
}

#endif // MALPIXDOCUMENT_HPP

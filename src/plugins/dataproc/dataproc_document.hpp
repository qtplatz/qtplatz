/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef DATAPROC_DOCUMENT_HPP
#define DATAPROC_DOCUMENT_HPP

#include <QObject>
#include <atomic>
#include <memory>
#include <set>
#include <mutex>

class QSettings;

namespace adcontrols {
    class MSQPeaks;
    class Chromatogram;
    class ProcessMethod;
    class LCMSDataset;
    class DataReader;
    class DataReader_iterator;
}

namespace dataproc {

    class Dataprocessor;
    class DataprocessorFactory;

    class dataproc_document : public QObject
    {
        Q_OBJECT
        
        explicit dataproc_document(QObject *parent = 0);
        
        static std::atomic<dataproc_document * > instance_;
        static std::mutex mutex_;
    public:
        static dataproc_document * instance();
        
        void initialSetup();
        void finalClose();

        void setMSQuanTable( const adcontrols::MSQPeaks& );
        adcontrols::MSQPeaks * msQuanTable();
        const adcontrols::MSQPeaks * msQuanTable() const;

        std::shared_ptr< adcontrols::ProcessMethod > processMethod() const;
        void setProcessMethod( const adcontrols::ProcessMethod&, const QString& filename = QString() );

        QSettings * settings() { return settings_.get(); }

        void addToRecentFiles( const QString& );
        QString recentFile( const char * group = 0, bool dir_on_fail = false );

        void saveScanLaw( const QString& model_name, double flength, double accv, double tdelay, double mass, const QString& );
        bool findScanLaw( const QString& model_name, double& flength, double& accv, double& tdelay, double& mass, QString& );

        void setDataprocessorFactory( std::unique_ptr< DataprocessorFactory >&& );
        DataprocessorFactory * dataprocessorFactory();

        void handleSelectTimeRangeOnChromatogram( double x1, double x2 );

        static bool load( const QString& filename, adcontrols::ProcessMethod& );
        static bool save( const QString& filename, const adcontrols::ProcessMethod& );

        static size_t findCheckedTICs( Dataprocessor *, std::set< int >& vfcn );
        static const std::shared_ptr< adcontrols::Chromatogram > findTIC( Dataprocessor *, int );

        void onSelectSpectrum_v2( double minutes, size_t pos, int fcn );
        void onSelectSpectrum_v3( double minutes, const adcontrols::DataReader_iterator& );

    public slots:
        void handle_folium_added( const QString& fname, const QString& path, const QString& id );
        void handle_portfolio_created( const QString& token );
        
    private:    
        std::shared_ptr< adcontrols::MSQPeaks > quant_;
        std::shared_ptr< QSettings > settings_;  // user scope settings
        std::shared_ptr< adcontrols::ProcessMethod > pm_;
        QString procmethod_filename_;
        std::unique_ptr< DataprocessorFactory > dataprocFactory_;
        
        void handleSelectTimeRangeOnChromatogram_v2( Dataprocessor *, const adcontrols::LCMSDataset *, double x1, double x2 );
        void handleSelectTimeRangeOnChromatogram_v3( Dataprocessor *, const adcontrols::LCMSDataset *, double x1, double x2 );

    signals:
        void onProcessMethodChanged( const QString& );
        void scanLawChanged( double length, double accV, double tdelay );

    };

}

#endif // DATAPROCDOCUMENT_HPP

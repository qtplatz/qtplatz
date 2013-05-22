// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#ifndef DATAPROCESSOR_H
#define DATAPROCESSOR_H

#include <coreplugin/ifile.h>
#include <boost/smart_ptr.hpp>
#include <adcontrols/datasubscriber.hpp>
#include "constants.hpp"

namespace adcontrols {
    class datafile;
    class LCMSDataset;
    class ProcessMethod;
    class MassSpectrum;
    class Chromatogram;
    class MSAssignedMasses;
    class MSCalibrateMethod;
    class CentroidMethod;
}

namespace portfolio {
    class Portfolio;
    class Folium;
    class Folder;
}

namespace SignalObserver { class Observer; }

namespace dataproc {

    class IFileImpl;
    class datafileObserver_i;

    class Dataprocessor : QObject
                        , public adcontrols::dataSubscriber {
        Q_OBJECT
    public:
        ~Dataprocessor();
        Dataprocessor();

        bool create( const QString& token );
        bool open( const QString& );
        Core::IFile * ifile();

        // QString filename() const;
		const std::wstring& filename() const;
        adcontrols::datafile& file();
        const adcontrols::LCMSDataset* getLCMSDataset();
        portfolio::Portfolio getPortfolio();
		bool fetch( portfolio::Folium& );
        void setCurrentSelection( portfolio::Folium& );
        void setCurrentSelection( portfolio::Folder& );
        void applyProcess( const adcontrols::ProcessMethod&, enum ProcessType );
        void applyCalibration( const adcontrols::ProcessMethod& );
        void applyCalibration( const adcontrols::ProcessMethod&, const adcontrols::MSAssignedMasses&  );
        void applyCalibration( const adcontrols::ProcessMethod&, const adcontrols::MSAssignedMasses&, portfolio::Folium&  );
        portfolio::Folium addSpectrum( const adcontrols::MassSpectrum&, const adcontrols::ProcessMethod& );
        portfolio::Folium addChromatogram( const adcontrols::Chromatogram&, const adcontrols::ProcessMethod& );
        SignalObserver::Observer * observer();

        // implement adcontrols::dataSubscriber
        virtual bool subscribe( const adcontrols::LCMSDataset& );
        virtual bool subscribe( const adcontrols::ProcessedDataset& );
        // <------------------------
    private:
        void addCalibration( const adcontrols::MassSpectrum&, const adcontrols::ProcessMethod& );
        void addCalibration( const adcontrols::MassSpectrum& profile
                             , const adcontrols::MassSpectrum& centroid
                             , const adcontrols::MSCalibrateMethod&, const adcontrols::MSAssignedMasses& );

    signals:
        // void changeSelection( portfolio::Folium& );

    public slots:
        // void slotTitleChanged( const QString& title );
        // void handle_changeSelection( portfolio::Folium& );

    private:
        boost::scoped_ptr< IFileImpl > ifileimpl_;
        boost::scoped_ptr< portfolio::Portfolio > portfolio_;
		boost::scoped_ptr< datafileObserver_i > fileObserver_;
        std::wstring idActiveFolium_;
    };

} // dataproc

#endif // DATAPROCESSOR_H

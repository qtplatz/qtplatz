// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
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

#include <coreplugin/idocument.h>
#include <adcontrols/datasubscriber.hpp>
#include <adprocessor/dataprocessor.hpp>
#include "constants.hpp"
#include <functional>
#include <memory>
#include <vector>
#include <tuple>

namespace adcontrols {
    class datafile;
    class LCMSDataset;
    class ProcessMethod;
    class MassSpectrum;
    class MassSpectra;
    class Chromatogram;
    class MSAssignedMasses;
    class MSCalibrateMethod;
	class MSCalibrateResult;
    class CentroidMethod;
    class SpectrogramClusters;
    namespace lockmass { class mslock; }
}

namespace adprocessor {
    class noise_filter;
}

namespace adprot { class digestedPeptides; }

namespace portfolio {
    class Portfolio;
    class Folium;
    class Folder;
}

namespace SignalObserver { class Observer; }

namespace dataproc {

    class IFileImpl;

    class Dataprocessor : public Core::IDocument
                        , public adprocessor::dataprocessor {

        Q_OBJECT

        Dataprocessor( const Dataprocessor& ) = delete;
        Dataprocessor& operator = ( const Dataprocessor& ) = delete;
#if _MSC_VER
	protected:
#endif
        Dataprocessor();
    public:
        ~Dataprocessor();

        static std::shared_ptr< Dataprocessor > make_dataprocessor() {
            struct make_shared_enabler : public Dataprocessor {};
            return std::make_shared< make_shared_enabler >();
        }

        // Core::IDocument
        // QtCreator9 based code
        OpenResult open(QString *errorString
                        , const Utils::FilePath &filePath
                        , const Utils::FilePath &realFilePath) override;
        ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const override;
        bool save(QString *errorString, const Utils::FilePath &filePath = Utils::FilePath(), bool autoSave = false) override;

        bool reload( QString *, Core::IDocument::ReloadFlag, Core::IDocument::ChangeType ) override;
        bool isModified() const override;
        bool isSaveAsAllowed() const override;

        // Dataprocessor
        void xicSelectedMassPeaks( adcontrols::MSPeakInfo&& info ) override;
        void markupMassesFromChromatograms( portfolio::Folium&& folium ) override;
        void clearMarkup( portfolio::Folium&& folium );

        void setModified( bool ) override;
        bool create( const QString& token );
        bool open( const std::filesystem::path&, std::string& errmsg ) override;
        bool open( const QString&, QString& errmsg );

        void exportXML() const;

        portfolio::Portfolio getPortfolio();
        bool load( const std::wstring& path, const std::wstring& id );
		bool fetch( portfolio::Folium& );
        void setCurrentSelection( portfolio::Folium& );
        void setCurrentSelection( portfolio::Folder& );
        void applyProcess( const adcontrols::ProcessMethod&, enum ProcessType );
        void applyProcess( portfolio::Folium&, const adcontrols::ProcessMethod&, enum ProcessType );
        void applyCalibration( const adcontrols::ProcessMethod& );
        void applyCalibration( const adcontrols::ProcessMethod&, const adcontrols::MSAssignedMasses&  );
        void applyCalibration( const adcontrols::ProcessMethod&, const adcontrols::MSAssignedMasses&, portfolio::Folium&  );
        portfolio::Folium currentSelection() const;
        void applyLockMass( std::shared_ptr< adcontrols::MassSpectra > ); // spectrogram data
        void exportMatchedMasses( std::shared_ptr< adcontrols::MassSpectra >, const std::wstring& foliumId ); // spectrogram data
        portfolio::Folium addProfiledHistogram( portfolio::Folium& ); // replace if already exist
        portfolio::Folium findProfiledHistogram( const portfolio::Folium& );

        // apply calibration to entire dataset
        void applyCalibration( const adcontrols::MSCalibrateResult& );
		void lockMassHandled( const std::wstring& foliumId, const std::shared_ptr< adcontrols::MassSpectrum >&, const adcontrols::lockmass::mslock& );
        adcontrols::lockmass::mslock doMSLock( portfolio::Folium&
                                               , std::shared_ptr< adcontrols::MassSpectrum >
                                               , const std::vector< std::pair< int, int > >& );
        void formulaChanged();

        void sendCheckedSpectraToCalibration( Dataprocessor * );
        void remove( portfolio::Folium );
        void createContour();
        void clusterContour();
        void findPeptide( const adprot::digestedPeptides& );
        void findSinglePeak( portfolio::Folium, std::pair< double, double > trange = { -1, 3600.0 } );
        void baselineCollection( portfolio::Folium );
        void setAttribute( portfolio::Folium, std::pair< std::string, std::string >&& );
        void deleteRemovedItems();
        void dftFilter( portfolio::Folium, std::shared_ptr< adcontrols::ProcessMethod > ); // filter for chromatogram

        // portfolio::Folium addSpectrum( const adcontrols::MassSpectrum&, const adcontrols::ProcessMethod& );
        portfolio::Folium addSpectrum( std::shared_ptr< adcontrols::MassSpectrum >, const adcontrols::ProcessMethod& );
        portfolio::Folium addSpectrum( std::shared_ptr< const adcontrols::MassSpectrum >, const adcontrols::ProcessMethod& );
        portfolio::Folium addChromatogram( std::shared_ptr< adcontrols::Chromatogram >
                                           , const adcontrols::ProcessMethod&
                                           , std::shared_ptr< adprocessor::noise_filter > );
        // [[deprecated]] portfolio::Folium addChromatogram( const adcontrols::Chromatogram&, const adcontrols::ProcessMethod& );
        portfolio::Folium addContour( std::shared_ptr< adcontrols::MassSpectra > );
        portfolio::Folium addContourClusters( std::shared_ptr< adcontrols::SpectrogramClusters > );

        void subtract( portfolio::Folium& base, portfolio::Folium& target );

        static const std::shared_ptr< adcontrols::ProcessMethod > findProcessMethod( const portfolio::Folium& );
        static bool MSCalibrationLoad( const QString&, adcontrols::MSCalibrateResult&, adcontrols::MassSpectrum& );
        static bool MSCalibrationSave( portfolio::Folium&, const QString& file );

        // implement adcontrols::dataSubscriber
        void notify( adcontrols::dataSubscriber::idError, const std::string& json ) override;

        //
		bool onFileAdded( const std::wstring& path, adfs::file& ) override;
        // <------------------------
    private:
        void addCalibration( const adcontrols::MassSpectrum&, const adcontrols::ProcessMethod& );
        void addCalibration( const adcontrols::MassSpectrum& profile
                             , const adcontrols::MassSpectrum& centroid
                             , const adcontrols::MSCalibrateMethod&, const adcontrols::MSAssignedMasses& );

    private slots:

    signals :
        void onNotify( const QString& );
        void openFinished(bool success);

    private:
        class impl;
        std::unique_ptr< impl > impl_;

        void setDisplayName( const QString& );
    };

} // dataproc

#endif // DATAPROCESSOR_H

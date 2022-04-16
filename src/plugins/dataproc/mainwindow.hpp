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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <utils/fancymainwindow.h>
#include "constants.hpp"
#include <array>
#include <functional>
#include <memory>
#include <adwidgets/mspeaktable.hpp> // callback type

namespace adportable { class Configuration; }
namespace adcontrols { class datafile; class ProcessMethod; class MSAssignedMasses; class MSPeaks; class MassSpectrum; }
namespace adprot { class digestedPeptides; }
namespace portfolio { class Folium; }
namespace Core { class IMode; }
namespace Utils { class StyledBar; }
class QString;
class QHBoxLayout;
class QWidget;
class QToolButton;
class QAction;
class QStackedWidget;
class QLineEdit;
class QComboBox;

namespace adextension { class iController; class iSequenceImpl; }

namespace dataproc {

    class Dataprocessor;
    class iSequenceImpl;
    class AboutDlg;
    class document;

    class MainWindow : public Utils::FancyMainWindow {
        Q_OBJECT
    public:
        ~MainWindow();
        explicit MainWindow(QWidget *parent = 0);

        static MainWindow * instance();

        void contextMenuEvent( QContextMenuEvent * ) override { /* disable */ }; // from Utils::FancyMainWindow <- QMainWindow <= QWidget

        QWidget * createContents( Core::IMode * );
        // if you are looking for 'create_actions', you should see dataproc::ActionManager

        void activateLayout();
        void setSimpleDockWidgetArrangement();

        static QToolButton * toolButton( const char * );
        static QToolButton * toolButton( QAction *, const QString& objectName = QString() );

        void getProcessMethod( adcontrols::ProcessMethod& );
		void setProcessMethod( const adcontrols::ProcessMethod& );

        void OnInitialUpdate();
        void OnFinalClose();

        void applyCalibration( const adcontrols::MSAssignedMasses& );
        void applyCalibration( const adcontrols::MSAssignedMasses&, portfolio::Folium& );

        int currentProcessView( std::string& ) const;
        void printCurrentView( const QString& ) const;

        void saveDefaultMSCalibrateResult( portfolio::Folium& );
        void lockMassHandled( const std::shared_ptr< adcontrols::MassSpectrum >& );
        void dataMayChanged();
        void zoomedOnSpectrum( const QRectF&, adcontrols::hor_axis );

        void proteinSelected( const adprot::digestedPeptides& );
        void setSpectrumAxisChoice( adcontrols::hor_axis );

        // [[deprecated("use make_filename")]]static QString makePrintFilename( const std::wstring& id, const std::wstring& insertor = L"__"
        //                                   , const char * extension = ".svg", const QString& = QString() );
        static void addPrintFileToSettings( const QString& );
        static QString makeDisplayName( const std::wstring& id, const char * insertor = ";", int nbsp = 0 );
        static std::wstring foliumName( const std::wstring& id );
        static std::wstring portfolioFilename( const std::wstring& id );
        static QString currentDir();

        void handleProcessChecked();
        void handleImportChecked();
        void handleExportPeakList();
        void handleExportAllChecked();
        void handleExportRMSAllChecked();

        enum idPage { idSelMSProcess, idSelElementalComp, idSelMSCalibration //, idSelMSCalibSpectra
                      , idSelChromatogram, idSelMSPeaks, idSelSpectrogram, idSelSpectra, idNum };

        void selPage( idPage );
        enum idPage curPage() const;

		// bool editor_factories( iSequenceImpl& );
        void getEditorFactories( adextension::iSequenceImpl& );

        void selectionChanged( std::shared_ptr< adcontrols::MassSpectrum >, std::function<adwidgets::MSPeakTable::callback_t> );

        void repaintTable();

    private:
        void handleProcessed( Dataprocessor *, portfolio::Folium& );

    public slots:
        void handleSessionAdded( Dataprocessor * );
        void handleSelectionChanged( Dataprocessor *, portfolio::Folium& );
        void actionApply();
        void handle_add_mspeaks( const adcontrols::MSPeaks& );
        void actCreateSpectrogram();
		void actClusterSpectrogram();
        void handleWarningMessage( const QString& );
        void aboutQtPlatz();
        void hideDock( bool );

    private slots:
        void handleApplyMethod();
        void handleFeatureSelected( int );
        void handleFeatureActivated( int );
        void handlePeptideTarget( const QVector<QPair<QString, QString> >& );
        void handleProcessMethodChanged( const QString& );
        void handleProcess( const QString& );
        void handleDataprocessor( Dataprocessor * );

        friend class MSPeaksWnd;

    private:
        QWidget * toolBar_;
        QHBoxLayout * toolBarLayout_;
        QComboBox * axisChoice_;

        QAction * actionSearch_;
        QAction * actionApply_;

        QStackedWidget * stack_;
        AboutDlg * aboutDlg_;

        enum ProcessType currentFeature_;

        void setToolBarDockWidget( QDockWidget * dock );
        void createToolbar();
        QDockWidget * createDockWidget( QWidget *, const QString& title = QString(), const QString& pageName = QString() );
        void createDockWidgets();
        Utils::StyledBar * createStyledBarTop();
        Utils::StyledBar * createStyledBarMiddle();
        void currentPageChanged( int );
        void handleScaleYChanged( int );
        void handleScaleY2Changed( int, int );
        void handleScaleX2Changed( int, int );
        void loadSettings();

    signals:
        void onPrintCurrentView( const QString& ) const;
        void onAddMSPeaks( const adcontrols::MSPeaks& ) const;
        void onDataMayCanged() const;
        void onZoomedOnSpectrum( const QRectF&, int axis ) const;
        void onZoomedOnChromatogram( const QRectF& ) const;
        void onScaleYChanged( bool checked, double bottom, double top ) const;
        void onScaleChromatogramYChanged( bool checked, double bottom, double top ) const;
        void onScaleChromatogramXChanged( bool checked, double bottom, double top ) const;
    };

    ///// SpectrumWidget scale (us) -> seconds
    template <bool isTime> struct range_t {};

    template <> struct range_t<true> {
        inline std::pair<double,double> operator()( const QRectF& rect ) const {
            return std::make_pair( rect.left() / std::micro::den, rect.right() / std::micro::den );
        }
        inline std::pair<double,double> operator()( const double& s, const double& e ) const {
            return std::make_pair( s / std::micro::den, e / std::micro::den );
        }
    };
    template<> struct range_t<false> {
        inline std::pair< double, double> operator()( const QRectF& rect ) const {
            return std::make_pair( rect.left(), rect.right() );
        }
        inline std::pair<double,double> operator()( const double& s, const double& e ) const { return std::make_pair( s, e ); }
    };


}

#endif // MAINWINDOW_HPP

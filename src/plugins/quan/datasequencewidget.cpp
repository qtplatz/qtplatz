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

#include "datasequencewidget.hpp"
#include "datasequencetree.hpp"
#include "datasequencetable.hpp"
#include "quandocument.hpp"
#include "paneldata.hpp"
#include "quanconstants.hpp"
#include <utils/styledbar.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/datafile.hpp>
#include <adcontrols/datareader.hpp>
#include <adcontrols/lcmsdataset.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/massspectrum.hpp>
#include <adcontrols/quansequence.hpp>
#include <adcontrols/quanmethod.hpp>
#include <adcontrols/quansample.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adlog/logger.hpp>
#include <adplot/chartview.hpp>
#include <adplot/xyseriesdata.hpp>
#include <adprocessor/dataprocessor.hpp>
#include <adportable/debug.hpp>
#include <adportable/profile.hpp>
#include <adportable/date_string.hpp>
#include <adfs/sqlite.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/format.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/variant.hpp>
#include <QAction>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolButton>
#include <QFileDialog>

namespace quan {

    struct datasequence_type {

        template<typename X> void setData( QWidget * w, const X& x ) {
            if ( auto p = qobject_cast<DataSequenceTree *>( w ) )
                p->setData( x );
            else if ( auto p = w->findChild<DataSequenceTable *>() )
                p->setData( x );
        }
        template<typename X> bool getContents( QWidget * w, X& x ) {
            if ( auto p = qobject_cast<DataSequenceTree *>( w ) )
                return p->getContents( x );
            else if ( auto p = w->findChild<DataSequenceTable *>() )
                return p->getContents( x );
            return false;
        }
        template<typename X> bool setContents( QWidget * w, const X& x ) {
            if ( auto p = qobject_cast<DataSequenceTree *>( w ) )
                return p->setContents( x );
            else if ( auto p = w->findChild<DataSequenceTable *>() )
                return p->setContents( x );
            return false;
        }
        void handleLevelChanged( QWidget * w, int x ) {
            if ( auto p = qobject_cast<DataSequenceTree *>( w ) )
                return p->handleLevelChanged( x );
            else if ( auto p = w->findChild<DataSequenceTable *>() )
                return p->handleLevelChanged( x );
        }
        void handleReplicatesChanged( QWidget * w, int x ) {
            if ( auto p = qobject_cast<DataSequenceTree *>( w ) )
                return p->handleReplicatesChanged( x );
            else if ( auto p = w->findChild<DataSequenceTable *>() )
                return p->handleReplicatesChanged( x );
        }
    };

}

using namespace quan;

DataSequenceWidget::~DataSequenceWidget()
{
}

DataSequenceWidget::DataSequenceWidget(QWidget *parent) : QWidget(parent)
                                                        , layout_( new QGridLayout )
                                                        , stack_(new QStackedWidget( this ))
                                                        , dataSequenceInfusion_( new DataSequenceTree )
                                                        , dataSequenceChromatography_( new DataSequenceTable )
                                                        , levels_(1)
                                                        , replicates_(1)
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    const int row = layout_->rowCount();
    layout_->addWidget( dataSelectionBar(), row, 0 );

    // Chromatography | Counting
    if ( QSplitter * splitter = new QSplitter ) {
        splitter->setOrientation( Qt::Horizontal );
        splitter->addWidget( dataSequenceChromatography_.get() );
        if ( auto chartView = new adplot::ChartView )
            splitter->addWidget( chartView );
        stack_->addWidget( splitter );
    }

    // Infusion
    stack_->addWidget( dataSequenceInfusion_.get() );

    layout_->addWidget( stack_.get() );

    connect( dataSequenceChromatography_.get(), &DataSequenceTable::plot, this, &DataSequenceWidget::handlePlot );

    QuanDocument::instance()->register_dataChanged( [this]( int id, bool fnChanged ){ handleDataChanged( id, fnChanged ); });
}

void
DataSequenceWidget::commit()
{
    if ( auto sequence = std::make_shared< adcontrols::QuanSequence >() ) {
        if ( auto edit = findChild< QLineEdit * >( Constants::editOutfile ) ) {
            sequence->outfile( edit->text().toStdWString().c_str() );
        }
        if ( datasequence_type().getContents( stack_->currentWidget(), *sequence ) )
            QuanDocument::instance()->quanSequence( sequence );
    }
}

QWidget *
DataSequenceWidget::dataSelectionBar()
{
    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );

        // [DATA OPEN]|[SAVE][...line edit...][EXEC]

        auto label = new QLabel;
        auto button = new QToolButton;
        button->setIcon( QIcon( ":/quan/images/fileopen.png" ) );
        button->setToolTip( tr("Open data files...") );
        toolBarLayout->addWidget( button );

        toolBarLayout->addWidget( new Utils::StyledSeparator );

        auto label2 = new QLabel;
        label2->setText( tr( "Results Save in:" ) );
        toolBarLayout->addWidget( label2 );

        auto edit = new QLineEdit;
        edit->setObjectName( Constants::editOutfile );
        toolBarLayout->addWidget( edit );
        
        edit->setClearButtonEnabled( true );
        auto icon = QIcon( ":/quan/images/filesave.png" );
        QAction * tgtFileAction = edit->addAction( icon, QLineEdit::ActionPosition::TrailingPosition );

        if ( Core::ActionManager * am = Core::ActionManager::instance() ) {

            if ( auto execButton = new QToolButton ) {
                execButton->setDefaultAction( am->command( Constants::QUAN_SEQUENCE_RUN )->action() );
                execButton->setToolTip( tr( "Run sequence in batch process" ) );
                toolBarLayout->addWidget( execButton );
            }
            
            if ( auto stopButton = new QToolButton ) {
                stopButton->setDefaultAction( am->command( Constants::QUAN_SEQUENCE_STOP )->action() );
                stopButton->setToolTip( tr( "Stop sequence executeion" ) );
                toolBarLayout->addWidget( stopButton );
            }
        }

        // open datafile(s) 
        connect( button, &QToolButton::clicked, this, [this] ( bool ){

                QFileDialog dlg( 0, tr( "Open data file(s)" ), QuanDocument::instance()->lastDataDir() );

                dlg.setNameFilter( tr("Data Files(*.adfs *.csv *.txt *.spc)") );
                dlg.setFileMode( QFileDialog::ExistingFiles );
                
                if ( dlg.exec() == QDialog::Accepted ) {
                    auto result = dlg.selectedFiles();
                    datasequence_type().setData( stack_->currentWidget(), result );
                    QDir dir( result[ 0 ] );
                    QuanDocument::instance()->addRecentDataDir( dir.absolutePath() );
                }
            } );

        // target file 'data save in'
        connect( tgtFileAction, &QAction::triggered, this, [this] (){
                QString dstfile;
                if ( auto edit = findChild< QLineEdit * >( Constants::editOutfile ) ) {
                    dstfile = edit->text();
                    if ( dstfile.isEmpty() ) {
                        boost::filesystem::path dir( adportable::profile::user_data_dir< wchar_t >() );
                        dstfile = QString::fromStdWString( (dir / L"data").wstring() );
                    }
                    try {
                        QString name = QFileDialog::getSaveFileName( this, "Data save in", dstfile, tr( "Quan result (*.adfs)" ) );
                        if ( !name.isEmpty() ) {
                            if ( auto edit = findChild< QLineEdit *>( Constants::editOutfile ) )
                                edit->setText( name );
                        }
                    }
                    catch ( ... ) {
                        ADTRACE() << "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again.";
                        QMessageBox::information( this, "Quan/DataSequence Edit", "Hit QTBUG-33119 that has no workaround right now.  Please be patient and try it again." );
                    }
                }
            } );

        return toolBar;
    }
    return 0;
}

void
DataSequenceWidget::handleDataChanged( int id, bool fnChanged )
{
    if ( id == idQuanSequence ) { // && fnChanged ) {
        if ( fnChanged ) { // result outfile changed
            if ( auto edit = findChild< QLineEdit * >( Constants::editOutfile ) ) {
                boost::filesystem::path path( QuanDocument::instance()->quanSequence()->outfile() );
                path = path.generic_wstring(); // posix format
                int number = 0;
                if ( boost::filesystem::exists( path ) ) {
                    std::wstring stem = path.stem().wstring();
                    if ( std::isdigit( stem.at( stem.size() - 1 ) ) ) {
                        auto pos = stem.find_last_not_of( L"0123456789" );
                        if ( pos != std::wstring::npos ) {
                            number = std::stoi( stem.substr( pos + 1 ) );
                            stem = stem.substr( 0, pos + 1 );
                        }
                    }
                    boost::filesystem::path next;
                    path.remove_filename();
                    do {
                        next = path / boost::filesystem::path( stem + (boost::wformat( L"%d.adfs" ) % ++number).str() );
                    } while ( boost::filesystem::exists( next ) );
                    path = next.generic_wstring();
                }
                edit->setText( QString::fromStdWString( path.wstring() ) ); // native format
            }
        }
        datasequence_type().setContents( stack_->currentWidget(), *QuanDocument::instance()->quanSequence() );
    }
    if ( id == idQuanMethod ) {
        auto& qm = QuanDocument::instance()->quanMethod();
        levels_ = qm.levels();
        replicates_ = qm.replicates();
        datasequence_type().handleLevelChanged( stack_->currentWidget(), qm.levels() );
        datasequence_type().handleReplicatesChanged( stack_->currentWidget(), qm.replicates() );
    }
}

void
DataSequenceWidget::handleLevelChaged( int value )
{
    levels_ = value;
    datasequence_type().handleLevelChanged( stack_->currentWidget(), value );
}

void
DataSequenceWidget::handleReplicatesChanged( int value )
{
    replicates_ = value;
    datasequence_type().handleReplicatesChanged( stack_->currentWidget(), value );
}

void
DataSequenceWidget::handleSampleInletChanged( int inlet )
{
    if ( adcontrols::QuanSample::Chromatography == inlet ) {
        stack_->setCurrentIndex( 0 );
        if ( auto table = stack_->currentWidget()->findChild< DataSequenceTable * >() )
            table->setSampleInlet( inlet );
    } else if ( adcontrols::QuanSample::Counting == inlet ) {
        stack_->setCurrentIndex( 0 );
        if ( auto table = stack_->currentWidget()->findChild< DataSequenceTable * >() )
            table->setSampleInlet( inlet );        
    } else {
        stack_->setCurrentIndex( 1 ); // Infusion; currently disabled due to a show stopper bug
    }
    datasequence_type().handleLevelChanged( stack_->currentWidget(), int(levels_) );
    datasequence_type().handleReplicatesChanged( stack_->currentWidget(), int(replicates_) );
}

void
DataSequenceWidget::handlePlot( const QString& file )
{
    boost::filesystem::path path( file.toStdString() );

    std::wstring errmsg;
    
    if ( boost::filesystem::exists( path ) ) {
        
        auto dp = std::make_shared< adprocessor::dataprocessor >();

        if ( dp->open( path.wstring(), errmsg ) ) {

            if ( auto hist = dp->readSpectrumFromTimeCount() ) {

                if ( auto chart = findChild< adplot::ChartView * >() ) {
                    auto data = new XYSeriesData();
                    for ( size_t i = 0; i < hist->size(); ++i )
                        (*data) << QPointF( hist->getMass( i ), hist->getIntensity( i ) );
                    chart->clear();
                    chart->setData( data, "Histogram", "m/z", "count(*)", "Line" );
                }
            }
        }
    }
}

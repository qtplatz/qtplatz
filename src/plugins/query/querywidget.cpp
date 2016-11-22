/**************************************************************************
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

#include "querywidget.hpp"
#include "queryconnection.hpp"
#include "queryconstants.hpp"
#include "document.hpp"
#include "queryform.hpp"
#include "queryresulttable.hpp"
#if QT5_CHARTS
# include "charts/chartview.hpp"
#endif
#include "plotdialog.hpp"
#include <adportable/profile.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/scanlaw.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/styledbar.h>
#include <QCompleter>
#include "qwt/chartview.hpp"
#include <QFileDialog>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QSplitter>
#include <QSqlError>
#include <QSqlRecord>
#include <QStringListModel>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QStandardItemModel>
#include <boost/exception/all.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <fstream>
#include <algorithm>

using namespace query;

#if QT5_CHARTS
QT_CHARTS_USE_NAMESPACE;
#endif

QueryWidget::~QueryWidget()
{
}

QueryWidget::QueryWidget(QWidget *parent) : QWidget(parent)
                                                    , layout_( new QGridLayout )
                                                    , form_( new QueryForm )
                                                    , table_( new QueryResultTable )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    connect( document::instance(), &document::onConnectionChanged, this, &QueryWidget::handleConnectionChanged );
    connect( document::instance(), &document::onHistoryChanged, this, [this](){
            form_->setSqlHistory( document::instance()->sqlHistory() );
        } );
    connect( form_.get(), &QueryForm::triggerQuery, this, &QueryWidget::handleQuery );
    connect( table_.get(), &QueryResultTable::plot, this, &QueryWidget::handlePlot );

    if ( auto toolBar = new Utils::StyledBar ) {
        
        layout_->addWidget( toolBar );
        
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 2 );
        toolBarLayout->setSpacing( 2 );
        
        if ( auto btnOpen = new QToolButton ) {
            btnOpen->setDefaultAction( Core::ActionManager::instance()->command( Constants::FILE_OPEN )->action() );
            btnOpen->setToolTip( tr("Open result file...") );
            toolBarLayout->addWidget( btnOpen );

            auto edit = new QLineEdit;
            edit->setReadOnly( true );
            edit->setObjectName( Constants::editQueryFilename );
            toolBarLayout->addWidget( edit );
        }
    }

    if ( QSplitter * splitter = new QSplitter ) {
        splitter->setOrientation( Qt::Vertical );

        splitter->addWidget( form_.get() );

        if ( auto hsplitter = new QSplitter ) {
            hsplitter->setOrientation( Qt::Horizontal );
            hsplitter->addWidget( table_.get() );
#if QT5_CHARTS            
            if ( auto chartView = new charts::ChartView )
                hsplitter->addWidget( chartView );
#else
            if ( auto chartView = new qwt::ChartView ) {
                hsplitter->addWidget( chartView );
                connect( chartView, &qwt::ChartView::makeQuery, this, &QueryWidget::buildQuery );
            }
#endif
            if ( auto w = hsplitter->widget( 1 ) )
                w->hide();

            splitter->addWidget( hsplitter );
        }

        layout_->addWidget( splitter );

        splitter->setStretchFactor( 0, 0 );
        splitter->setStretchFactor( 1, 2 );
    }

    form_->setSqlHistory( document::instance()->sqlHistory() );
}

void
QueryWidget::handleConnectionChanged()
{
    ADDEBUG() << "set file: " << document::instance()->connection()->filepath();

    if ( auto edit = findChild< QLineEdit * >( Constants::editQueryFilename ) ) {
        edit->setText( QString::fromStdWString( document::instance()->connection()->filepath() ) );
    }

    if ( auto conn = document::instance()->connection() ) {
        if ( auto form = findChild< QueryForm * >() ) {
            QStringList tables;
            bool hasPeak( false ), hasTrigger( false );
            auto query = conn->sqlQuery( "SELECT name FROM sqlite_master WHERE type='table'" );
            while ( query.next() ) {
                tables << query.value( 0 ).toString();
                if ( query.value( 0 ).toString().compare( "peak", Qt::CaseInsensitive ) )
                    hasPeak = true;
                else if ( query.value( 0 ).toString().compare( "trigger", Qt::CaseInsensitive ) )
                    hasTrigger = true;
            }
            tables.push_back( "sqlite_master" );

            if ( hasPeak && hasTrigger )
                tables.push_back( "{Counting}" ); // '{}' never appear on sql table name

            form->setTableList( tables );
            
            form->setSqlHistory( document::instance()->sqlHistory() );            
            
            QStringList words ( tables );

            if ( QCompleter * completer = new QCompleter( this ) ) {
                QFile file( ":/query/wordlist.txt" );
                if ( file.open( QFile::ReadOnly ) ) {
                    while ( !file.atEnd() ) {
                        QByteArray line = file.readLine();
                        if ( ! line.isEmpty() )
                            words << line.trimmed();
                    }
                }

                for ( auto& table: tables ) 
                    words << table;
                
                query = conn->sqlQuery( "SELECT objuuid FROM AcquiredConf" );
                while ( query.next() ) 
                    words << query.value( 0 ).toString(); // guid

                words.sort( Qt::CaseInsensitive );
                words.removeDuplicates();

                completer->setModel( new QStringListModel( words, completer ) );
                completer->setModelSorting( QCompleter::CaseInsensitivelySortedModel );
                completer->setCaseSensitivity( Qt::CaseInsensitive );
                completer->setWrapAround( false );
                form->setCompleter( completer );
            }
        }
    }

    executeQuery();
}

void
QueryWidget::executeQuery()
{
    if ( auto connection = document::instance()->connection() ) {
        {
            form_->setSQL( "SELECT * FROM sqlite_master WHERE type='table'" );
            auto query = connection->sqlQuery( form_->sql() );
            table_->setQuery( query, connection->shared_from_this() );
        }
            
        {
            QSqlQuery query( connection->sqlDatabase() );
            query.prepare( "SELECT acclVoltage,tDelay,clsidSpectrometer FROM ScanLaw WHERE spectrometer='InfiTOF' LIMIT 1" );
            if ( query.exec() ) {
                while ( query.next() ) {
                    auto rec = query.record();
                    double acclVoltage = rec.value( 0 ).toDouble();
                    double tDelay = rec.value( 1 ).toDouble();
                    auto uuid = boost::uuids::string_generator()( rec.value( 2 ).toString().toStdString() );
                    if ( auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( uuid ) ) {
                        spectrometer->setScanLaw( acclVoltage, tDelay, 1.0 );
                        document::instance()->setMassSpectrometer( spectrometer );
                        table_->setMassSpectrometer( spectrometer );
                    }
                }
            }
        }
        {
            // see infitof2/document.cpp line 940
            auto idstr = boost::lexical_cast< std::string >( adcontrols::ControlMethod::Method::clsid() );
            QSqlQuery query( connection->sqlDatabase() );
            query.prepare( "SELECT data FROM MetaData WHERE clsid = ?" );
            query.bindValue( 0, QString::fromStdString( idstr ) );
            if ( query.exec() ) {
                while ( query.next() ) {
                    auto rec = query.record();
                    auto blob = rec.value( 0 ).toByteArray();
                    adcontrols::ControlMethod::Method m;
                    boost::iostreams::basic_array_source< char > device( blob.data(), size_t( blob.size() ) );
                    boost::iostreams::stream< boost::iostreams::basic_array_source< char > > strm( device );
                    if ( adcontrols::ControlMethod::Method::restore( strm, m ) ) {
                        if ( auto sp = table_->massSpectrometer() )
                            sp->setMethod( m );
                    }
                }
            }
        }
    }
}

void
QueryWidget::handleQuery( const QString& sql )
{
    if ( auto connection = document::instance()->connection() ) {

        auto query = connection->sqlQuery( sql );

        auto sqlError = query.lastError();
        if ( sqlError.type() != QSqlError::NoError ) {
            QMessageBox::information( this
                                      , tr( "QtPlatz/Query" )
                                      , sqlError.driverText() + "\n" + sqlError.databaseText() );
        } else {
            document::instance()->addSqlHistory( sql );
        }
        
        table_->setQuery( query );
    }
}

void
QueryWidget::handlePlot()
{
#if QT5_CHARTS
    typedef charts::ChartView ChartView_t;
#else
    typedef qwt::ChartView ChartView_t;
#endif
    
    if ( auto chart = findChild< ChartView_t * >() ) {

        PlotDialog dlg( this );

        dlg.setModel( table_->model() );
        
        auto& settings = document::instance()->settings();
        settings.beginGroup( "PlotDialog" );
        dlg.setClearExisting( settings.value( "clearExisting", true ).toBool() );
        dlg.setChartType( settings.value( "chartType", "Histogram" ).toString() );
        size_t size = settings.beginReadArray( "plot" );
        for ( size_t i = 0; i < size; ++i ) {
            settings.setArrayIndex( i );
            dlg.setPlot( i, settings.value( "title" ).toString(), settings.value( "plot_x" ).toInt(), settings.value( "plot_y" ).toInt() );
        }
        settings.endArray();
        settings.endGroup();
        
        if ( dlg.exec() ) {
            
            settings.beginGroup( "PlotDialog" );
            settings.setValue( "clearExisting", dlg.clearExisting() );
            settings.setValue( "chartType", dlg.chartType() );
            settings.beginWriteArray( "plot" );
            size_t i(0);
            for ( const auto& plot: dlg.plots() ) {
                settings.setArrayIndex( i );
                settings.setValue( "title", std::get< 0 > ( plot ) );
                settings.setValue( "plot_x", std::get< 1 > ( plot ) );
                settings.setValue( "plot_y", std::get< 2 > ( plot ) );
            }
            settings.endArray();
            settings.endGroup();

            if ( dlg.clearExisting() )
                chart->clear();

            auto type = dlg.chartType();
            auto plots = dlg.plots();

            for( auto& plot: plots ) {

                auto title = std::get< 0 >( plot );
                int iX = std::get< 1 >( plot );
                int iY = std::get< 2 >( plot );

                auto xtitle = table_->model()->headerData( iX, Qt::Horizontal ).toString();
                auto ytitle = table_->model()->headerData( iY, Qt::Horizontal ).toString();
                
                chart->setData( table_->model(), title, iX, iY, xtitle, ytitle, type );
            }
            
            if ( auto chart = findChild< ChartView_t * >() )
                chart->show();
        }
    }
}

void
QueryWidget::buildQuery( const QString& q, const QRectF& rc, bool isMass )
{
    qtwrapper::waitCursor wait;
    
    if ( q.contains( "COUNTING" ) ) {

        std::vector< std::pair< int, std::pair< double, double > > > t_ranges;

        if ( auto conn = document::instance()->connection()->shared_from_this() ) {
            std::vector< std::pair< int, std::pair< double, double > > > p_ranges; // protocol tof range
            {
                QSqlQuery sql( "SELECT protocol, min( peak_time ), max( peak_time ) FROM peak,trigger WHERE id=idTrigger GROUP BY protocol"
                               , conn->sqlDatabase() );
                sql.exec();
                while( sql.next() )
                    p_ranges.emplace_back( sql.value( 0 ).toInt(), std::make_pair( sql.value( 1 ).toDouble(), sql.value( 2 ).toDouble() ) );
            }

            if ( isMass ) {
                if ( auto sp = document::instance()->massSpectrometer() ) {
                    if ( auto scanlaw = sp->scanLaw() ) {
                        for ( auto& p_range: p_ranges ) {
                            auto t_range = std::make_pair( scanlaw->getTime( rc.left(), sp->mode( p_range.first ) )
                                                           , scanlaw->getTime( rc.right(), sp->mode( p_range.first ) ) );
                            if ( t_range.first >= p_range.second.first && t_range.second <= p_range.second.second )
                                t_ranges.emplace_back( /* proto */ p_range.first, t_range );
                        }
                    }
                }
            } else {
                for ( auto& p_range: p_ranges ) {
                    if ( rc.left() >= p_range.second.first && rc.right() <= p_range.second.second )
                        t_ranges.emplace_back( /* proto */ p_range.first, std::make_pair( rc.left(), rc.right() ) );
                }
            }
        }

        std::ostringstream stmt;
        if ( t_ranges.size() == 1 && q == "COUNTING" ) {
            const auto& t = t_ranges[0];
            stmt << boost::format("SELECT COUNT(*),protocol FROM peak,trigger WHERE id=idTrigger AND protocol=%d AND peak_time > %.8e AND peak_time < %.8e" )
                % t.first
                % t.second.first
                % t.second.second;

        } else {
            char name('A');
            stmt << "SELECT name,ROUND(peak_intensity/10)*10 AS threshold,avg(peak_time) AS time, avg(peak_intensity) as mV, COUNT(*) FROM (\r\n";
            for ( auto& t: t_ranges ) {
                stmt << "\tSELECT peak_time,peak_intensity, '" << name++ << "' AS name FROM peak,trigger "
                     << boost::format( "WHERE id=idTrigger AND protocol=%d AND peak_time>%.8e AND peak_time < %.8e " )
                    % t.first
                    % t.second.first
                    % t.second.second;
                if ( t_ranges.size() > 1 )
                    stmt << "UNION ALL";
                stmt << "\r\n";
            }
            stmt <<") WHERE peak_intensity < threshold GROUP by name,threshold";            
        }
        form_->setSQL( QString::fromStdString( stmt.str() ) );
    }
}


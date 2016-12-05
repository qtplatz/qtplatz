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
#include "countingquerydialog.hpp"
#include "sqlhistorydialog.hpp"
#include "plotdialog.hpp"
#include <adportable/profile.hpp>
#include <adportable/debug.hpp>
#include <adcontrols/controlmethod.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/massspectrometerbroker.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adplot/chartview.hpp>
#include <qtwrapper/waitcursor.hpp>
#include <qtwrapper/progresshandler.hpp>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/styledbar.h>
#include <QCompleter>
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
                                          , dlg_( new CountingQueryDialog )
                                          , hdlg_( new SqlHistoryDialog )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );
    topLayout->addLayout( layout_ );

    connect( document::instance(), &document::onConnectionChanged, this, &QueryWidget::handleConnectionChanged );
    connect( document::instance(), &document::onHistoryChanged, this, [this](){
            hdlg_->appendSql( document::instance()->sqlHistory() );
        } );
    connect( form_.get(), &QueryForm::triggerQuery, this, &QueryWidget::handleQuery );
    connect( form_.get(), &QueryForm::showHistory, this, &QueryWidget::showHistory );
    connect( table_.get(), &QueryResultTable::plot, this, &QueryWidget::handlePlot );

    dlg_->setModal( false );
    connect( dlg_, &CountingQueryDialog::accepted, this, [this]{ accept(); } );
    connect( dlg_, &CountingQueryDialog::applied, this, [this]{ applyQuery(); } );

    hdlg_->setModal( false );
    connect( hdlg_, &SqlHistoryDialog::accepted, this, [&]{ hdlg_->hide(); } );

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
            if ( auto chartView = new adplot::ChartView ) {
                hsplitter->addWidget( chartView );
                connect( chartView, &adplot::ChartView::makeQuery, this, &QueryWidget::buildQuery );
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

    hdlg_->appendSql( document::instance()->sqlHistory() );
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
            tables.insert( 0, "sqlite_master" );

            if ( hasPeak && hasTrigger )
                tables.insert( 0, "{Counting}" ); // '{}' never appear on sql table name
            tables.insert( 0, "" ); // empty on top of combobox

            form->setTableList( tables );
            
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
            auto query = connection->sqlQuery( "SELECT * FROM sqlite_master WHERE type='table'" );
            table_->setQuery( query, connection->shared_from_this() );
        }
            
        {
            QSqlQuery query( connection->sqlDatabase() );
            //query.prepare( "SELECT acclVoltage,tDelay,clsidSpectrometer FROM ScanLaw WHERE spectrometer='InfiTOF' LIMIT 1" );
            query.prepare( "SELECT acclVoltage,tDelay,fLength,clsidSpectrometer FROM ScanLaw,Spectrometer WHERE id=clsidSpectrometer LIMIT 1" );
            if ( query.exec() ) {
                while ( query.next() ) {
                    auto rec = query.record();
                    double acclVoltage = rec.value( 0 ).toDouble();
                    double tDelay = rec.value( 1 ).toDouble();
                    double fLength = rec.value( 2 ).toDouble();
                    auto uuid = boost::uuids::string_generator()( rec.value( 3 ).toString().toStdString() );
                    if ( auto spectrometer = adcontrols::MassSpectrometerBroker::make_massspectrometer( uuid ) ) {
                        spectrometer->setScanLaw( acclVoltage, tDelay, fLength );
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
    typedef adplot::ChartView ChartView_t;
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
    (void)q;
    
    qtwrapper::waitCursor wait;

    auto model = qobject_cast< QStandardItemModel *>( dlg_->model() );
    dlg_->setCommandText( q );
    dlg_->activateWindow();
    dlg_->show();
    dlg_->raise();
    
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
    QString postfix = QString("%1-%2").arg( rc.left() ).arg( rc.right() );
        
    int row = model->rowCount();
    model->setRowCount( row + t_ranges.size() );
    for ( auto& t: t_ranges ) {
        model->setData( model->index( row, 0 ), QString("pk%1@%2").arg( row ).arg( postfix ), Qt::EditRole );
        model->setData( model->index( row, 1 ), t.first );
        model->setData( model->index( row, 2 ), t.second.first );
        model->setData( model->index( row, 3 ), t.second.second );
    }
    dlg_->tableView()->resizeColumnsToContents();
}

void
QueryWidget::accept()
{
    std::ostringstream stmt;

    auto model = dlg_->model();
    std::vector< std::tuple< QString, int, double, double > > ranges;

    if ( model->rowCount() == 0 )
        return;

    for ( int row = 0; row < model->rowCount(); ++row ) {
        ranges.emplace_back( std::make_tuple( model->index( row, 0 ).data( Qt::EditRole ).toString()
                                              , model->index( row, 1 ).data( Qt::EditRole ).toInt()
                                              , model->index( row, 2 ).data( Qt::EditRole ).toDouble()
                                              , model->index( row, 3 ).data( Qt::EditRole ).toDouble() ) );
    }
    
    if ( dlg_->commandText() == "COUNTING" ) {
        size_t idx( 0 );
        for ( auto& t: ranges ) {
            stmt << boost::format( "SELECT '%s' as 'name',avg(peak_time),COUNT(*) FROM peak,trigger "
                                   "WHERE id=idTrigger AND protocol=%d AND peak_time>%.8e AND peak_time < %.8e AND peak_intensity < -10" )
                % std::get< 0 >( t ).toStdString() // label
                % std::get< 1 >( t )  // protocol
                % std::get< 2 >( t )  // peak_time left
                % std::get< 3 >( t ); // peak_time right
            if ( ++idx < ranges.size() )
                stmt << "\r\n\tUNION ALL";
            stmt << "\r\n";
        }

    } else if ( dlg_->commandText() == "COUNTING.FREQUENCY" ) {
        
        stmt << "SELECT name,ROUND(peak_intensity/10)*10 AS threshold,avg(peak_time), avg(peak_intensity), avg(peak_width), COUNT(*) FROM (\r\n";
        size_t idx( 0 );
        for ( auto& t: ranges ) {
            stmt << boost::format( "\tSELECT '%s' AS name, peak_time,peak_intensity, (back_offset-front_offset) as peak_width FROM peak,trigger "
                                   "WHERE id=idTrigger AND protocol=%d AND peak_time>%.8e AND peak_time < %.8e " )
                % std::get< 0 >( t ).toStdString()
                % std::get< 1 >( t )  // protocol
                % std::get< 2 >( t )  // peak_time left
                % std::get< 3 >( t ); // peak_time right

            if ( ++idx < ranges.size() )
                stmt << "\r\n\tUNION ALL";
            stmt << "\r\n";
        }
        stmt <<") WHERE peak_intensity < threshold GROUP by name,threshold";

    } else if ( dlg_->commandText() == "ELAPSEDTIME.INTENSITY" ) {
        const auto& t = ranges[ 0 ]; // take 1st one
        stmt << boost::format( "SELECT elapsedTime - (select min(elapsedTime) from trigger ) as 'elapsed time', peak_time, peak_intensity FROM peak,trigger "
                               "WHERE id=idTrigger AND protocol=%d AND peak_time>%.8e AND peak_time < %.8e AND peak_intensity < -10 ORDER by elapsedTime" )
            % std::get< 1 >( t ) % std::get< 2 >( t ) % std::get< 3 >( t );
    }
    form_->setSQL( QString::fromStdString( stmt.str() ) );
}

void
QueryWidget::applyQuery()
{
    accept();
    handleQuery( form_->sql() );
}

void
QueryWidget::showHistory()
{
    hdlg_->activateWindow();
    hdlg_->show();
    hdlg_->raise();
}

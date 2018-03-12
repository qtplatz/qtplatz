/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "queryresulttable.hpp"
#include "queryconnection.hpp"
#include <acqrscontrols/constants.hpp>
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/scanlaw.hpp>
#include <adacquire/signalobserver.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <qtwrapper/font.hpp>
#include <QHeaderView>
#include <QMenu>
#include <QStyledItemDelegate>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QVariant>
#include <QPainter>
#include <QDebug>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/lexical_cast.hpp>

namespace query {

    namespace so = adacquire::SignalObserver;
    using namespace acqrscontrols;

    static std::map< boost::uuids::uuid, std::string > __uuid_db__ = {
        { boost::uuids::name_generator( so::Observer::base_uuid() )( u5303a::waveform_observer_name ),        u5303a::waveform_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( u5303a::timecount_observer_name ),     u5303a::timecount_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( u5303a::histogram_observer_name ),     u5303a::histogram_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( u5303a::softavgr_observer_name ),      u5303a::softavgr_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( u5303a::tdcdoc_traces_observer_name ), u5303a::tdcdoc_traces_observer_name }
    };

}


namespace query {

    class SqlQueryModel : public QSqlQueryModel {
    public:
        SqlQueryModel( QObject * parent = nullptr ) : QSqlQueryModel( parent )
                                                    , computed_mass_column_( -1 ) {
        }
        QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override {
            if ( computed_mass_column_ == index.column() && ( role == Qt::EditRole || role == Qt::DisplayRole ) ) {
                if ( index.isValid() ) {
                    // don't call record( row ), it will recurse data() and stack overflow; use query().record() instead
                    if ( spectrometer_ ) {
                        auto rec = query().record();
                        int proto = rec.value( "protocol" ).toInt();
                        double t = rec.value( "time" ).toDouble();
                        return spectrometer_->scanLaw()->getMass( t, spectrometer_->mode( proto ) );
                    }
                }
            }
            return QSqlQueryModel::data( index, role );
        }
        int computed_mass_column_;
        std::shared_ptr< adcontrols::MassSpectrometer > spectrometer_;
    };
    
    namespace queryresulttable {
        
        class ItemDelegate : public QStyledItemDelegate {
        public:
            void paint( QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                if ( auto queryModel = qobject_cast<const QSqlQueryModel *>( index.model() ) ) {
                    if ( bool isAcquiredData = queryModel->query().lastQuery().contains( "AcquiredData", Qt::CaseInsensitive ) ) {
                        auto column_name = queryModel->headerData( index.column(), Qt::Horizontal ).toString();
                        if ( column_name == "elapsed_time" ) {
                            auto t0 = queryModel->record(0).value( column_name ).toDouble() * 1.0e-9;
                            double raw = index.data().toDouble() * 1.0e-9;
                            painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter
                                               , QString("%1 [%2s]").arg( QString::number( raw, 'f', 5 ), QString::number( raw - t0, 'f', 5 ) ) );
                        } else if ( index.data().type() == QVariant::ByteArray ) {
                            auto v = queryModel->record(index.row()).value( "objuuid" );
                            if ( v.isValid() ) {
                                auto uuid = boost::uuids::string_generator()( v.toString().toStdString() );
                                auto it = __uuid_db__.find( uuid );
                                if ( it != __uuid_db__.end() )
                                    painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString::fromStdString( it->second ) );
                            }
                        } else if ( column_name == "meta" && index.data().type() == QVariant::ByteArray ) {
                            
                        } else {
                            QStyledItemDelegate::paint( painter, op, index );
                        }
                    } else {
                        QStyledItemDelegate::paint( painter, op, index );
                    }
                } else
                    QStyledItemDelegate::paint( painter, op, index );
            }

            void setEditorData( QWidget * editor, const QModelIndex& index ) const override {
                QStyledItemDelegate::setEditorData( editor, index );
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const override {
                QStyledItemDelegate::setModelData( editor, model, index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                if ( index.data().isNull() ) {
                    return QSize();
                } else if ( index.data().type() == QVariant::Double ) {
                    auto fm = op.fontMetrics;
                    double width = fm.boundingRect( op.rect, Qt::AlignJustify | Qt::AlignVCenter, QString::number( index.data().toDouble(), 'f', 5 ) ).width();
                    QSize sz = QStyledItemDelegate::sizeHint( option, index );
                    sz.setWidth( width );
                    return sz;
                } else
                    return QStyledItemDelegate::sizeHint( option, index );
            }
        };
    }
}

using namespace query;

QueryResultTable::~QueryResultTable()
{
}

QueryResultTable::QueryResultTable(QWidget *parent) : adwidgets::TableView(parent)
                                                    , model_( new SqlQueryModel() )
{
    setAllowDelete( false );
    setModel( model_.get() );
    // setItemDelegate( new queryresulttable::ItemDelegate );
    // setHorizontalHeader( new adwidgets::HtmlHeaderView );
}

void
QueryResultTable::setMassSpectrometer( std::shared_ptr< adcontrols::MassSpectrometer > sp )
{
    if ( auto model = dynamic_cast< SqlQueryModel * >( model_.get() ) )
        model->spectrometer_ = sp;
}

std::shared_ptr< adcontrols::MassSpectrometer >
QueryResultTable::massSpectrometer()
{
    if ( auto model = dynamic_cast< SqlQueryModel * >( model_.get() ) )
        return model->spectrometer_;
    return nullptr;
}

void
QueryResultTable::setDatabase( QSqlDatabase& db )
{
}

void
QueryResultTable::setQuery( const QSqlQuery& query, std::shared_ptr< QueryConnection > connection )
{
    // this is the workaround preventing segmentation violation at model_->clear();
    if ( connection_ )
        model_->setQuery( QSqlQuery() );
    connection_ = connection;
    // end workaound

    setQuery( query );
}

void
QueryResultTable::setQuery( const QSqlQuery& query )
{
    model_->clear();
    model_->setQuery( query );

    int tIndex = query.record().indexOf( "time" ); // non-case sensitive

    if ( tIndex >= 0 ) {
        model_->insertColumns( tIndex, 1 );
        model_->setHeaderData( tIndex, Qt::Horizontal, tr("m/z") );
    }

    if ( auto model = dynamic_cast< SqlQueryModel * >( model_.get() ) )
        model->computed_mass_column_ = tIndex;

    resizeColumnsToContents();
}

void
QueryResultTable::currentChanged( const QModelIndex& current, const QModelIndex& )
{
    emit onCurrentChanged( current );
}

int
QueryResultTable::findColumn( const QString& name )
{
    int nColumn = model_->columnCount();
    for ( int col = 0; col < nColumn; ++col ) {
        if ( model_->headerData( col, Qt::Horizontal, Qt::EditRole ).toString() == name )
            return col;
    }
    return -1;
}

QAbstractItemModel *
QueryResultTable::model()
{
    return model_.get();
}

void
QueryResultTable::addActionsToContextMenu( QMenu& menu, const QPoint& pt ) const
{
    adwidgets::TableView::addActionsToContextMenu( menu, pt );
    menu.addAction( tr( "Plot..." ), this, SLOT( handlePlot() ) );
}

void
QueryResultTable::handlePlot()
{
    emit plot();
}


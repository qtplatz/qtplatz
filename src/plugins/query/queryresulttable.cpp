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
#include "queryquery.hpp"
#include <acqrscontrols/constants.hpp>
#include <adicontroller/signalobserver.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <qtwrapper/font.hpp>
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

    namespace so = adicontroller::SignalObserver;

    static std::map< boost::uuids::uuid, std::string > __uuid_db__ = {
        { boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::waveform_observer_name ), acqrscontrols::u5303a::waveform_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::timecount_observer_name ), acqrscontrols::u5303a::timecount_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::histogram_observer_name ), acqrscontrols::u5303a::histogram_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::softavgr_observer_name ), acqrscontrols::u5303a::softavgr_observer_name }
        , { boost::uuids::name_generator( so::Observer::base_uuid() )( acqrscontrols::u5303a::tdcdoc_traces_observer_name ), acqrscontrols::u5303a::tdcdoc_traces_observer_name }
    };

}



namespace query {
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
                            painter->drawText( op.rect, Qt::AlignRight | Qt::AlignVCenter, QString("%1 [%2s]").arg( QString::number( raw, 'f', 5 ), QString::number( raw - t0, 'f', 5 ) ) );
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

            void setEditorData( QWidget * editor, const QModelIndex& index ) const {
                QStyledItemDelegate::setEditorData( editor, index );
            }

            void setModelData( QWidget * editor, QAbstractItemModel * model, const QModelIndex& index ) const {
                QStyledItemDelegate::setModelData( editor, model, index );
            }
            
            QSize sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const override {
                QStyleOptionViewItem op( option );
                if ( index.data().isNull() ) {
                    return QSize();
                } else if ( index.data().type() == QVariant::Double ) {
                    QFont font;
                    qtwrapper::font::setFont( font, qtwrapper::fontSizeNormal, qtwrapper::fontTableBody );
                    QFontMetricsF fm( font );
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
                                                    , model_( new QSqlQueryModel() )
{
    setAllowDelete( false );
    setModel( model_.get() );
    setItemDelegate( new queryresulttable::ItemDelegate );
    setHorizontalHeader( new adwidgets::HtmlHeaderView );
}

void
QueryResultTable::setDatabase( QSqlDatabase& db )
{
}

void
QueryResultTable::setQuery( const QSqlQuery& query )
{
    model_->setQuery( query );
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

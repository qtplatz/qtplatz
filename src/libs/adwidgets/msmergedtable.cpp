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

#include "msmergedtable.hpp"
#include "htmlheaderview.hpp"
#include <qtwrapper/font.hpp>

#include <QItemDelegate>
#include <QStandardItemModel>
#include <functional>

namespace adwidgets {
    namespace detail {

        enum {
            c_datasource
            , c_formula
            , c_mass
            , c_mass_error
            , c_delta_mass
            , c_intensity
            , c_relative_intensity
            , c_mode
            , c_time
            , c_protocol
            , c_description
            , c_index
            , c_fcn
            , c_num_columns
        };
        
        class ItemDelegate : public QItemDelegate {
        public:
            explicit ItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) {
            }
            void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
                QItemDelegate::paint( painter, option, index );
            }
            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override {
                QItemDelegate::setModelData( editor, model, index );
                // emit valueChanged(index)
            }
            std::function<void( const QModelIndex& )> valueChanged_;
        };
    }
}

using namespace adwidgets;
using namespace adwidgets::detail;

MSMergedTable::MSMergedTable( QWidget * parent ) : QTableView( parent )
                                                 , model_( new QStandardItemModel )
                                                 , inProgress_( false )
{
    setHorizontalHeader( new HtmlHeaderView );
    setModel( model_ );
    if ( auto delegate = new detail::ItemDelegate() ) {
        delegate->valueChanged_ = [=] ( const QModelIndex& idx ){ handleValueChanged( idx ); };
        setItemDelegate( delegate );
    }
    setSortingEnabled( true );
    verticalHeader()->setDefaultSectionSize( 18 );
    setContextMenuPolicy( Qt::CustomContextMenu );

    QFont font;
    this->setFont( qtwrapper::font::setFont( font, qtwrapper::fontSizeSmall, qtwrapper::fontTableBody ) );

    connect( this, &QTableView::customContextMenuRequested, this, &MSMergedTable::handleContextMenuRequested );
}

void
MSMergedTable::OnCreate( const adportable::Configuration& )
{
}

void
MSMergedTable::OnInitialUpdate()
{
    QStandardItemModel & model = *model_;
    
    model.setColumnCount( c_num_columns );

    model.setHeaderData( c_time,        Qt::Horizontal, QObject::tr( "time(&mu;s)" ) );
    model.setHeaderData( c_mass,        Qt::Horizontal, QObject::tr( "<i>m/z</i>" ) );
    model.setHeaderData( c_mass_error,  Qt::Horizontal, QObject::tr( "error(mDa)" ) );
    model.setHeaderData( c_delta_mass,  Qt::Horizontal, QObject::tr( "&delta;Da" ) );
    model.setHeaderData( c_intensity,   Qt::Horizontal, QObject::tr( "Abandance" ) );
    model.setHeaderData( c_relative_intensity,   Qt::Horizontal, QObject::tr( "R. A." ) );
    model.setHeaderData( c_mode,        Qt::Horizontal, QObject::tr( "mode" ) );
    model.setHeaderData( c_protocol,    Qt::Horizontal, QObject::tr( "protocol" ) );
    model.setHeaderData( c_formula,     Qt::Horizontal, QObject::tr( "formula" ) );
    model.setHeaderData( c_description, Qt::Horizontal, QObject::tr( "description" ) );

    setColumnHidden( c_index, true );
    setColumnHidden( c_fcn, true );  // a.k.a. protocol id, internally used as an id
}

void
MSMergedTable::onUpdate( boost::any& )
{
}

void
MSMergedTable::OnFinalClose()
{
}

bool
MSMergedTable::getContents( boost::any& ) const
{
    return false;
}

bool
MSMergedTable::setContents( boost::any& )
{
    return false;
}

void *
MSMergedTable::query_interface_workaround( const char * typnam )
{
    if ( typnam == typeid( MSMergedTable ).name() )
        return this;
    return 0;
}


// reimplement QTableView
void
MSMergedTable::currentChanged( const QModelIndex&, const QModelIndex& )
{
}

void
MSMergedTable::keyPressEvent( QKeyEvent * event )
{
}

void
MSMergedTable::valueChanged()
{
}

void
MSMergedTable::currentChanged( int idx, int fcn )
{
}

void
MSMergedTable::formulaChanged( int idx, int fcn )
{
}

void
MSMergedTable::handleCopyToClipboard()
{
}

void
MSMergedTable::handle_zoomed( const QRectF& )   // zoomer zoomed
{
}

void
MSMergedTable::handleValueChanged( const QModelIndex& )
{
}

void
MSMergedTable::handleContextMenuRequested( const QPoint& )
{
}

/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2016 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics LLC.
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

#include "timedtableview.hpp"
#include "delegatehelper.hpp"
#include "htmlheaderview.hpp"
#include <adportable/float.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adcontrols/controlmethod/modulecap.hpp>
#include <adportable/float.hpp>
#include <QApplication>
#include <QByteArray>
#include <QClipboard>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QMimeData>
#include <QPainter>
#include <QSignalBlocker>
#include <QStyledItemDelegate>
#include <QSvgRenderer>
#include <QUrl>
#include <sstream>

#include <boost/format.hpp>
#include <boost/archive/xml_woarchive.hpp>
#include <boost/archive/xml_wiarchive.hpp>
#include <qtwrapper/font.hpp>
#include <functional>

using namespace adwidgets;

namespace adwidgets {

    namespace ac = adcontrols;

    class TimedTableView::impl  {

        TimedTableView * this_;

    public:
        impl( TimedTableView * p ) : this_( p ) {
        }

        ~impl() {
        }

        std::function<void(const QPoint& )> handleContextMenu_;

        //-------------------------------------------------
        std::vector < adcontrols::ControlMethod::ModuleCap > capList_;
        
        std::map< int, ColumnState > columnStates_;

        inline const ColumnState& state( int column ) { return columnStates_[ column ]; }

        inline ColumnState::fields field( int column ) { return columnStates_[ column ].field; }

        inline int findColumn( ColumnState::fields field ) const {
             auto it = std::find_if( columnStates_.begin(), columnStates_.end()
                                     , [field]( const std::pair< int, ColumnState >& c ){ return c.second.field == field; });
             if ( it != columnStates_.end() )
                 return it->first;
             return (-1);
        }

        QAbstractItemModel * model() { return this_->model(); }
    };

}

TimedTableView::TimedTableView(QWidget *parent) : TableView(parent)
                                                , impl_( new impl( this ) )
{
    setHorizontalHeader( new HtmlHeaderView );

    setSortingEnabled( true );
    setAcceptDrops( true );

    QFont font;
    setFont( qtwrapper::font::setFamily( font, qtwrapper::fontTableBody ) );

    setContextMenuPolicy( Qt::CustomContextMenu );

    connect( this, &QTableView::customContextMenuRequested, [this]( const QPoint& pt ){
            if ( impl_->handleContextMenu_ )
                impl_->handleContextMenu_( pt );
        });
}

TimedTableView::~TimedTableView()
{
}

void
TimedTableView::onInitialUpdate()
{
    // horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
}

void
TimedTableView::setContextMenuHandler( std::function<void(const QPoint& )> f )
{
    impl_->handleContextMenu_ = f;
}

void
TimedTableView::handleContextMenu( const QPoint& pt )
{
    if ( impl_->handleContextMenu_ )
        impl_->handleContextMenu_( pt );
}

void
TimedTableView::dragEnterEvent( QDragEnterEvent * event )
{
}

void
TimedTableView::dragMoveEvent( QDragMoveEvent * event )
{
}

void
TimedTableView::dragLeaveEvent( QDragLeaveEvent * event )
{
}

void
TimedTableView::dropEvent( QDropEvent * event )
{
}

void
TimedTableView::handleCopyToClipboard()
{
}

void
TimedTableView::handlePaste()
{
}


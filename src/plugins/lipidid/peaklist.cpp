/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "document.hpp"
#include "peaklist.hpp"
#include "scoped_debug.hpp"
#include <adcontrols/massspectrum.hpp>
#include <adwidgets/htmlheaderview.hpp>
#include <adwidgets/delegatehelper.hpp>
#include <adwidgets/grid_render.hpp>
#include <adportfolio/folium.hpp>
#include <QStandardItemModel>

using lipidid::PeakList;

PeakList::PeakList( QWidget *parent ) : adwidgets::TableView( parent )
{
    setHorizontalHeader( new adwidgets::HtmlHeaderView );
    setModel( new QStandardItemModel() );
	// setItemDelegate( impl_->delegate_.get() );
    setSortingEnabled( true );
    verticalHeader()->setDefaultSectionSize( 18 );
    setContextMenuPolicy( Qt::CustomContextMenu );

    qobject_cast< QStandardItemModel * >( model() )->setColumnCount( 4 );
    int col(0);
    model()->setHeaderData( col++, Qt::Horizontal, tr( "index" ) );
    model()->setHeaderData( col++, Qt::Horizontal, tr( "<i>m/z</i>" ) );
    model()->setHeaderData( col++, Qt::Horizontal, tr( "Abundance" ) );
    model()->setHeaderData( col++, Qt::Horizontal, tr( "Color" ) );
}

void
PeakList::handleDataChanged( const portfolio::Folium& folium )
{
    ScopedDebug(__t);
    using portfolio::is_any_shared_of;
    if ( is_any_shared_of< adcontrols::MassSpectrum, const adcontrols::MassSpectrum >( folium ) ) {
        using portfolio::get_shared_of;
        if ( auto ptr = get_shared_of< const adcontrols::MassSpectrum, adcontrols::MassSpectrum >()( folium.data() ) ) {
            if ( ptr->isCentroid() ) {
                qobject_cast< QStandardItemModel * >( model() )->setRowCount( ptr->size() );
                for ( size_t i = 0; i < ptr->size(); ++i ){
                    int col(0);
                    auto [ time, mass, abundance, color ] = ptr->value( i );
                    (void)time;
                    // ADDEBUG() << ptr->value(i);
                    model()->setData( model()->index( i, col++ ), int( i ) );
                    model()->setData( model()->index( i, col++ ), mass );
                    model()->setData( model()->index( i, col++ ), abundance );
                    model()->setData( model()->index( i, col++ ), color );
                    // if ( color != 15 ) {
                    //    this->hideRow( i );
                    // }
                }
            }
        }
    }
}

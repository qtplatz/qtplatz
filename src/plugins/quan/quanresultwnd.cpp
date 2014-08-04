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

#include "quanresultwnd.hpp"
#include "quandocument.hpp"
#include "quanresultwidget.hpp"
#include "quanresulttable.hpp"
#include "quanconnection.hpp"
#include "quanquery.hpp"
#include "quancmpdwidget.hpp"
#include <adwplot/dataplot.hpp>
#include <utils/styledbar.h>
#include <coreplugin/minisplitter.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>
#include <qwt_scale_widget.h>
#include <qwt_symbol.h>
#include <qwt_scale_engine.h>
#include <QBoxLayout>
#include <QSplitter>
#include <QTextEdit>
#include <QLabel>

using namespace quan;

QuanResultWnd::QuanResultWnd(QWidget *parent) : QWidget(parent)
                                              , cmpdTable_( new QuanResultTable )
                                              , respTable_( new QuanResultWidget )
{
    Core::MiniSplitter * splitter = new Core::MiniSplitter;// compound-table | plots
    
    splitter->setOrientation( Qt::Horizontal );
    
    // compound-table on left
    if ( Core::MiniSplitter * wndSplitter = new Core::MiniSplitter ) {
        splitter->addWidget( wndSplitter ); // <<------------ add to splitter

        wndSplitter->setOrientation( Qt::Vertical );  // horizontal line
        wndSplitter->addWidget( respTable_ );

        if ( Core::MiniSplitter  * splitter2 = new Core::MiniSplitter ) { // left pane split top (table) & bottom (time,mass plot)
            splitter2->setOrientation( Qt::Horizontal );        // Plot | Text
            wndSplitter->addWidget( splitter2 );
            
            splitter2->addWidget( new adwplot::Dataplot );
            splitter2->addWidget( new QTextEdit );
        }
    }
    splitter->addWidget( new QuanCmpdWidget( cmpdTable_ ) );
    splitter->setStretchFactor( 0, 5 );
    splitter->setStretchFactor( 1, 1 );
    
    auto layout = new QVBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->addWidget( splitter );

    connect( QuanDocument::instance(), &QuanDocument::onConnectionChanged, this, &QuanResultWnd::handleConnectionChanged );

}

void
QuanResultWnd::handleConnectionChanged()
{
    if ( auto connection = QuanDocument::instance()->connection() ) {
        respTable_->setConnection ( connection );

        if ( auto query = connection->query() ) {
            if ( query->prepare( std::wstring ( L"SELECT uuid, formula, description FROM QuanCompound" ) ) ) {
                cmpdTable_->setColumnHide( "uuid" );
                cmpdTable_->prepare( *query );
                while ( query->step() == adfs::sqlite_row ) {
                    cmpdTable_->addRecord( *query );
                }
            }
        }

    }
}

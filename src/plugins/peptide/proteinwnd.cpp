/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#include "proteinwnd.hpp"
#include "proteintable.hpp"
#include "digestedpeptidetable.hpp"
#include "mainwindow.hpp"
#include <adwplot/spectrumwidget.hpp>
#include <adprot/protfile.hpp>
#include <adprot/protease.hpp>
#include <coreplugin/minisplitter.h>
#include <QVBoxLayout>
#include <QTextEdit>

using namespace peptide;

ProteinWnd::ProteinWnd(QWidget *parent) : QWidget(parent)
{
    init();
}

void
ProteinWnd::init()
{
    if ( QBoxLayout * layout = new QVBoxLayout( this ) ) {

        layout->setMargin( 0 );
        layout->setSpacing( 0 );
    
        if ( Core::MiniSplitter * splitter = new Core::MiniSplitter ) {  // protein | spectrum
            widgets_.push_back( new ProteinTable );
            splitter->addWidget( widgets_.back() );
            connect( widgets_.back(), SIGNAL( currentChanged( int ) ), this, SLOT( protSelChanged( int ) ) );

            if ( Core::MiniSplitter * splitter2 = new Core::MiniSplitter ) {
                splitter->addWidget( splitter2 );

                widgets_.push_back( new DigestedPeptideTable );
                splitter2->addWidget( widgets_.back() );

                widgets_.push_back( new adwplot::SpectrumWidget );
                splitter2->addWidget( widgets_.back() );

                splitter2->setOrientation( Qt::Vertical );
            }

            splitter->setOrientation( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
}

void
ProteinWnd::setData( const adprot::protfile& file )
{
    for ( auto w: widgets_ ) {
        if ( ProteinTable * p = dynamic_cast< ProteinTable* >( w ) ) {
            p->setData( file );
        }
    }
}

void
ProteinWnd::protSelChanged( int row )
{
    if ( std::shared_ptr< adprot::protfile > ptr = MainWindow::instance()->get_protfile() ) {
        if ( row < ptr->size() ) {
            auto it = ptr->begin() + row;

            for ( auto w: widgets_ ) {
                if ( DigestedPeptideTable * p = dynamic_cast< DigestedPeptideTable* >( w ) ) {
                    p->setData( MainWindow::instance()->get_protease() );
                    p->setData( *it );
                }
            }
        }

    }
}

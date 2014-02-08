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
            widgets_.push_back( new QTextEdit );
            for ( auto w: widgets_ )
                splitter->addWidget( w );
            splitter->setOrientation( Qt::Horizontal );
            layout->addWidget( splitter );
        }
    }
}

void
ProteinWnd::setData( const adprot::protfile& file )
{
    for ( auto w: widgets_ ) {
        if ( ProteinTable * p = dynamic_cast< ProteinTable* >( w ) )
            p->setData( file );
    }
}

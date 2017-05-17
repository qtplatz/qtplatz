/**************************************************************************
** Copyright (C) 2016 Toshinobu Hondo, Ph.D.
** Copyright (C) 2016 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "mslockdialog.hpp"
#include "mslockform.hpp"
#include "moltable.hpp"
#include <adcontrols/moltable.hpp>
#include <adcontrols/mslockmethod.hpp>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QSplitter>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <sstream>

using namespace adwidgets;

MSLockDialog::MSLockDialog( QWidget *parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {

        layout->setMargin( 0 );
        layout->setSpacing( 2 );

        if ( QSplitter * splitter = new QSplitter ) {
            splitter->addWidget( ( new MSLockForm ) ); 
            splitter->addWidget( ( new MolTable ) );
            splitter->setStretchFactor( 0, 0 );
            splitter->setStretchFactor( 1, 4 );
            splitter->setOrientation ( Qt::Horizontal );
            layout->addWidget( splitter );
        }
        if ( auto table = findChild< MolTable * >() )
            table->onInitialUpdate();

        auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        layout->addWidget( buttons );

        connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
        connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
    }

    resize( 650, size().height() );
    adjustSize();
}

void
MSLockDialog::setContents( const adcontrols::MSLockMethod& cm )
{
    if ( auto table = findChild< MolTable * >() )
        table->setContents( cm.molecules() );

    if ( auto form = findChild< MSLockForm * >() )
        form->setContents( cm );
}

bool
MSLockDialog::getContents( adcontrols::MSLockMethod& cm ) const
{
    if ( auto form = findChild< MSLockForm * >() ) {
        if ( form->getContents( cm ) ) {
            if ( auto table = findChild< MolTable * >() ) 
                table->getContents( cm.molecules() );
            return true;
        }
    }
    return false;
}


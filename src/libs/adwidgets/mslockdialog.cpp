/**************************************************************************
** Copyright (C) 2010-2016 Toshinobu Hondo, Ph.D.
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

#include "lockmassdialog.hpp"
#include "mschromatogramwidget.hpp"
#include <QBoxLayout>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QLabel>
#include <boost/uuid/uuid.hpp>
#include <stdexcept>
#include <sstream>

using namespace adwidgets;

LockMassDialog::LockMassDialog( QWidget *parent ) : QDialog( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {

        auto widget = new MSChromatogramWidget( this );
        layout->addWidget( widget );

        auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel );
        layout->addWidget( buttons );

        widget->OnInitialUpdate();
        connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
        connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
    }

    adjustSize();
}

void
LockMassDialog::setContents( const adcontrols::MSChromatogramMethod& cm )
{
    if ( auto widget = findChild< MSChromatogramWidget * >() )
        widget->setContents( cm );
}

bool
LockMassDialog::getContents( adcontrols::MSChromatogramMethod& cm ) const
{
    if ( auto widget = findChild< MSChromatogramWidget * >() )
        return widget->getContents( cm );    
    return false;
}


/**************************************************************************
** Copyright (C) 2010-2021 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2021 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "lapdeconvdlg.hpp"
#include <adcontrols/massspectrometer.hpp>
#include <adcontrols/mspeak.hpp>
#include <adcontrols/mspeaks.hpp>
#include <adportable/debug.hpp>
#include <adwidgets/moltable.hpp>
#include <qtwrapper/make_widget.hpp>
#include <QBoxLayout>
#include <QDialogButtonBox>
#include <QMenu>
#include <QMimeData>
#include <QPushButton>
#include <QSplitter>
#include <QStandardItemModel>
#include <boost/uuid/uuid.hpp>
#include <memory>

namespace dataproc {

    class lapDeconvDlg::impl {
    public:
        impl() {
        };
    };

}

using namespace dataproc;

lapDeconvDlg::lapDeconvDlg(QWidget *parent) : QDialog(parent)
                                            , impl_( std::make_unique< impl >() )
{
    if ( QVBoxLayout * layout = new QVBoxLayout( this ) ) {
        if ( auto table = qtwrapper::make_widget< adwidgets::MolTable >( "lapMolTable" ) ) {
            table->onInitialUpdate();
            layout->addWidget( table );
        }
        if ( auto buttons = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel ) ) {
            // connect( buttons, &QDialogButtonBox::accepted, this, [&] () { QDialog::accept(); } );
            // connect( buttons, &QDialogButtonBox::rejected, this, [&] () { QDialog::reject(); } );
            connect( buttons, &QDialogButtonBox::accepted, this, &QDialog::accept );
            connect( buttons, &QDialogButtonBox::rejected, this, &QDialog::reject );

            layout->addWidget( buttons );
        }
    }
    resize( 640, 340 );
}

lapDeconvDlg::~lapDeconvDlg()
{
}

//////////////

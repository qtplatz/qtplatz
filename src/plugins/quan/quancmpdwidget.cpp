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

#include "quancmpdwidget.hpp"
#include "quanresulttable.hpp"
#include <adlog/logger.hpp>
#include <utils/styledbar.h>
#include <QBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include <QMessageBox>
#include <workaround/boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception/all.hpp>

using namespace quan;

QuanCmpdWidget::~QuanCmpdWidget()
{
    delete table_;
}

QuanCmpdWidget::QuanCmpdWidget( QWidget * parent ) : QWidget( parent )
                                                   , table_( new QuanResultTable )
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        auto label = new QLabel;
        label->setText( "Compounds" );
        toolBarLayout->addWidget( label );

        topLayout->addWidget( toolBar );
    }
    topLayout->addWidget( table_ );

    table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows); // Selecting only rows
}

boost::uuids::uuid
QuanCmpdWidget::uuid( int row )
{
    int col = table_->findColumn( "uuid" );
    if ( col >= 0 ) {
        QStandardItemModel& model = table_->model();
        std::string data = model.index( row, col ).data().toString().toStdString();
        try {
            return boost::lexical_cast<boost::uuids::uuid>(data);
        }
        catch ( boost::bad_lexical_cast& ex ) {
            ADTRACE() << boost::diagnostic_information( ex );
            QMessageBox::warning( this, "QuanCmpdWidget", QString( "Can't convert to uuid from '%1'" ).arg( data.c_str() ) );
        }
    }
    return boost::uuids::uuid(); // null 
}

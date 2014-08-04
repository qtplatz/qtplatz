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

#include "quanresultwidget.hpp"
#include "quanresulttable.hpp"
#include "quandocument.hpp"
#include "quanconnection.hpp"
#include "quanquery.hpp"
#include <utils/styledbar.h>
#include <QBoxLayout>
#include <QLabel>

using namespace quan;

QuanResultWidget::QuanResultWidget(QWidget *parent) :  QWidget(parent)
{
    auto topLayout = new QVBoxLayout( this );
    topLayout->setMargin( 0 );
    topLayout->setSpacing( 0 );

    if ( auto toolBar = new Utils::StyledBar ) {
        QHBoxLayout * toolBarLayout = new QHBoxLayout( toolBar );
        toolBarLayout->setMargin( 0 );
        toolBarLayout->setSpacing( 0 );
        auto label = new QLabel;
        label->setText( "Results" );
        toolBarLayout->addWidget( label );

        topLayout->addWidget( toolBar );
    }
    if ( table_ = new QuanResultTable )
        topLayout->addWidget( table_ );
}

void
QuanResultWidget::setConnection( QuanConnection * connection )
{
    if ( auto query = connection->query() ) {

        if ( query->prepare( std::wstring( L"SELECT * from QuanResponse" ) ) ) {
            table_->prepare( *query );
            while ( query->step() == adfs::sqlite_row ) {
                table_->addRecord( *query );
            }
        }
    }
}

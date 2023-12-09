/**************************************************************************
** Copyright (C) 2014-2024 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "pubchemwnd.hpp"
#include <adportable/debug.hpp>
#include <QWidget>
#include <QTextEdit>
#include <QBoxLayout>
#include <QtWidgets/qtextedit.h>
#include <boost/system.hpp>
#include <boost/json.hpp>

using namespace chemistry;

PubChemWnd::~PubChemWnd()
{
}

PubChemWnd::PubChemWnd( QWidget * parent ) : QWidget( parent )
{
    if ( auto layout = new QVBoxLayout( this ) ) {
        layout->addWidget( new QTextEdit );
    }
}

void
PubChemWnd::handleReply( const QByteArray& ba )
{
    boost::system::error_code ec;
    auto jv = boost::json::parse( ba.toStdString(), ec );
    if ( !ec ) {
        ADDEBUG() << "\n" << jv;
    }

    if ( auto edit = findChild< QTextEdit * >() ) {
        edit->append( QString::fromStdString( ba.toStdString() ) );
    }
}

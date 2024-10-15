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

#include "csvwidget.hpp"
#include "csvtable.hpp"
#include "csvform.hpp"
#include <QtCore/qjsondocument.h>
#include <QtCore/qnamespace.h>
#include <QtGui/qtextdocument.h>
#include <adchem/drawing.hpp>
#include <adcontrols/chemicalformula.hpp>
#include <adportable/csv_reader.hpp>
#include <adportable/debug.hpp>
#include <adportable/json/extract.hpp>
#include <adportable/json_helper.hpp>
#include <adwidgets/create_widget.hpp>

#include <QBoxLayout>
#include <QImage>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSplitter>
#include <QSvgRenderer>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextTable>
#include <QTextTableCell>
#include <QWidget>

#include <boost/algorithm/string/trim.hpp>
#include <boost/json/kind.hpp>
#include <boost/system.hpp>
#include <boost/json.hpp>
#include <map>
#include <memory>

namespace figshare {

    class CSVWidget::impl {
    public:
    };
}

using namespace figshare;

CSVWidget::~CSVWidget()
{
    delete impl_;
}

CSVWidget::CSVWidget( QWidget * parent ) : QWidget( parent )
                                         , impl_( new impl{} )
{
    if ( auto topLayout = new QVBoxLayout( this ) ) {
        topLayout->setContentsMargins( {} );
        topLayout->setSpacing( 0 );

        if ( auto vsplitter = adwidgets::add_widget( topLayout, adwidgets::create_widget< QSplitter >( "ver" ) ) ) {
            vsplitter->setOrientation( Qt::Vertical );

            auto table = adwidgets::add_widget( vsplitter, adwidgets::create_widget< CSVTable >( "CSVTable" ) );
            //
            auto form = adwidgets::add_widget( vsplitter, adwidgets::create_widget< CSVForm >( "CSVForm" ) );

            vsplitter->setStretchFactor( 0, 3 );
            vsplitter->setStretchFactor( 1, 1 );
        }
    }
}

void
CSVWidget::handleReply( const QByteArray& ba, const QString& url )
{
}

void
CSVWidget::handleFigshareReply( const QByteArray& ba, const QString& url )
{
}

void
CSVWidget::handleDownloadReply( const QByteArray& ba, const QString& url )
{
}

void
CSVWidget::setData( const std::vector< adportable::csv::list_string_type >& vlist )
{
    if ( auto table = findChild< CSVTable * >() ) {
        table->setData( vlist );
    }
}

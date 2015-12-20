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

#include "docbrowser.hpp"
#include <memory>
#include "adpublisher_global.hpp"
#include <QtGlobal>
#include <QByteArray>
#include <QTextBrowser>
#include <QLayout>
#if QT_VERSION < 0x050600
#include <QWebView>
#else
#include <QWebEngineView>
#endif
#include <QUrl>

namespace adpublisher {
    
#if QT_VERSION < 0x050600
    class docBrowser::impl {
    public:
        impl() : browser( new QWebView ) {
        }
        QWebView * browser;
    };
#else
    class docBrowser::impl {
    public:
        impl() : browser( new QWebEngineView ) {
        }
        QWebEngineView * browser;
    };
#endif

}

using namespace adpublisher;
    
docBrowser::docBrowser(QWidget * parent) : QWidget( parent )
                                         , impl_( new impl )
{
    auto layout = new QHBoxLayout( this );
    layout->addWidget( impl_->browser );
}

docBrowser::~docBrowser()
{
}

void
docBrowser::setOutput( const QString& output )
{
#if 0
    impl_->browser->clear();
    impl_->browser->setText( output );
#else
    QByteArray a( reinterpret_cast<const char *>(output.data()), output.size() );
    impl_->browser->setHtml( output );
#endif
}

void
docBrowser::setOutput( const QUrl& url )
{
    impl_->browser->setUrl( url );
}
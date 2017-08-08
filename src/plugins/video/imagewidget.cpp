/**************************************************************************
** Copyright (C) 2010-2017 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2017 MS-Cheminformatics LLC, Toin, Mie Japan
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

#include "imagewidget.hpp"
#include "document.hpp"
#include <QPixmap>
#include <QLabel>
#include <QBoxLayout>
#include <QResizeEvent>

namespace video {

    class AspectRatioPixmapLabel : public QLabel {
        Q_OBJECT
    public:
        explicit AspectRatioPixmapLabel( QWidget *parent = 0 ) : QLabel( parent ) {
            this->setMinimumSize( 1, 1 );
        }

        virtual int heightForWidth( int width ) const {
            return ( (qreal)pix.height()*width ) / pix.width();
        }

        virtual QSize sizeHint() const {
            int w = this->width();
            return QSize( w, heightForWidth( w ) );
        }

    public slots:
        void setPixmap( const QPixmap & p ) {
            pix = p;
            //QLabel::setPixmap( p );
            QLabel::setPixmap( pix.scaled( this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
        }

        void resizeEvent( QResizeEvent * ) {
            QLabel::setPixmap( pix.scaled( this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation ) );
        }

    private:
        QPixmap pix;
    };
}

using namespace video;

ImageWidget::ImageWidget( QWidget * parent ) : QWidget( parent )
{
    auto layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    layout->addWidget( new AspectRatioPixmapLabel );

    // connect( document::instance(), &document::onConnectionChanged, this, &ImageWidget::handleConnectionChanged );
}

ImageWidget::~ImageWidget()
{
}

void
ImageWidget::handleConnectionChanged()
{
#if 0
    if ( auto conn = document::instance()->connection() ) {
        if ( auto ptr = document::instance()->getFile( QString::fromStdWString( conn->filepath() ) ) ) {
            QPixmap pixmap;
            if ( !ptr->waveform.empty() )
                pixmap.loadFromData( reinterpret_cast<const uchar *>( ptr->waveform.data() ), uint( ptr->waveform.size() ) );
            
            if ( auto label = findChild < AspectRatioPixmapLabel * >() )
                label->setPixmap( pixmap );
        }
    }
#endif
}

#include "imagewidget.moc"

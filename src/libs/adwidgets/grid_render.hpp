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

#pragma once

#include <QRectF>
#include <QPainter>
#include <QFontDatabase>

namespace adwidgets {

    class grid_render {
        const QRectF& rect_;
        double bottom_;
		QRectF boundingRect_;
        QRectF rc_;
        
    public:
        std::vector< std::pair< double, double > > tab_stops_;
            
        grid_render( const QRectF& rect ) : rect_( rect ), bottom_( 0 ), rc_( rect ) {
        }

        void add_tab( double width ) {
            if ( tab_stops_.empty() )
                tab_stops_.push_back( std::make_pair( rect_.left(), width ) );
            else
                tab_stops_.push_back( std::make_pair( tab_stops_.back().second + tab_stops_.back().first, width ) );
        }

        void operator()( QPainter& painter, int col, const QString& text, const QString& align = QString( "right" ) ) {

            if ( tab_stops_[col].second == 0 ) // no width
                return;

            rc_.setRect( tab_stops_[ col ].first, rc_.y(), tab_stops_[col].second, rect_.bottom() - rc_.y() );

            QTextDocument document;
            document.setTextWidth( tab_stops_[ col ].second );
            QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );
            font.setPointSize( 6 );
            document.setDefaultFont( font );
            document.setHtml( QString("<body><div align='%1'>%2</div></body>").arg( align, text ) );
            painter.save();
            painter.translate( rc_.topLeft() );
            document.drawContents( &painter );
            painter.restore();
            bottom_ = std::max<double>( rc_.topLeft().y() + document.size().height(), bottom_ );
        }

        bool new_line( QPainter& painter ) {
            rc_.moveTo( rect_.x(), bottom_ + rect_.height() * 0.005 );
            bottom_ = 0;
            if ( rc_.y() > rect_.bottom() ) {
                draw_horizontal_line( painter ); // footer separator
                return true;
            }
            return false;
        }

        void new_page( QPainter& painter ) {
            rc_.setRect( tab_stops_[ 0 ].first, rect_.top(), tab_stops_[ 0 ].second, rect_.height() );
            draw_horizontal_line( painter ); // header separator
        }

        void draw_horizontal_line( QPainter& painter ) {
            painter.drawLine( rect_.left(), rc_.top(), rect_.right(), rc_.top() );
        }
    };

}

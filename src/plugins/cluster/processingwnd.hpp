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

#pragma once

#include <adcontrols/datareader.hpp>
#include <QWidget>
#include <memory>

class QGridLayout;
class QEvent;

namespace portfolio { class Folium; }
namespace adcontrols { class MappedImage; class MappedSpectra; class MassSpectrum; }

namespace cluster {

    class ProcessingWnd : public QWidget {
        Q_OBJECT
    public:
        ~ProcessingWnd();
        explicit ProcessingWnd( QWidget *parent = 0 );

        void setHistogramWindow( double tof, double width );
        void setEnabled( int id, bool enable ); // check/uncheck map rect (0) or tof range(1)

    private:
        class impl;
        impl * impl_;

    signals:
        void nextMappedSpectra( bool );
        void tofMoved( int );
        void cellMoved( int hor, int vert );

    public slots :
        void handleProcessorChanged();
        void handleDataChanged( const portfolio::Folium& );
        void handleCheckStateChanged( const portfolio::Folium& );

        void handleAxisChanged();
        //
        void handleSelectedOnSpectrum( const QRectF& );
        void handleSelectedOnChromatogram( const QRectF& );
        void handleNextMappedSpectra( bool );
        void handleTofMoved( int );

        //
        void handleCellSelected( const QRect& );
        void handleCellMoved( int hor, int vert );

    private:
        bool eventFilter( QObject *, QEvent * );

        void setData( std::shared_ptr< const adcontrols::MappedSpectra >&&
                      , const std::pair< double, double >& trig
                      , const std::pair< double, double >& tof );

    };

}

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

#ifndef DATASEQUENCEWIDGET_HPP
#define DATASEQUENCEWIDGET_HPP

#include <QWidget>
#include <memory>

class QGridLayout;
class QStackedWidget;

namespace quan {

    class DataSequenceTree;  // Infusion (no timed peak)
    class DataSequenceTable; // Chromatography 

    class DataSequenceWidget : public QWidget
    {
        Q_OBJECT
    public:
        ~DataSequenceWidget();
        explicit DataSequenceWidget( QWidget *parent = 0 );

        //QSize sizeHint() const { return QSize( 600, 600 ); }

        void commit();

    private:
        QGridLayout * layout_;
        std::unique_ptr< QStackedWidget > stack_;
        std::unique_ptr< DataSequenceTree > dataSequenceInfusion_;
        std::unique_ptr< DataSequenceTable > dataSequenceChromatography_;
        size_t levels_;
        size_t replicates_;

        QWidget * dataSelectionBar();
        void handleDataChanged( int, bool );

    signals:
        void plot( const QString& );

    public slots :
        void handleLevelChaged( int );
        void handleReplicatesChanged( int );
        void handleSampleInletChanged( int /* adcontrols::QuanSample::QuanInlet */);
    private slots:
        void handlePlot( const QString& );
    };

}

#endif // DATASEQUENCEWIDGET_HPP

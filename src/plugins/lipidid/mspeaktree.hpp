/**************************************************************************
** Copyright (C) 2022-2022 Toshinobu Hondo, Ph.D.
** Copyright (C) 2022-2022 MS-Cheminformatics LLC, Toin, Mie Japan
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

#pragma once

#include <QTreeView>
#include <QItemDelegate>
#include <functional>
#include <tuple>
#include <memory>

class QStandardItemModel;
class QPrinter;
class QJsonDocument;
class QByteArray;

namespace adcontrols {
    class MassSpectrum;
    class ChemicalFormula;
    class Targeting;
    class MSPeaks;
    class MSPeak;
}

namespace portfolio { class Folium; }

namespace lipidid {

    class MSPeakTree : public QTreeView {
        Q_OBJECT

    public:
        virtual ~MSPeakTree();
        explicit MSPeakTree(QWidget *parent = 0);
        void onInitialUpdate();
        QStandardItemModel& model();

        void OnInitialUpdate();
        void OnFinalClose();

        typedef void( callback_t )( int event, const QVector< QPair<int, int> >& );

        virtual void addContextMenu( QMenu&, const QPoint&, std::shared_ptr< const adcontrols::MassSpectrum > ) const;

    signals:
        void currentChanged( const QModelIndex& );
        void inChIKeySelected( const QString& );

    public slots:
        void handleCopyToClipboard();
        void handleCopyAllToClipboard();
        void handleCopyCheckedToClipboard();

        void handleZoomedOnSpectrum( int view, const QRectF& );   // zoomer zoomed

        //
        void handleDataChanged( const portfolio::Folium& );
        void handleIdCompleted();

    private slots:
        void showContextMenu( const QPoint& );

    protected:
        // reimplement QTableView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void keyPressEvent( QKeyEvent * event ) override;

    private:
        class impl;
        impl * impl_;
    };

    //////////////////////
}

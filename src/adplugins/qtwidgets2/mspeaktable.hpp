/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
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

#ifndef MSPEAKTABLE_HPP
#define MSPEAKTABLE_HPP

#include <QTableView>
#include <memory>
#include <QItemDelegate>
class QStandardItemModel;

namespace adcontrols { class MSPeaks; class MSPeak; }

namespace qtwidgets2 {

    class MSPeakTable : public QTableView  {
        Q_OBJECT
    public:
        explicit MSPeakTable(QWidget *parent = 0);
        void onInitialUpdate();
        QStandardItemModel& model() { return *model_; }
        void setPeaks( const adcontrols::MSPeaks& );
    signals:

    public slots:

    private:
        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< QItemDelegate > delegate_;

        void addPeak( const adcontrols::MSPeak& );
        friend class MSPeakView;
    };

    class MSPeakTableDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit MSPeakTableDelegate(QObject *parent = 0);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		// void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        // void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        // bool editorEvent( QEvent * event, QAbstractItemModel *
        //                   , const QStyleOptionViewItem&, const QModelIndex& ) override;
    signals:
    public slots:
    };
}

#endif // MSPEAKTABLE_HPP

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

#ifndef MSPEAKSUMMARY_HPP
#define MSPEAKSUMMARY_HPP

#include <QTreeView>
#include <QItemDelegate>
#include <memory>

class QStandardItemModel;

namespace qtwidgets2 {

    class MSPeakView;

    class MSPeakSummary : public QTreeView {
        Q_OBJECT
    public:
        explicit MSPeakSummary(QWidget *parent = 0);

        void onInitialUpdate( MSPeakView * );

        void setPolynomials( int mode, const std::vector< double >&, double sd, double v );
        void setPolynomials( const std::string& formula, const std::vector< double >&, double sd, double v );
        void setResult( int id, const std::vector< double >&, double sd );

    signals:

    public slots:

    private:
        std::shared_ptr< QStandardItemModel > model_;
        std::unique_ptr< QItemDelegate > delegate_;
        MSPeakView * parent_;

        // reimplement QTreeView
        void currentChanged( const QModelIndex&, const QModelIndex& ) override;
        void keyPressEvent( QKeyEvent * event ) override;
        void handleCopyToClipboard();
    };

    class MSPeakSummaryDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit MSPeakSummaryDelegate(QObject *parent = 0);
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		// void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        // void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        // bool editorEvent( QEvent * event, QAbstractItemModel *
        //                   , const QStyleOptionViewItem&, const QModelIndex& ) override;
    signals:
    public slots:
    };

}

#endif // MSPEAKSUMMARY_HPP

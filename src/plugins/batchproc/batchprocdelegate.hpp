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

#ifndef BATCHPROCDELEGATE_HPP
#define BATCHPROCDELEGATE_HPP

#include "process.hpp"
#include <QItemDelegate>

namespace batchproc {

    class BatchprocDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit BatchprocDelegate(QObject *parent = 0);

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
		void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        bool editorEvent( QEvent * event, QAbstractItemModel *, const QStyleOptionViewItem&, const QModelIndex& ) override;
        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem& option, const QModelIndex& ) const override;

    signals:
        void stateChanged( const QModelIndex& );

    public slots:

    };

}

Q_DECLARE_METATYPE( batchproc::process );

#endif // BATCHPROCDELEGATE_HPP

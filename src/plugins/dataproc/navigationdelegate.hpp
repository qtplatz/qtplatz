// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
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

#ifndef NAVIGATIONDELEGATE_H
#define NAVIGATIONDELEGATE_H

#include <QStyledItemDelegate>
#if !defined Q_MOC_RUN
#include <adportfolio/folium.hpp>
#include <adportfolio/folder.hpp>
#endif
#if QT_VERSION >= 0x050000
# include "dataprocessor.hpp"
#endif

namespace dataproc {

    class Dataprocessor;

    class NavigationDelegate : public QStyledItemDelegate { // QItemDelegate {
        Q_OBJECT
    public:
        explicit NavigationDelegate(QObject *parent = 0);

        void paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& ) const override;
        void setEditorData( QWidget* editor, const QModelIndex& ) const override;
        void setModelData( QWidget *editor, QAbstractItemModel * model, const QModelIndex &index) const override;
        QWidget * createEditor( QWidget * parent, const QStyleOptionViewItem &option, const QModelIndex& index ) const override;

    signals:
        // void checkStateChanged( const QModelIndex&, Qt::CheckState );

    public slots:

    };

}

#endif // NAVIGATIONDELEGATE_H

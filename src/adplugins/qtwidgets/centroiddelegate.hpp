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

#pragma once

#include <QItemDelegate>

#if ! defined Q_MOC_RUN
#include <adcontrols/centroidmethod.hpp>
#endif

namespace qtwidgets {

    class CentroidDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit CentroidDelegate(QObject *parent = 0);

        QWidget *createEditor(QWidget *parent
                             , const QStyleOptionViewItem &option
                             , const QModelIndex &index) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        /////////////////
        class PeakWidthMethod {
        public:
            PeakWidthMethod( adcontrols::CentroidMethod::ePeakWidthMethod value = adcontrols::CentroidMethod::ePeakWidthTOF );
            adcontrols::CentroidMethod::ePeakWidthMethod methodValue() const;
            QString displayValue() const;
        private:
            adcontrols::CentroidMethod::ePeakWidthMethod value_;
        };
        ///////////////////
        class AreaHeight {
        public:
            AreaHeight( bool value = true );
            bool methodValue() const;
            QString displayValue() const;
        private:
            bool value_;
        };
        //////////////////
    signals:

    public slots:

    };

}

Q_DECLARE_METATYPE( qtwidgets::CentroidDelegate::PeakWidthMethod )
Q_DECLARE_METATYPE( qtwidgets::CentroidDelegate::AreaHeight )



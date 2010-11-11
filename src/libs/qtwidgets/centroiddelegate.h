// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <QItemDelegate>
#include <adcontrols/centroidmethod.h>

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

    Q_DECLARE_METATYPE( CentroidDelegate::PeakWidthMethod )
    Q_DECLARE_METATYPE( CentroidDelegate::AreaHeight )
}



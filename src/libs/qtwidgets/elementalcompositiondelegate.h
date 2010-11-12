// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#pragma once

#include <QItemDelegate>

namespace qtwidgets {

    class ElementalCompositionDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit ElementalCompositionDelegate(QObject *parent = 0);
        QWidget *createEditor(QWidget *parent
                             , const QStyleOptionViewItem &option
                             , const QModelIndex &index) const;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        ///////////////////////////////////
        class ElectronMode {
        public:
            ElectronMode( unsigned int value = 0 ); // even
            QString displayValue() const;
            unsigned int methodValue() const;
        private:
            unsigned int value_; // Even/Odd/both
        };
        //////////////////////////////////
        
        signals:
        
        public slots:
            
    };

    Q_DECLARE_METATYPE( ElementalCompositionDelegate::ElectronMode )
    
}


// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
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

#ifndef MSCALIBRATEDELEGATE_H
#define MSCALIBRATEDELEGATE_H

#include <QItemDelegate>
#if ! defined Q_MOC_RUN
#include <adcontrols/msreferencedefns.hpp>
#include <adcontrols/msreferences.hpp>
#include <adcontrols/msreference.hpp>
#endif

namespace qtwidgets2 {

    class MSCalibrateDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit MSCalibrateDelegate(QObject *parent = 0);

        QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
        void setEditorData(QWidget *editor, const QModelIndex &index) const;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
        void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    signals:
        void signalMSReferencesChanged( const QModelIndex& ) const;

    public slots:

    public:
        class MSReferences {
        public:
            MSReferences();
            MSReferences( const std::wstring& );

            const std::wstring& methodValue() const;
            QString displayValue() const;
            void setCurrentValue( const std::wstring& );
        private:
            std::wstring value_;
        };

        std::map< std::wstring, adcontrols::MSReferences > refs_;
        typedef std::map< std::wstring, adcontrols::MSReferences > refs_type;

    private:


    };
}

Q_DECLARE_METATYPE( qtwidgets2::MSCalibrateDelegate::MSReferences )

#endif // MSCALIBRATEDELEGATE_H

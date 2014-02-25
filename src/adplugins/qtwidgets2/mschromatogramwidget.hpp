/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
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

#ifndef MSCHROMATOGRAMWIDGET_HPP
#define MSCHROMATOGRAMWIDGET_HPP

#include <adplugin/lifecycle.hpp>
#include <memory>
#include <QTreeView>
#include <QItemDelegate>

class QStandardItemModel;

namespace adcontrols {
    class MSChromatogramMethod;
	class ProcessMethod;
}

namespace qtwidgets2 {

    class MSChromatogramDelegate;

    class MSChromatogramWidget : public QTreeView
                               , public adplugin::LifeCycle {
        Q_OBJECT
    public:
        explicit MSChromatogramWidget(QWidget *parent = 0);
        ~MSChromatogramWidget();

        // adplugin::LifeCycle
		virtual void OnCreate( const adportable::Configuration& ) override;
		virtual void OnInitialUpdate() override;
		virtual void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;
    public slots:
        void getLifeCycle( adplugin::LifeCycle *& p );
        void getContents( adcontrols::ProcessMethod& );

    signals:
            
    private slots:
        void handleValueChanged( const QModelIndex& );

    private:
        std::unique_ptr< QStandardItemModel > model_;
        std::unique_ptr< MSChromatogramDelegate > delegate_;
        std::unique_ptr< adcontrols::MSChromatogramMethod > method_;

        void setContents( const adcontrols::MSChromatogramMethod& );
        void getContents( adcontrols::MSChromatogramMethod& ) const;
    };

    class MSChromatogramDelegate : public QItemDelegate {
        Q_OBJECT
    public:
		explicit MSChromatogramDelegate( QObject *parent = 0 );

        QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        void setEditorData(QWidget *editor, const QModelIndex &index) const override;
        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
        QSize sizeHint( const QStyleOptionViewItem&, const QModelIndex& ) const override;
        bool editorEvent( QEvent * event, QAbstractItemModel *
                          , const QStyleOptionViewItem&, const QModelIndex& ) override;
    signals:
        void valueChanged( const QModelIndex& ) const;

    private:
    };

}

#endif // MSCHROMATOGRAMWIDGET_HPP

/**************************************************************************
** Copyright (C) 2010-2014 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013-2014 MS-Cheminformatics LLC, Toin, Mie Japan
*
** Contact: toshi.hondo@qtplatz.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminfomatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with
** the Software or, alternatively, in accordance with the terms contained in
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
/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#ifndef DOUBLETABWIDGET_HPP
#define DOUBLETABWIDGET_HPP

#include <QWidget>

namespace quan {

    namespace Ui {
        class DoubleTabWidget;
    }

    class DoubleTabWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit DoubleTabWidget( QWidget *parent = 0 );
        ~DoubleTabWidget();

        void setTitle(const QString &title);
        QString title() const { return title_; }
        void addTab(const QString &name, const QString &fullName, const QStringList &subTabs);
        void insertTab(int index, const QString &name, const QString &fullName, const QStringList &subTabs);
        void removeTab(int index);
        int tabCount() const;

        int currentIndex() const;
        void setCurrentIndex(int index);

        int currentSubIndex() const;
        void setCurrentIndex( int index, int subIndex );

    signals:
        void currentIndexChanged(int index, int subIndex);
        
    protected:
        void paintEvent(QPaintEvent *event);
        void mousePressEvent(QMouseEvent *event);
        bool event(QEvent *event);
        QSize minimumSizeHint() const;

    private:
        struct Tab {
            QString name;
            QString fullName;
            bool nameIsUnique;
            QStringList subTabs;
            int currentSubTab;
            QString displayName() const { return nameIsUnique ? name : fullName; }
        };

        void updateNameIsUniqueAdd(Tab *tab);
        void updateNameIsUniqueRemove(const Tab &tab);

        enum HitArea { HITNOTHING, HITOVERFLOW, HITTAB, HITSUBTAB };
        QPair<DoubleTabWidget::HitArea, int> convertPosToTab(QPoint pos);

        const QPixmap left_;
        const QPixmap mid_;
        const QPixmap right_;
        Ui::DoubleTabWidget *ui;
        QString title_;
        QList<Tab> tabs_;
        int currentIndex_;
        QVector<int> currentTabIndices_;
        int lastVisibleIndex_;
    };

}

#endif // DOUBLETABWIDGET_HPP

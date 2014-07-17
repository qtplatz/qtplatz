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

#ifndef BROADTABWIDGET_HPP
#define BROADTABWIDGET_HPP

#include <QWidget>

namespace quan {

    class BroadTabWidget : public QWidget {
        Q_OBJECT
    public:
        explicit BroadTabWidget(QWidget *parent = 0);
        ~BroadTabWidget();
        void setTitle(const QString &title);
        QString title() const { return title_; }

        void addTab(const QString &name, const QString &fullName, const QStringList &subTabs);
        void insertTab(int index, const QString &name, const QString &fullName, const QStringList &subTabs);
        void removeTab(int index);
        int tabCount() const;

        int currentIndex() const;
        void setCurrentIndex(int index);

        int currentSubIndex() const;

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
            QString displayName() const {
                return nameIsUnique ? name : fullName;
            }
        };

        void updateNameIsUniqueAdd(Tab *tab);
        void updateNameIsUniqueRemove(const Tab &tab);

        enum HitArea { HITNOTHING, HITOVERFLOW, HITTAB, HITSUBTAB };
        QPair<BroadTabWidget::HitArea, int> convertPosToTab(QPoint pos);

        const QPixmap left_;
        const QPixmap mid_;
        const QPixmap right_;
        QWidget * ui_;
        QString title_;
        QList<Tab> tabs_;
        int currentIndex_;
        QVector<int> currentTabIndices_;
        int lastVisibleIndex_;
    };
}

#endif // BROADTABWIDGET_HPP

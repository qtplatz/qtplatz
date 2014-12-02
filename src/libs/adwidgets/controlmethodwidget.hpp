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

#ifndef CONTROLMETHODWIDGET_HPP
#define CONTROLMETHODWIDGET_HPP

#include <QWidget>
#include "adwidgets_global.hpp"
#include <adplugin/lifecycle.hpp>
#include <memory>

class QTabWidget;

namespace adcontrols { 
    class ControlMethod;
    namespace controlmethod { class MethodItem; }
}

namespace adwidgets {

    class ControlMethodTable;

    class  ADWIDGETSSHARED_EXPORT ControlMethodWidget : public QWidget
                                                      , adplugin::LifeCycle {
        Q_OBJECT
    public:
        ~ControlMethodWidget();
        explicit ControlMethodWidget(QWidget *parent = 0);
		QSize sizeHint() const override;

        void addWidget( QWidget *, const QString& label );
        void addWidget( QWidget *, const QIcon&, const QString& );

        void addEditor( QWidget * );
        bool getControlMethod( adcontrols::ControlMethod& );
        void setControlMethod( const adcontrols::ControlMethod& );

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );
        // end LifeCycle
        bool getMethod( adcontrols::controlmethod::MethodItem& mi );
        bool setMethod( const adcontrols::controlmethod::MethodItem& mi );
    private:
        class impl;
        std::unique_ptr< impl > impl_;

    signals:
        void onCurrentChanged( QWidget * );
            
    public slots:
        void getLifeCycle( adplugin::LifeCycle *& p );
    private slots:
        void showContextMenu( const QPoint& pt );
    };

}

#endif // CONTROLMETHODWIDGET_HPP

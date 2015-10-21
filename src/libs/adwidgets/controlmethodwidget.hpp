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
#include <adplugin_manager/lifecycle.hpp>
#include <memory>

class QTabWidget;

namespace adcontrols { 
    namespace ControlMethod { class Method; class MethodItem; }
}

namespace adwidgets {

    class ControlMethodTable;

    /** \brief ControlMethodWidget is an editor for instrument control time event.
     * It is a complex of consisted instrument method for thier initial-condition
     * and timed events.
     *
     */

    class  ADWIDGETSSHARED_EXPORT ControlMethodWidget : public QWidget
                                                      , adplugin::LifeCycle {
        Q_OBJECT
        Q_INTERFACES( adplugin::LifeCycle )
    public:
        ~ControlMethodWidget();
        explicit ControlMethodWidget(QWidget *parent = 0);
		QSize sizeHint() const override;

        void addWidget( QWidget *, const QString& label );
        void addWidget( QWidget *, const QIcon&, const QString& );

        void addEditor( QWidget * );
        bool getControlMethod( adcontrols::ControlMethod::Method& );
        void setControlMethod( const adcontrols::ControlMethod::Method& );

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& ) override;
        void OnInitialUpdate() override;
        void OnFinalClose() override;
        bool getContents( boost::any& ) const override;
        bool setContents( boost::any& ) override;
        // end LifeCycle
        bool getMethod( adcontrols::ControlMethod::MethodItem& mi );
        bool setMethod( const adcontrols::ControlMethod::MethodItem& mi );
    private:
        class impl;
        std::unique_ptr< impl > impl_;

    signals:
        void onCurrentChanged( QWidget * );

        /** Menu items "Import Initial Condition" emit 'onImportInitialCondition' signal
        */
        void onImportInitialCondition();
            
    private slots:
        void showContextMenu( const QPoint& pt );
    };

}

#endif // CONTROLMETHODWIDGET_HPP

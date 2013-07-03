/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this file in
** accordance with the ScienceLiaison Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and ScienceLiaison.
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

#ifndef ACQUISITIONWIDGET_HPP
#define ACQUISITIONWIDGET_HPP

#include "datamediator.hpp"
#include <adplugin/lifecycle.hpp>
#include <QWidget>

namespace Ui {
class AcquisitionWidget;
}

namespace toftune {

    class AcquisitionWidget : public QWidget
			    , public dataMediator
			    , public adplugin::LifeCycle {

        Q_OBJECT

    public:
        explicit AcquisitionWidget(QWidget *parent = 0);
        ~AcquisitionWidget();

        virtual void setMethod( const TOF::ControlMethod& );
        virtual void getMethod( TOF::ControlMethod& ) const;

        // LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        void onUpdate( boost::any& );
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );
    
    private:
        Ui::AcquisitionWidget *ui;

    signals:
        void dataChanged( const dataMediator * );

    private slots:
        void onSampIntvalChanged( int );
        void onResolvingPowerChanged( int );
        void onNumAverageChanged( int );
    };

}

#endif // ACQUISITIONWIDGET_HPP

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

#ifndef ANALYZERWIDGET_HPP
#define ANALYZERWIDGET_HPP

#include "datamediator.hpp"
#include "interactor.hpp"
#include <adplugin/lifecycle.hpp>
#include <QWidget>
#include <memory>
#include <vector>

namespace Ui {   class AnalyzerWidget;    }

namespace toftune {

    class AnalyzerWidget : public QWidget
                         , public dataMediator
                         , public adplugin::LifeCycle {

        Q_OBJECT
    
    public:
        explicit AnalyzerWidget(QWidget *parent = 0);
        ~AnalyzerWidget();

        // dataMediator
        void setMethod( const TOF::ControlMethod& );
        void getMethod( TOF::ControlMethod& ) const;
        // <---
        // LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        void onUpdate( boost::any& );
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

        std::vector< Interactor_t > interactor_vec;
    
    private:
        Ui::AnalyzerWidget *ui;
    };

}

#endif // ANALYZERWIDGET_HPP

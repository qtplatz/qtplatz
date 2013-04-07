// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
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

#ifndef CENTROIDFORM_H
#define CENTROIDFORM_H

#include <QWidget>
// #include <QDeclarativeView>
#include <QStandardItemModel>
#include <adplugin/lifecycle.hpp>
#include <adportable/configuration.hpp>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class CentroidMethod;
    class datafile;
    class ProcessMethod;
}

namespace Ui {
    class CentroidForm;
}

namespace qtwidgets {

    class CentroidDelegate;

    class CentroidForm : public QWidget 
                       , public adplugin::LifeCycle {
        Q_OBJECT

    public:
        explicit CentroidForm(QWidget *parent = 0);
        virtual ~CentroidForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        bool getContents( boost::any& ) const;
        bool setContents( boost::any& );

        // QWidget
        virtual QSize sizeHint() const;
        //<----

    private:
        Ui::CentroidForm *ui;

        boost::scoped_ptr<adcontrols::CentroidMethod> pMethod_;
		// boost::scoped_ptr< CentroidDelegate > pDelegate_;
        adportable::Configuration config_;
    private:
		// void update_model();
        void update_data();

    public slots:
        void getLifeCycle( adplugin::LifeCycle *& p );
        void getContents( adcontrols::ProcessMethod& );
		virtual void update();

    signals:
         void apply( adcontrols::ProcessMethod& );
    };

}

#endif // CENTROIDFORM_H

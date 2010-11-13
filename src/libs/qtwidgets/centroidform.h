// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef CENTROIDFORM_H
#define CENTROIDFORM_H

#include <QWidget>
#include <QStandardItemModel>
#include <adplugin/lifecycle.h>
#include <adportable/configuration.h>
#include <boost/smart_ptr.hpp>

namespace adcontrols {
    class CentroidMethod;
}

namespace Ui {
    class CentroidForm;
}

class QStandardItemModel;

namespace qtwidgets {

    class CentroidDelegate;

    class CentroidForm : public QWidget
                       , public adplugin::LifeCycle {
        Q_OBJECT

    public:
        explicit CentroidForm(QWidget *parent = 0);
        ~CentroidForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();

    private:
        Ui::CentroidForm *ui;

        boost::scoped_ptr<adcontrols::CentroidMethod> pMethod_;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< CentroidDelegate > pDelegate_;
        adportable::Configuration config_;
    private:
        void update_model();
    };

}

#endif // CENTROIDFORM_H

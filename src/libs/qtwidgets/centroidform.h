// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef CENTROIDFORM_H
#define CENTROIDFORM_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <adportable/configuration.h>

namespace Ui {
    class CentroidForm;
}

namespace qtwidgets {

    class CentroidForm : public QWidget
                       , adplugin::LifeCycle {
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
        adportable::Configuration config_;
    };

}

#endif // CENTROIDFORM_H

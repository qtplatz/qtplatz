// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef LOGWIDGET_H
#define LOGWIDGET_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <adportable/configuration.h>
#include <string>

namespace Ui {
    class LogWidget;
}

namespace qtwidgets {

    class LogWidget : public QWidget
                    , public adplugin::LifeCycle {
        Q_OBJECT
        
    public:
        explicit LogWidget(QWidget *parent = 0);
        ~LogWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();

private:
        Ui::LogWidget *ui;
        adportable::Configuration config_;
    };

}

#endif // LOGWIDGET_H

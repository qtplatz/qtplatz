// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef TARGETINGFORM_H
#define TARGETINGFORM_H

#include <QWidget>

namespace Ui {
    class TargetingForm;
}

namespace qtwidgets {

    class TargetingForm : public QWidget
    {
        Q_OBJECT
        
    public:
        explicit TargetingForm(QWidget *parent = 0);
        ~TargetingForm();
        
    private:
        Ui::TargetingForm *ui;
    };

}

#endif // TARGETINGFORM_H

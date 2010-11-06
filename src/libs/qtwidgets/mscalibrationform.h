// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MSCALIBRATIONFORM_H
#define MSCALIBRATIONFORM_H

#include <QWidget>

namespace Ui {
    class MSCalibrationForm;
}

namespace qtwidgets {

    class MSCalibrationForm : public QWidget {
        Q_OBJECT

    public:
        explicit MSCalibrationForm(QWidget *parent = 0);
        ~MSCalibrationForm();

    private:
        Ui::MSCalibrationForm *ui;
    };

}

#endif // MSCALIBRATIONFORM_H

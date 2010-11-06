// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef REPORTFORM_H
#define REPORTFORM_H

#include <QWidget>

namespace Ui {
    class ReportForm;
}

namespace qtwidgets {

    class ReportForm : public QWidget {
        Q_OBJECT
        
    public:
        explicit ReportForm(QWidget *parent = 0);
        ~ReportForm();
        
    private:
        Ui::ReportForm *ui;
    };

}

#endif // REPORTFORM_H

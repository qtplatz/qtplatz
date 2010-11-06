// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef PEAKIDTABLEFORM_H
#define PEAKIDTABLEFORM_H

#include <QWidget>

namespace Ui {
    class PeakIDTableForm;
}

namespace qtwidgets {

    class PeakIDTableForm : public QWidget {
        Q_OBJECT
        
    public:
        explicit PeakIDTableForm(QWidget *parent = 0);
        ~PeakIDTableForm();
        
    private:
        Ui::PeakIDTableForm *ui;
    };
}

#endif // PEAKIDTABLEFORM_H

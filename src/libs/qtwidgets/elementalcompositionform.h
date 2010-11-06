// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ELEMENTALCOMPOSITIONFORM_H
#define ELEMENTALCOMPOSITIONFORM_H

#include <QWidget>

namespace Ui {
    class ElementalCompositionForm;
}

namespace qtwidgets {

    class ElementalCompositionForm : public QWidget {
        Q_OBJECT

    public:
        explicit ElementalCompositionForm(QWidget *parent = 0);
        ~ElementalCompositionForm();

    private:
        Ui::ElementalCompositionForm *ui;
    };
}

#endif // ELEMENTALCOMPOSITIONFORM_H

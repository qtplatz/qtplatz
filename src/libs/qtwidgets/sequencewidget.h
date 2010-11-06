// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef SEQUENCEWIDGET_H
#define SEQUENCEWIDGET_H

#include <QWidget>

namespace Ui {
    class SequenceWidget;
}

namespace qtwidgets {

    class SequenceModel;

    class SequenceWidget : public QWidget {
        Q_OBJECT

    public:
        explicit SequenceWidget(QWidget *parent = 0);
        ~SequenceWidget();

    private:
        Ui::SequenceWidget *ui;
        qtwidgets::SequenceModel * pModel_;
    };

}

#endif // SEQUENCEWIDGET_H

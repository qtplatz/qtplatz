/* Copyright (C) 2010 Toshinobu Hondo, Ph.D.
   Science Liaison Project
   */

#ifndef DATAANALYSISWINDOW_H
#define DATAANALYSISWINDOW_H

#include <QtGui/QWidget>

QT_FORWARD_DECLARE_CLASS(QLabel);

namespace DataAnalysis {
    namespace Internal {

    class DataAnalysisWindow : public QWidget {
    public:
        DataAnalysisWindow( QWidget * parent = 0 );
    };
}
}

#endif // DATAANALYSISWINDOW_H

// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MSCALIBRATEDELEGATE_H
#define MSCALIBRATEDELEGATE_H

#include <QItemDelegate>

namespace qtwidgets {

    class MSCalibrateDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit MSCalibrateDelegate(QObject *parent = 0);

    signals:

    public slots:

    };
}

#endif // MSCALIBRATEDELEGATE_H

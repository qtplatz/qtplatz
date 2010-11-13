// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MSLOCKDELEGATE_H
#define MSLOCKDELEGATE_H

#include <QItemDelegate>

namespace qtwidgets {

    class MSLockDelegate : public QItemDelegate {
        Q_OBJECT
    public:
        explicit MSLockDelegate(QObject *parent = 0);

    signals:

    public slots:

    };

}

#endif // MSLOCKDELEGATE_H

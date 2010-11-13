// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef TARGETINGDELEGATE_H
#define TARGETINGDELEGATE_H

#include <QItemDelegate>

namespace qtwidgets {

    class TargetingDelegate : public QItemDelegate    {
        Q_OBJECT
    public:
        explicit TargetingDelegate(QObject *parent = 0);

    signals:

    public slots:

    };

}

#endif // TARGETINGDELEGATE_H

// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef NAVIGATIONWIDGETFACTORY_H
#define NAVIGATIONWIDGETFACTORY_H

#include <coreplugin/inavigationwidgetfactory.h>

namespace dataproc {

    class NavigationWidgetFactory : public Core::INavigationWidgetFactory {
        Q_OBJECT
    public:
        explicit NavigationWidgetFactory();
        virtual ~NavigationWidgetFactory();

        virtual QString displayName();
        virtual QKeySequence activationSequence();
        virtual Core::NavigationView createWidget();

    signals:

    public slots:

    };

}

#endif // NAVIGATIONWIDGETFACTORY_H

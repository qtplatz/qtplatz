// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef SEQUENCEWIDGET_H
#define SEQUENCEWIDGET_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace Ui {
    class SequenceWidget;
}

namespace qtwidgets {

    class SequenceModel;

    class SequenceWidget : public QWidget
                                  , public adplugin::LifeCycle {
        Q_OBJECT

    public:
        explicit SequenceWidget(QWidget *parent = 0);
        ~SequenceWidget();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--

    private:
        Ui::SequenceWidget *ui;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
    };

}

#endif // SEQUENCEWIDGET_H

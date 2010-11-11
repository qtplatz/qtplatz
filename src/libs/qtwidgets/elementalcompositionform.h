// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef ELEMENTALCOMPOSITIONFORM_H
#define ELEMENTALCOMPOSITIONFORM_H
              
#include <QWidget>
#include <adplugin/lifecycle.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;
namespace adportable {
    class Configuration;
}

namespace Ui {
    class ElementalCompositionForm;
}

namespace qtwidgets {

    class ElementalCompositionDelegate;

    class ElementalCompositionForm : public QWidget
                                   , public adplugin::LifeCycle { 
        Q_OBJECT

    public:
        explicit ElementalCompositionForm(QWidget *parent = 0);
        ~ElementalCompositionForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        //<--


    private:
        Ui::ElementalCompositionForm *ui;

        boost::scoped_ptr< QStandardItemModel > pModel_;
        boost::scoped_ptr< adportable::Configuration > pConfig_;
        boost::scoped_ptr< ElementalCompositionDelegate > pDelegate_;
    };
}

#endif // ELEMENTALCOMPOSITIONFORM_H

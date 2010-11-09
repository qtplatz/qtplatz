// This is a -*- C++ -*- header.
//////////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison Project
//////////////////////////////////////////////

#ifndef SEQUENCES_H
#define SEQUENCES_H

#include <QWidget>
#include <adplugin/lifecycle.h>
#include <adportable/configuration.h>
#include <boost/smart_ptr.hpp>

class QStandardItemModel;

namespace Ui {
    class SequencesForm;
}

namespace qtwidgets {

    class SequencesModel;

    class SequencesForm : public QWidget
                        , public adplugin::LifeCycle {
        Q_OBJECT
        
    public:
        explicit SequencesForm(QWidget *parent = 0);
        ~SequencesForm();

        // adplugin::LifeCycle
        void OnCreate( const adportable::Configuration& );
        void OnInitialUpdate();
        void OnFinalClose();
        
    private:
        Ui::SequencesForm *ui;
        // qtwidgets::SequencesModel * pModel_;
        boost::scoped_ptr< QStandardItemModel > pModel_;
        adportable::Configuration config_;
        
    };
}

#endif // SEQUENCES_H

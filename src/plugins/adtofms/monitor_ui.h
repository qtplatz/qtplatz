// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MONITOR_UI_H
#define MONITOR_UI_H

#include <adplugin/imonitor.h>
#include <adportable/configuration.h>

namespace Ui {
    class Form;
}

namespace adtofms {

    namespace impl{
        class TOF;
    }
    
    class monitor_ui : public adplugin::ui::IMonitor	{
        Q_OBJECT
        Q_INTERFACES( adplugin::ui::IMonitor )
    public:
        explicit monitor_ui(QWidget *parent = 0);
        ~monitor_ui();
        
        // implement IMonitor
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
        virtual void OnUpdate( boost::any& );
        virtual void OnUpdate( unsigned long );
        virtual void OnFinalClose();
        
    signals:
        void signal_pushButton_clicked();
        
    public slots:
        void handle_clicked();
        void on_pushButton_clicked();
    private:
        friend impl::TOF;
        adportable::Configuration config_;
        impl::TOF * pTof_;
        Ui::Form * ui;
    };
    
}

#endif // MONITOR_UI_H

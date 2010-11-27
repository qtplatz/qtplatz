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

class TreeModel;

namespace TOFInstrument {
	struct AnalyzerDeviceData;
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

        // monitor_ui
		void update_analyzer_device_data();
		void display_analyzer_device_data( const TOFInstrument::AnalyzerDeviceData& );

    signals:
        void signal_pushButton_clicked();
        void signal_log( QString, QString );
        void signal_message( unsigned long, unsigned long );        
    public slots:
        void handle_clicked();
        void on_pushButton_clicked();
    private slots:
        void handle_log( QString, QString );
        void handle_message( unsigned long, unsigned long );
    private:
        friend impl::TOF;
        adportable::Configuration config_;
        impl::TOF * pTof_;
        Ui::Form * ui;
        TreeModel * treeModel_;
    };
    
}

#endif // MONITOR_UI_H

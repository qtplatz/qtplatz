#ifndef DEBUG_UI_H
#define DEBUG_UI_H

#include <adplugin/imonitor.h>
#include <adportable/configuration.h>
// #include <QWidget>

namespace Ui {
    class debug_ui;
}

namespace adtofms {

    namespace impl{
        class TOF2;
    }
    
    class debug_ui : public adplugin::ui::IMonitor	{
        Q_OBJECT
        Q_INTERFACES( adplugin::ui::IMonitor )
    public:
        explicit debug_ui(QWidget *parent = 0);
        ~debug_ui();
        
        // implement IMonitor
        virtual void OnCreate( const adportable::Configuration& );
        virtual void OnInitialUpdate();
        virtual void OnUpdate( boost::any& );
        virtual void OnUpdate( unsigned long );
        virtual void OnFinalClose();
        
    signals:
        void signal_log_out( const QString text );
        void signal_pushButton_clicked();
        
    public slots:
        void slot_log_out( const QString text );
        void handle_log_out( const QString text );
        //void handle_clicked();
        //void on_pushButton_clicked();
    private:
        friend impl::TOF2;
        adportable::Configuration config_;
        impl::TOF2 * pTof_;
        Ui::debug_ui *ui;
    };


}

#endif // DEBUG_UI_H

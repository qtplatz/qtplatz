#ifndef MAINCONTROLLERWINDOW_H
#define MAINCONTROLLERWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainControllerWindow;
}

#include <boost/smart_ptr.hpp>
#include <adportable/protocollifecycle.h>

namespace acewrapper {
    template<class T> class EventHandler;
    class DgramHandler;
    class McastHandler;
    class TimerHandler;
    template<class T> class DgramReceiver;
    template<class T> class McastReceiver;
    template<class T> class TimerReceiver;
}

class MainDeviceWindow;
class ACE_Reactor;
class ACE_Time_Value;
class ACE_INET_Addr;
class ACE_Message_Block;

class QEventReceiver;

/////////////////////

class MainControllerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainControllerWindow(QWidget *parent = 0);
    ~MainControllerWindow();

    void mcast_init();

private:
    Ui::MainControllerWindow *ui;
    adportable::protocol::LifeCycle lifeCycle_;

    boost::shared_ptr< acewrapper::EventHandler< acewrapper::DgramReceiver<QEventReceiver> > > dgramHandler_;
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> > > mcastHandler_;
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> > > timerHandler_;

    std::string ident_;
    unsigned long timerId_;
    void register_device( const ACE_INET_Addr& );

private slots:
    void on_MainControllerWindow_destroyed();
    void on_connectButton_clicked();

    void on_notify_mcast( ACE_Message_Block * mb );
    void on_notify_dgram( ACE_Message_Block * mb );
    void on_notify_timeout( unsigned long, long );
};

#endif // MAINCONTROLLERWINDOW_H

#ifndef MAINCONTROLLERWINDOW_H
#define MAINCONTROLLERWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainControllerWindow;
}

#include <boost/smart_ptr.hpp>
#include <adportable/protocollifecycle.h>
#include <map>

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
class TreeModel;

class DeviceProxy;

/////////////////////

class MainControllerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainControllerWindow(QWidget *parent = 0);
    ~MainControllerWindow();

    void mcast_init();
    void on_initial_update();
protected:
    void closeEvent(QCloseEvent *);

private:
    Ui::MainControllerWindow *ui;

    // Hardware configuration & status display in tree view
    boost::shared_ptr<TreeModel> treeModel_;

    // Hardware life cycle control
    adportable::protocol::LifeCycle lifeCycle_;

    // Hardware Half Sync/Async patterns

    boost::shared_ptr< acewrapper::EventHandler< acewrapper::McastReceiver<QEventReceiver> > > mcastHandler_;
    boost::shared_ptr< acewrapper::EventHandler< acewrapper::TimerReceiver<QEventReceiver> > > timerHandler_;

    std::string ident_;
    unsigned long timerId_;
    void multicast_update_device( const ACE_INET_Addr&
        , const adportable::protocol::LifeCycleFrame&
        , const adportable::protocol::LifeCycleData& );
    typedef std::map< std::string, boost::shared_ptr< DeviceProxy > > map_type;

private:
    std::map< std::string /* inet_addr */, boost::shared_ptr< DeviceProxy > > devices_;

private slots:
    void on_actionInitialize_triggered();
    void on_MainControllerWindow_destroyed();
    void on_connectButton_clicked();

    void on_notify_mcast( ACE_Message_Block * mb );
    void on_notify_dgram( ACE_Message_Block * mb );
    void on_notify_timeout( unsigned long, long );

    void handle_dgram_to_device( std::string remote_addr, QString local_address, QString description );
};

#endif // MAINCONTROLLERWINDOW_H

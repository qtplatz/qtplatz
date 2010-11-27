// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MAINDEVICEWINDOW_H
#define MAINDEVICEWINDOW_H

#include <QMainWindow>
#include <boost/smart_ptr.hpp>
#include <adportable/protocollifecycle.h>

namespace Ui {
    class MainDeviceWindow;
}

namespace acewrapper {
    template<class T> class EventHandler;
    class TimerHandler;
}

// class MainDeviceWindow;
class ACE_Reactor;
class ACE_Time_Value;
class ACE_INET_Addr;
class ACE_Message_Block;
class ACE_InputCDR;

class QEventReceiver;

namespace device_emulator {

	class MainDeviceWindow : public QMainWindow {
		Q_OBJECT

	public:
		explicit MainDeviceWindow(QWidget *parent = 0);
		~MainDeviceWindow();
        void initialize_device_facade();
		void initial_update();

	protected:
		void closeEvent(QCloseEvent *);

	private:
		Ui::MainDeviceWindow *ui;

		unsigned long timerId_;

		private slots:
			void on_pushDisconnect_clicked();
			void on_checkBoxAnalyzer_stateChanged(int );
			void on_checkBoxIonSource_stateChanged(int );
			void on_checkBoxAverager_stateChanged(int );
			void on_dismisButton_clicked();
			void on_pushInit_clicked();
			void on_pushHello_clicked();

			// device_facade notifications
			void handle_device_attached( std::string device );
			void handle_device_detached( std::string device );
            // void handle_send_dgram( ACE_Message_Block * );
			void handle_debug( QString );
	};
}
#endif // MAINDEVICEWINDOW_H

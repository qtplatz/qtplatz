#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::setSession( Instrument::Session_ptr p )
{
    isession_ = Instrument::Session::_duplicate( p );
    if ( ! CORBA::is_nil( isession_.in() ) )
        isession_->setConfiguration( L"<xml?/>" );
}

//// POA_Receiver
void
MainWindow::message( ::Receiver::eINSTEVENT msg, CORBA::ULong value )
{
}

void
MainWindow::log( const EventLog::LogMessage& log )
{
    (void)log;
}

void
MainWindow::shutdown()
{
    // Receiver::shutdown -- do nothing
}

void
MainWindow::debug_print( CORBA::Long pri, CORBA::Long cat, const char * text )
{
}

//// <-- POA_Receiver

void MainWindow::on_pushButton_start_stop_clicked(bool checked)
{
    if ( ! CORBA::is_nil( isession_.in() ) )
        isession_->initialize();
}

void MainWindow::on_pushButton_connect_clicked(bool checked)
{
    if ( ! CORBA::is_nil( isession_.in() ) )
        isession_->connect( this->_this(), L"device_controller" );
}

void MainWindow::on_pushButton_inject_clicked(bool checked)
{
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow * MainWindow::instance_ = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    instance_ = this;
    connect( this, SIGNAL(signal_debug_print(int, int, QString)), this, SLOT(on_debug_print(int, int, QString) ) );
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void
MainWindow::setSession( Instrument::Session_ptr p )
{
    try {
        isession_ = Instrument::Session::_duplicate( p );
        if ( ! CORBA::is_nil( isession_.in() ) ) {
            isession_->setConfiguration( L"<xml?/>" );
            on_pushButton_connect_clicked( true );
        }
    } catch ( CORBA::Exception& ex ) {
        QMessageBox::critical( 0, "MainWindow::setSession", ex._info().c_str() );
    }
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
    ui->textEdit->append( text );
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

void
MainWindow::on_debug_print( int cat, int pri, QString text )
{
    ui->textEdit->append( text );
}
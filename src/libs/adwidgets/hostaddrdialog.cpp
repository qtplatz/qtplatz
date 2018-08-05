#include "hostaddrdialog.hpp"
#include <QUrl>

using namespace adwidgets;

HostAddrDialog::HostAddrDialog(QWidget *parent) : QDialog(parent)
{
    ui.setupUi(this);

    connect( ui.buttonBox, &QDialogButtonBox::accepted, this, [&](){ QDialog::accept(); } );
    connect( ui.buttonBox, &QDialogButtonBox::rejected, this, [&](){ QDialog::reject(); } );
}

HostAddrDialog::~HostAddrDialog()
{
}

void
HostAddrDialog::setHostAddr( const QString& host, const QString& port )
{
    ui.lineEditHost->setText( host );
    ui.lineEditPort->setText( port );
}

std::pair< QString, QString >
HostAddrDialog::hostAddr() const
{
    return std::make_pair( ui.lineEditHost->text(), ui.lineEditPort->text() );
}

void
HostAddrDialog::setUrl( const QString& addr )
{
    {
        QUrl url( addr );
        ui.lineEditHost->setText( url.host() );
        // ui.lineEditPort->setText( QString::number( url.port() ) );
    }
    
    std::string url = addr.toStdString();
    auto pos = url.find( "http://" );
    if ( pos != std::string::npos )
        pos += 7;
    auto colon_pos = url.find_first_of( ':', pos );
    if ( colon_pos != std::string::npos ) {
        std::string port = url.substr( colon_pos + 1 );
        ui.lineEditPort->setText( QString::fromStdString( port ) );
    }
}

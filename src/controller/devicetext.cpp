#include "devicetext.h"

DeviceText::DeviceText(QWidget *parent) :
    QPlainTextEdit(parent)
{
}

void
DeviceText::handle_debug( std::string remote_addr, QString description )
{
    this->appendPlainText( description );
}
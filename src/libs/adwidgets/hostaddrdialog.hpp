#ifndef HOSTADDRDIALOG_HPP
#define HOSTADDRDIALOG_HPP

#include <QDialog>
#include "adwidgets_global.hpp"

namespace adwidgets {

    class ADWIDGETSSHARED_EXPORT HostAddrDialog;
    
    namespace Ui {
        class HostAddrDialog;
    }

    class HostAddrDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit HostAddrDialog(QWidget *parent = 0);
        ~HostAddrDialog();

        void setUrl( const QString& );
        void setHostAddr( const QString&, const QString& );
        std::pair< QString, QString > hostAddr() const;

    private:
        Ui::HostAddrDialog * ui;
    };

}

#endif // HOSTADDRDIALOG_HPP

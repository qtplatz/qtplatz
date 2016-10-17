#ifndef PLOTDIALOG_HPP
#define PLOTDIALOG_HPP

#include <QDialog>
#include <QStringList>
#include <tuple>
#include <vector>

class QAbstractItemModel;

namespace query {

    namespace Ui {
        class PlotDialog;
    }

    class PlotDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit PlotDialog(QWidget *parent = 0);
        ~PlotDialog();

        void setModel( QAbstractItemModel * );

        QString chartType() const;

        std::vector< std::tuple< QString, int, int > > plots() const;

        bool clearExisting() const;

    private:
        Ui::PlotDialog *ui;
        QStringList list_;

        class delegate;

    private slots:
        void handleContextMenuRequested( const QPoint& );
    };

}

#endif // PLOTDIALOG_HPP

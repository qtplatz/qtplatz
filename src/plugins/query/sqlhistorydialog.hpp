#ifndef SQLHISTORYDIALOG_HPP
#define SQLHISTORYDIALOG_HPP

#include <QDialog>
#include <memory>
class QTextCursor;
class QTextCharFormat;
class QTextEdit;
class QTextFrame;
class QTextFrameFormat;

namespace query {

    namespace Ui {
        class SqlHistoryDialog;
    }

    class SqlHistoryDialog : public QDialog
    {
        Q_OBJECT

    public:
        explicit SqlHistoryDialog(QWidget *parent = 0);
        ~SqlHistoryDialog();

        void appendSql( const QString& );
        void appendSql( const QStringList& );
        QTextEdit * textEdit();

    private:
        Ui::SqlHistoryDialog *ui;
        size_t size_;
        QTextFrame * mainFrame_;
        std::unique_ptr< QTextCharFormat > plainFormat_;
        std::unique_ptr< QTextCharFormat > headingFormat_;
        std::unique_ptr< QTextCharFormat > tagFormat_;
        std::unique_ptr< QTextCharFormat > underlineFormat_;
        std::unique_ptr< QTextFrameFormat > frameFormat_;
    };

}

#endif // SQLHISTORYDIALOG_HPP

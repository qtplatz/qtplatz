#ifndef CGENFORM_HPP
#define CGENFORM_HPP

#include <QWidget>

namespace adwidgets {

    namespace Ui {
        class CGenForm;
    }

    class CGenForm : public QWidget   {
        Q_OBJECT

    public:
        explicit CGenForm(QWidget *parent = nullptr);
        ~CGenForm();
        void setMassWidth( double );
        void setTimeWidth( double );
        double massWidth() const;
        double timeWidth() const;
        void setEnableTime( bool );
        bool enableTime() const;
        void setLabel( const QString& );

        enum { ID_MASS_WIDTH, ID_TIME_WIDTH };

    signals:
        void valueChanged( int id, double value );
        void enableTimeChanged( bool );

    private slots:
        void on_doubleSpinBox_valueChanged(double arg1);
        void on_doubleSpinBox_2_valueChanged(double arg1);
        void on_radioButton_2_toggled(bool checked);

    private:
        Ui::CGenForm *ui;
    };

}

#endif // CGENFORM_HPP

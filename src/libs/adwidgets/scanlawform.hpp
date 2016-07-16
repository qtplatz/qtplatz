#ifndef SCANLAWFORM_HPP
#define SCANLAWFORM_HPP

#include <QWidget>

namespace adwidgets {
    
    namespace Ui {
        class ScanLawForm;
    }

    class ScanLawForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScanLawForm(QWidget *parent = 0);
        ~ScanLawForm();

        void setLength( double );
        void setAcceleratorVoltage( double );
        void setTDelay( double );
        void setLengthPrecision( int );
        
        double length() const;
        double acceleratorVoltage() const;
        double tDelay() const;

    signals:
        void valueChanged( int id ); // 0 = length, 1 = accl.(v), 2 = tDelay

    private:
        Ui::ScanLawForm *ui;
    };

}

#endif // SCANLAWFORM_HPP

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

        void setLength( double, bool variable = false );
        void setAcceleratorVoltage( double, bool variable = true );
        void setTDelay( double, bool variable = true );
        
        double length() const;
        double acceleratorVoltage() const;
        double tDelay() const;

    private:
        Ui::ScanLawForm *ui;
    };

}

#endif // SCANLAWFORM_HPP

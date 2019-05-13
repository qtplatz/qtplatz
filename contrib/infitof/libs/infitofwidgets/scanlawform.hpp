#ifndef SCANLAWFORM_HPP
#define SCANLAWFORM_HPP

#include <QWidget>

namespace infitofwidgets {
    
    namespace Ui {
        class ScanLawForm;
    }

    class ScanLawForm : public QWidget
    {
        Q_OBJECT

    public:
        explicit ScanLawForm(QWidget *parent = 0);
        ~ScanLawForm();

        enum { idNLaps, idTDelay, idAcclVoltage, idLinearLength, idOrbitalLength };

        void setLinearLength( double );
        void setOrbitalLength( double );
        void setLengthPrecision( int );
        void setNlaps( int );
        
        void setAcceleratorVoltage( double, bool original = false );
        void setTDelay( double, bool original = false );
        void setL1( double, bool original = false );

        int nlaps() const;
        double linearLength() const;
        double orbitalLength() const;
        double acceleratorVoltage() const;
        double tDelay() const;
        double L1() const;

    signals:
        void valueChanged( int id ); // 0 = length, 1 = accl.(v), 2 = tDelay

    private:
        Ui::ScanLawForm *ui;
    };

}

#endif // SCANLAWFORM_HPP

#ifndef CENTROIDMETHODMODEL_HPP
#define CENTROIDMETHODMODEL_HPP

//#include <QAbstractListModel>
#include <QVariant>
#include <QStandardItemModel>
#include <QDeclarativeItem>
#include <adcontrols/centroidmethod.hpp>
#include <boost/noncopyable.hpp>

class CentroidMethodModel : public QObject {
    Q_OBJECT
    Q_ENUMS( ScanType )
    Q_ENUMS( AreaHeight )
    Q_PROPERTY( ScanType scanType READ scanType WRITE scanType NOTIFY scanTypeChanged )
    Q_PROPERTY( AreaHeight areaHeight READ areaHeight WRITE areaHeight NOTIFY areaHeightChanged )
    Q_PROPERTY( double baseline_width READ baseline_width WRITE baseline_width )
    Q_PROPERTY( double peak_centroid_fraction READ peak_centroid_fraction WRITE peak_centroid_fraction )
public:
    explicit CentroidMethodModel( QObject * parent = 0 );

    enum CentroidRoles {
        name = Qt::UserRole + 1
        , value
    };

    enum ScanType {
        ScanTypeTof = adcontrols::CentroidMethod::ePeakWidthTOF
        , ScanTypeProportional = adcontrols::CentroidMethod::ePeakWidthProportional
        , ScanTypeConstant = adcontrols::CentroidMethod::ePeakWidthConstant
    };

    enum AreaHeight {
        Area, Height
    };

    //------------------------

    double baseline_width() const;
    void baseline_width(double);
    double peak_centroid_fraction() const;
    void peak_centroid_fraction(double);

    ScanType scanType() const;
    void scanType( ScanType );

    AreaHeight areaHeight() const;
    void areaHeight( AreaHeight );

    //------------
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

signals:
    void scanTypeChanged();
    void areaHeightChanged();

public slots:

private:
    adcontrols::CentroidMethod method_;
};

#endif // CENTROIDMETHODMODEL_HPP

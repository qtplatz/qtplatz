#ifndef CENTROIDMETHODMODEL_HPP
#define CENTROIDMETHODMODEL_HPP

//#include <QAbstractListModel>
#include <QVariant>
#include <QStandardItemModel>
#include "centroidmethod.hpp"
#include <boost/noncopyable.hpp>

class MethodItem : public QObject, boost::noncopyable {
    Q_OBJECT
public:
    explicit MethodItem( QObject * parent = 0 ) : QObject( parent ) { }
    MethodItem( const QString& name, const QVariant& item, QObject * parent = 0 ) : QObject( parent )
      , name_(name)
      , item_(item) {
    }

    QString name_;
    QVariant item_;
};


class CentroidMethodModel : public QObject { // public QAbstractListModel {
    Q_OBJECT
public:
    explicit CentroidMethodModel(QObject *parent = 0);

    enum CentroidRoles {
        name = Qt::UserRole + 1
        , value
    };

    class ScanTypeTof {
    public:
        double dalton() const;
        void dalton( double );
        double mz() const;
        void mz(double);
    private:
        double width_;
        double at_;
    };

    class ScanTypeProportional {
    public:
        double ppm() const;
        void ppm( double );
    private:
        double ppm_;
    };

    class ScanTypeConstant {
    public:
        double dalton() const;
        void dalton( double );
    private:
        double width_;
    };
    
    //------------------------
    class ScanType {
    public:
        ScanType() : value_( 0 ) {}
        QString display_value() const { return "Tof"; }
        void value( int e ) { value_ = e; }
    private:
        int value_;
    };

    class AreaHeight {
    public:
        AreaHeight() : value_( true ) {}
        QString display_value() const { return value_ ? "Area" : "Height"; }
        bool value() const { return value_; }
        void value( bool v ) { value_ = v; }
    private:
        bool value_;
    };
    //-----------------------

    double baseline_width() const;
    void baseline_width(double);
    double peak_centroid_fraction() const;
    void peak_centroid_fraction(double);

    Q_INVOKABLE QVariant scanType() const;
    Q_INVOKABLE void scanType( const QVariant& );

    Q_INVOKABLE QVariant areaHeight() const;
    Q_INVOKABLE void areaHeight( const QVariant& );

    Q_INVOKABLE QList<QString> getEnumPeakMethod() const;

    //------------
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;

signals:

public slots:

private:
    QString peakMethod_;
    CentroidMethod method_;
    // QList< MethodItem > items_;
};

Q_DECLARE_METATYPE( CentroidMethodModel::AreaHeight )
Q_DECLARE_METATYPE( CentroidMethodModel::ScanType )
Q_DECLARE_METATYPE( CentroidMethodModel::ScanTypeTof )
Q_DECLARE_METATYPE( CentroidMethodModel::ScanTypeConstant )
Q_DECLARE_METATYPE( CentroidMethodModel::ScanTypeProportional )

#endif // CENTROIDMETHODMODEL_HPP

#ifndef CENTROIDMETHOD_HPP
#define CENTROIDMETHOD_HPP

class CentroidMethod {
public:

    enum ePeakWidthMethod {
        ePeakWidthTOF,
        ePeakWidthProportional,
        ePeakWidthConstant
    };
    
    ~CentroidMethod(void);
    CentroidMethod(void);
    CentroidMethod(const CentroidMethod &);
    CentroidMethod & operator = ( const CentroidMethod & rhs );
    
    bool operator == ( const CentroidMethod & rhs ) const;
    bool operator != ( const CentroidMethod & rhs ) const;
    
    double baselineWidth() const;
    double rsConstInDa() const;
    double rsPropoInPpm() const;
    double rsTofInDa() const;
    double rsTofAtMz() const;
    double attenuation() const;
    double peakCentroidFraction() const;
    ePeakWidthMethod peakWidthMethod() const;
    
    bool centroidAreaIntensity() const;
    void baselineWidth(double);
    void rsConstInDa(double);
    void rsPropoInPpm(double);
    void rsTofInDa(double);
    void rsTofAtMz(double);
    void attenuation(double);
    void peakWidthMethod(ePeakWidthMethod);
    void centroidAreaIntensity(bool);
    void peakCentroidFraction(double);
    
private:
    double baselineWidth_;
    double rsConstInDa_;
    double rsPropoInPpm_;
    double rsTofInDa_;
    double rsTofAtMz_;
    double attenuation_;
    bool bCentroidAreaIntensity_;
    double peakCentroidFraction_;
    ePeakWidthMethod peakWidthMethod_;
};

#endif // CENTROIDMETHOD_HPP

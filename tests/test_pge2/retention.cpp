
#include <iostream>
#include <libs/adprocessor/peakd.hpp>

extern std::vector< std::tuple< double, double, double, double, double > > __data;

#define COMP 1 // orca based

int
main()
{
    constexpr size_t ndim = 6;

#if COMP == 1
    constexpr size_t nprod = 3;
    Eigen::Matrix<double, ndim, nprod> A;
    // std::cout << "****** POLAR x COSMO AREA x COSMO VOLUME\n";
    // A << /* EPS=2.0 default                                         EPS=1.6 */
    //     /* ARA           37.61226,            378.20,    440.86,*/  38.233142,  379.04, 440.90,
    //     /* PGE2          38.723809,           399.53,    463.06,*/  39.314939,  402.02, 463.62,
    //     /* PGD2          38.807167,           401.88,    462.21,*/  39.439376,  399.89, 461.11,
    //     /* TXB2          39.945163,           422.83,    478.26,*/  40.553666,  423.92, 478.44,
    //     /* PGF1a        39.50597,            430.53,    482.92,*/   40.115560,  429.61, 483.37,
    //     /* PGF2a        39.388558,           420.34,    471.44;*/   39.926212,  419.49, 471.95;

    std::cout << "****** mopac POLAR x; orca Cavity area, Cavity volume";
    A << /*                 EPS=1.6 */
        /* ARA          */  38.233142,  1541.5084, 2899.4642,
        /* PGE2         */  39.314939,  1607.1137, 3064.8958,
        /* PGD2         */  39.439376,  1616.8172, 3076.8643,
        /* TXB2         */  40.553666,  1706.4509, 3195.3217,
        /* PGF1a        */  40.115560,  1657.9654, 3105.6922,
        /* PGF2a        */  39.926212,  1690.8686, 3184.7243;
#else
    constexpr size_t nprod = 2;
    Eigen::Matrix<double, ndim, nprod> A;

    // POLAR x COSMO AREA
# if COMP == 2
    std::cout << "****** POLAR x COSMO AREA\n";
    A <<
        /* ARA */          37.61226,            378.20,//    440.86,
        /* PGE2 */         38.723809,           399.53,//    463.06,
        /* PGD2 */         38.807167,           401.88,//    462.21,
        /* TXB2 */         39.945163,           422.83,//    478.26,
        /* PGF1a */        39.50597,            430.53,//    482.92,
        /* PGF2a */        39.388558,           420.34;//    471.44;
# endif
# if COMP == 3
    // POLAR x COSMO VOLUME
    std::cout << "****** POLAR x COSMO VOLUME\n";
     A <<
         /* ARA */          37.61226,            /*378.20,*/    440.86,
         /* PGE2 */         38.723809,           /*399.53,*/    463.06,
         /* PGD2 */         38.807167,           /*401.88,*/    462.21,
         /* TXB2 */         39.945163,           /*422.83,*/    478.26,
         /* PGF1a */        39.50597,            /*430.53,*/    482.92,
         /* PGF2a */        39.388558,           /*420.34;*/    471.44;
#endif
# if COMP == 4
    std::cout << "****** COSMO AREA x COSMO VOLUME\n";
    A <<
        /* ARA */          /*37.61226,*/            378.20,    440.86,
        /* PGE2 */         /*38.723809,*/           399.53,    463.06,
        /* PGD2 */         /*38.807167,*/           401.88,    462.21,
        /* TXB2 */         /*39.945163,*/           422.83,    478.26,
        /* PGF1a */        /*39.50597,*/            430.53,    482.92,
        /* PGF2a */        /*39.388558,*/           420.34,    471.44;
# endif
#endif

    Eigen::Vector< double, ndim > b;
    b << 0.5426777334931926   // ARA
        , 0.846317822846256   // E2
        , 1.000260856677475   // D2
        , 1.120806529025018   // TXT2
        , 1.1661030828431818  // F1a
        , 1.2000878060001323  // F2a
        ;

    Eigen::Vector< double, nprod > v = A.colPivHouseholderQr().solve(b);
    std::cout << "x=\n" << v << std::endl;
    std::cout << "A=\n" << A << std::endl;

    std::cout << "A * v =\n" << (A * v) << "\n<-----" << std::endl;
    std::cout << "SUM{||A*b-x||} = " << (A * v - b).norm() << std::endl;
    std::cout << "||A*b-x|| = \n" << (A * v - b) << "<-----" << std::endl;

}

// formula_parser.cpp : Defines the entry point for the console application.
//

#if ARRAYFIRE
# include <arrayfire.h>
# include <af/cuda.h>
#endif
#if OPENCV
# include <opencv2/opencv.hpp>
#endif
#include <boost/format.hpp>
#include "increment.hpp"

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    float data[]  = { 0.1, 0.2, 0.3
                      , 0.4, 0.5, 0.6 };

    // mat is the row,col array
    cv::Mat mat = cv::Mat( 2, 3, CV_32FC(1), data );
    std::cout << "Source cv::Mat : " << mat << std::endl;

    // no transposed version
    {
        // array is the col,row array
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) );
        af::print( "Converted to af::array gray: ", gray );

        af::array rgb = af::gray2rgb( gray );
        //af::print( "rgb: ", rgb );

        // https://groups.google.com/forum/#!topic/arrayfire-users/34_AFiXRnKg
        af::array rgb_t = af::reorder( rgb, 2, 0, 1 ) * 255; // Converts 3rd dimention to be the first dimension
        //af::print( "rgb_t: ", rgb_t );

        auto mat2 = cv::Mat( mat.rows, mat.cols, CV_8UC(3) );
        rgb_t.as( u8 ).host( /* reinterpret_cast< void * > */ ( mat2.ptr< uchar >( 0 ) ) );

        std::cout << "Back to cv::Mat : " << mat2 << std::endl;
    }

    // transposed version
    {
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) ).T();
        af::print( "Converted to af::array (transposed) gray: ", gray );

        // rgb as af native format
        af::array rgb = af::gray2rgb( gray );
        //af::print( "rgb: ", rgb );

        const int channels = rgb.dims( 2 );
        af::array rgb_t = rgb.T();
        //af::print( "rgb_t", rgb_t );

        // https://groups.google.com/forum/#!topic/arrayfire-users/34_AFiXRnKg
        // Converts 3rd dimention to be the first dimension
        auto cv_format_rgb = af::reorder( rgb_t, 2, 0, 1 ) * 255; 
        //af::print( "cv_format_rgb", cv_format_rgb );
        
        auto mat2 = cv::Mat( mat.rows, mat.cols, CV_8UC(3) );
        cv_format_rgb.as( u8 ).host( /* reinterpret_cast< void * > */ ( mat2.ptr< uchar >( 0 ) ) );

        std::cout << "Back to cv::Mat : " << mat2 << std::endl;
    }


    {
        const float __levels [] = { 0.0, 0.2, 0.4, 0.6, 0.8, 0.97, 1.0 };
        //                          black, navy, cyan, green,yellow,red, white
        const float __colors [] = {   0.0,  0.0,  0.0,  0.0,  1.0,  1.0, 1.0      // R
                                    , 0.0,  0.0,  1.0,  1.0,  1.0,  0.0, 1.0      // G
                                    , 0.0,  0.5,  1.0,  0.0,  0.0,  0.0, 1.0 };   // B

        af::array levels = af::array( sizeof(__levels)/sizeof(__levels[0]), 1, __levels );
        af::array colors = af::array( 3, sizeof(__colors)/sizeof(__colors[0]) / 3, __colors );
        // af::print( "colors", colors );
        
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) );

        //auto max = af::max( af::max( gray ) );
        //af::print( "max of gray", max );

        increment( gray );

        af::print( "incremented", gray );
        
        // for ( int i = 0; i < gray.dims(0); ++i ) {
        //     auto a = af::scan( gray( i, af::span ), 0, AF_BINARY_MUL );
        //     af::print( (boost::format("a[%1%]") % i).str().c_str(), gray( i, af::span) );
        //     af::print( (boost::format("a[%1%]") % i).str().c_str(), a );
            //auto a = af::approx1( gray( i, af::span ), levels );
            //af::print( "apprix1", a );
            //auto a = af::constant( 0.3, 2, f32 );
                //auto intersection = af::setIntersect( gray( i, j ), levels );
                //float value;
                //a.host( &value );
                //std::cout << "gray[" << i << ", " << j << "]=" << value << std::endl;
        //}
    }
    
    
	return 0;
}


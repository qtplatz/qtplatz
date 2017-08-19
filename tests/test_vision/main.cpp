// formula_parser.cpp : Defines the entry point for the console application.
//

#if ARRAYFIRE
# include <arrayfire.h>
#endif
#if OPENCV
# include <opencv2/opencv.hpp>
#endif

int
main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

    float data[]  = { 0.1, 0.2, 0.3
                      , 0.4, 0.5, 0.6 };

    // mat is the row,col array
    cv::Mat mat = cv::Mat( 2, 3, CV_32FC(1), data );
    std::cout << mat << std::endl;

    // no transposed version
    {
        // array is the col,row array
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) );
        af::print( "gray: ", gray );

        af::array rgb = af::gray2rgb( gray );
        af::print( "rgb: ", rgb );

        // https://groups.google.com/forum/#!topic/arrayfire-users/34_AFiXRnKg
        af::array rgb_t = af::reorder( rgb, 2, 0, 1 ) * 255; // Converts 3rd dimention to be the first dimension
        af::print( "rgb_t: ", rgb_t );

        auto mat2 = cv::Mat( mat.rows, mat.cols, CV_8UC(3) );
        rgb_t.as( u8 ).host( /* reinterpret_cast< void * > */ ( mat2.ptr< uchar >( 0 ) ) );

        std::cout << mat2 << std::endl;
    }

    // transposed version
    {
        af::array gray = af::array( mat.cols, mat.rows, 1, mat.ptr< float >( 0 ) ).T();
        af::print( "gray: ", gray );

        // rgb as af native format
        af::array rgb = af::gray2rgb( gray );
        af::print( "rgb: ", rgb );

        const int channels = rgb.dims( 2 );
        af::array rgb_t = rgb.T();
        af::print( "rgb_t", rgb_t );

        // https://groups.google.com/forum/#!topic/arrayfire-users/34_AFiXRnKg
        // Converts 3rd dimention to be the first dimension
        auto cv_format_rgb = af::reorder( rgb_t, 2, 0, 1 ) * 255; 
        af::print( "cv_format_rgb", cv_format_rgb );
        
        auto mat2 = cv::Mat( mat.rows, mat.cols, CV_8UC(3) );
        cv_format_rgb.as( u8 ).host( /* reinterpret_cast< void * > */ ( mat2.ptr< uchar >( 0 ) ) );

        std::cout << mat2 << std::endl;
    }
    
    
	return 0;
}


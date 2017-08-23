// formula_parser.cpp : Defines the entry point for the console application.
//

#if ARRAYFIRE
# include <arrayfire.h>
# include <af/cuda.h>
#endif

#if OPENCV
# include <opencv2/opencv.hpp>
# include <opencv2/imgproc/imgproc.hpp>
# include <opencv2/highgui/highgui.hpp>
# include <advision/cvtypes.hpp>
#endif

#include <advision/applycolormap.hpp>
#include <advision/bgr2rgb.hpp>
#include <advision/transform.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <chrono>

namespace po = boost::program_options;

int
main( int argc, char * argv[] )
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "algo",        po::value< std::string >()->default_value( "cv" ),  "cuda access algo ['cpu'|'af'|'cv']" )
            ( "args",        po::value< std::vector< std::string > >(),  "input files (.mp4)" )
            ;

        po::positional_options_description p;
        p.add( "args",  -1 );
        po::store( po::command_line_parser( argc, argv ).options( description ).positional(p).run(), vm );
        po::notify(vm);
    }

    if ( vm.count( "help" ) ) {
        std::cout << description;
        return 0;
    }

    bool useCUDA = vm[ "algo" ].as< std::string >() != "cpu";
    advision::cuda_algo algo = advision::cuda_none;
    if ( vm[ "algo" ].as< std::string >() == "cv" )
        algo = advision::cuda_direct;
    if ( vm[ "algo" ].as< std::string >() == "af" )
        algo = advision::cuda_arrayfire;

    af::Window wnd( 1200, 300, "test" );
    wnd.grid( 1, 3 );

    for ( auto& file: vm[ "args" ].as< std::vector< std::string > >() ) {

        boost::filesystem::path path( file );
        wnd.setTitle( path.filename().string().c_str() );
        
        std::deque< cv::Mat > cv_vec;
        size_t nframes(0);

        std::unique_ptr< advision::mat_< float, 1 > > avg;

        advision::ApplyColorMap map;
        
        cv::VideoCapture capture;
        if ( capture.open( file ) && capture.isOpened() ) {

            auto tp = std::chrono::high_resolution_clock::now();
            
            cv::Mat m;
            while ( capture.read( m ) ) {

                auto a = advision::transform_<af::array>()( m );
                
                auto dur = std::chrono::high_resolution_clock::now() - tp;
                auto us = std::chrono::duration_cast< std::chrono::microseconds >( dur ).count();                

                advision::mat_< float, 1 > gs( m.rows, m.cols );
                {
                    cv::Mat_< uchar > gray;
                    cv::cvtColor( m, gray, cv::COLOR_BGR2GRAY );
                    gray.convertTo( gs, advision::mat_< float, 1 >::type_value, 1.0/255 );
                }
                cv_vec.emplace_back( gs );
                if ( !avg ) {
                    avg = std::make_unique< advision::mat_< float, 1 > >( gs );
                } else {
                    (*avg) += gs;
                }

                auto colored = advision::transform_< af::array >()( map( *avg, 8.0 / nframes, algo ) );
                
                // convert to af::array
                auto b = af::array( avg->cols, avg->rows, 1, avg->ptr< float >( 0 ) ).T();
                
                wnd(0,0).image( a, (boost::format( "Org. %g fps" ) % ( double(nframes * 1e6) / us ) ).str().c_str() );

                wnd(0,1).image( b * 8 / nframes
                                , (boost::format( "Gray Scale %1%/%2%" ) % nframes % ( double(us) / nframes ) ).str().c_str() );

                wnd(0,2).image( advision::bgr2rgb_< af::array >()( colored )
                                , (boost::format( "%1%/%2% %3%" )
                                   % ( useCUDA ? "CUDA" : "CPU" )
                                   % nframes
                                   % ( double(us) / nframes )
                                    ).str().c_str() );
                
                wnd.show();

                if ( nframes++ == 0 )
                    tp = std::chrono::high_resolution_clock::now();
                
                while ( cv_vec.size() > 2000 )
                    cv_vec.pop_front();
            }

            using namespace std::chrono;
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "Data read & display total: %g s, %g fps" ) % s % ( nframes / s ) << std::endl;
        }

        do {
            auto tp = std::chrono::high_resolution_clock::now();
            for ( auto& m: cv_vec ) {
                map( m, 1.0, advision::cuda_arrayfire );
            }
            using namespace std::chrono;        
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "GPU Total: %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
        } while ( 0 );

        do {
            auto tp = std::chrono::high_resolution_clock::now();
            for ( auto& m: cv_vec ) {
                map( m, 1.0, advision::cuda_direct );
            }
            using namespace std::chrono;        
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "CPU Total: %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
        } while ( 0 );

        do {
            auto tp = std::chrono::high_resolution_clock::now();
            for ( auto& m: cv_vec ) {
                map( m, 1.0, advision::cuda_none );
            }
            using namespace std::chrono;        
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "CPU Total: %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
        } while ( 0 );

    }

	return 0;
}


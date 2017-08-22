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
            ( "args",        po::value< std::vector< std::string > >(),  "input files" )
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

    af::Window wnd( 800, 600, "test" );
    wnd.grid( 2, 2 );

    for ( auto& file: vm[ "args" ].as< std::vector< std::string > >() ) {

        boost::filesystem::path path( file );
        wnd.setTitle( path.filename().string().c_str() );
        
        std::deque< cv::Mat > cv_vec;
        size_t nframes(0);
        std::unique_ptr< advision::mat_< float, 1 > > avg;
        
        cv::VideoCapture capture;
        if ( capture.open( file ) && capture.isOpened() ) {

            auto tp = std::chrono::high_resolution_clock::now();
            
            cv::Mat m;
            while ( capture.read( m ) ) {

                cv::imshow( m );
                cv::waitKey( 1 );
                
                auto a = advision::transform::array( m );
                wnd( 0, 1 ).image( a, "" );
                
                advision::mat_< float, 1 > gs( m.rows, m.cols );
                {
                    cv::Mat_< uchar > gray;
                    cv::cvtColor( m, gray, cv::COLOR_BGR2GRAY );
                    gray.convertTo( gs, advision::mat_< float, 1 >::type_value, 1.0/255 );
                }

                if ( !avg )
                    avg = std::make_unique< advision::mat_< float, 1 > >( gs );
                else
                    (*avg) += gs;
                ++nframes;

                cv_vec.emplace_back( gs );
                
                // convert to af::arry
                auto a = af::array( cv_vec.back().cols, cv_vec.back().rows, 1, cv_vec.back().ptr< float >( 0 ) ).T();
                auto b = af::array( avg->cols, avg->rows, 1, avg->ptr< float >( 0 ) ).T();

                auto dur = std::chrono::high_resolution_clock::now() - tp;
                auto us = std::chrono::duration_cast< std::chrono::microseconds >( dur ).count();

                wnd(0,0).image( a, (boost::format( "%g fps" ) % ( double(nframes * 1e6) / us ) ).str().c_str() );
                wnd(1,0).image( b / nframes, (boost::format( "%1%/%2%" ) % nframes % ( double(us) / nframes ) ).str().c_str() );

                wnd.show();

                while ( cv_vec.size() > 2000 )
                    cv_vec.pop_front();
            }

            using namespace std::chrono;
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "Data read & display total: %g s, %g fps" ) % s % ( nframes / s ) << std::endl;
        }

        advision::ApplyColorMap map;

        do {
            auto tp = std::chrono::high_resolution_clock::now();
            for ( auto& m: cv_vec ) {
                map( m );
            }
            using namespace std::chrono;        
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "GPU Total: %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
        } while ( 0 );

        do {
            auto tp = std::chrono::high_resolution_clock::now();
            for ( auto& m: cv_vec ) {
                map( m, 1.0, false );
            }
            using namespace std::chrono;        
            auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
            std::cout << boost::format( "CPU Total: %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
        } while ( 0 );
        
    }

	return 0;
}


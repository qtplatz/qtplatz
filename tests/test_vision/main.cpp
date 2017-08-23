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

#include <adportable/semaphore.hpp>
#include <advision/applycolormap.hpp>
#include <advision/bgr2rgb.hpp>
#include <advision/transform.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <condition_variable>
#include <chrono>
#include <mutex>
#include <map>
#include <thread>

namespace po = boost::program_options;

static std::vector< std::string > algo_name = { "CPU", "CUDA/AF", "CUDA" };

void single_thread_test( const std::string& file, af::Window * wnd, advision::cuda_algo );
void thread_pool_test( const std::string& file, af::Window * wnd, advision::cuda_algo, int );

int
main( int argc, char * argv[] )
{
    po::variables_map vm;
    po::options_description description( argv[0] );
    {
        description.add_options()
            ( "help,h",      "Display this help message" )
            ( "threads",     po::value< int >()->implicit_value( 4 ), "number of threads" )
            ( "algo",        po::value< std::string >()->default_value( "cv" ),  "cuda access algo ['cpu'|'af'|'cv']" )
            ( "show",        po::value< std::string >()->default_value( "yes" ),  "display image ['yes'|'no']" )
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

    advision::cuda_algo algo = advision::cuda_none;
    
    if ( vm[ "algo" ].as< std::string >() == "cv" )
        algo = advision::cuda_direct;
    if ( vm[ "algo" ].as< std::string >() == "af" )
        algo = advision::cuda_arrayfire;

    bool showImage = vm[ "show" ].as< std::string >() == "yes";    

    std::unique_ptr< af::Window > wnd;
    if ( showImage ) {
        wnd = std::make_unique< af::Window >( 1200, 300, "test" );
        wnd->grid( 1, 3 );
    }

    for ( auto& file: vm[ "args" ].as< std::vector< std::string > >() ) {

        if ( vm.count( "threads" ) ) {
            thread_pool_test( file, wnd.get(), algo, vm[ "threads" ].as<int>() );
        } else {
            single_thread_test( file, wnd.get(), algo );
        }
    }
        
	return 0;
}

void
single_thread_test( const std::string& file, af::Window * wnd, advision::cuda_algo algo )
{
    boost::filesystem::path path( file );
    if ( wnd )
        wnd->setTitle( path.filename().string().c_str() );

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

            if ( wnd ) {
                (*wnd)(0,0).image( a, (boost::format( "Org. %g fps" ) % ( double(nframes * 1e6) / us ) ).str().c_str() );
                    
                (*wnd)(0,1).image( b * 8 / nframes
                                , (boost::format( "Gray Scale %1%/%2%" ) % nframes % ( double(us) / nframes ) ).str().c_str() );
                    
                (*wnd)(0,2).image( advision::bgr2rgb_< af::array >()( colored )
                                , (boost::format( "%1%/%2% %3%" )
                                   % algo_name.at( algo )
                                   % nframes
                                   % ( double(us) / nframes )
                                    ).str().c_str() );
                wnd->show();
            }
                
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
        std::cout << boost::format( "GPU (arrayfire)  : %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
    } while ( 0 );

    do {
        auto tp = std::chrono::high_resolution_clock::now();
        for ( auto& m: cv_vec ) {
            map( m, 1.0, advision::cuda_direct );
        }
        using namespace std::chrono;        
        auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
        std::cout << boost::format( "GPU (cuda direct): %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
    } while ( 0 );

    do {
        auto tp = std::chrono::high_resolution_clock::now();
        for ( auto& m: cv_vec ) {
            map( m, 1.0, advision::cuda_none );
        }
        using namespace std::chrono;        
        auto s = duration_cast< duration<double> >( high_resolution_clock::now() - tp ).count();
        std::cout << boost::format( "CPU              : %g s, %g fps" ) % s % ( cv_vec.size() / s ) << std::endl;
    } while ( 0 );

}

///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class VideoStream {
    std::mutex mutex_;
    adportable::semaphore sema_;
    std::deque< cv::Mat > stream;
    bool running_;
public:
    VideoStream() : running_( true ) {}

    inline size_t count() const {  return sema_.count(); }

    void stop() {
        running_ = false;
        sema_.signal();
    }
    
    void operator << ( cv::Mat&& m ) {
        std::lock_guard< std::mutex > lock( mutex_ );
        stream.emplace_back( std::move( m ) );
        sema_.signal(); // V
    }

    bool move( cv::Mat& m ) {
        sema_.wait();
        if ( running_ ) {
            m = std::move( stream.front() );
            stream.pop_front();
        }
        return running_;
    }
};

class VideoProvider {
    std::mutex mutex_;
    cv::VideoCapture capture_;
    std::size_t count_;
    std::set< std::thread::id > id_;
public:
    VideoProvider() : count_( 0 ) {}

    inline size_t count() const { return count_; }

    bool open( const std::string& file ) {
        return capture_.open( file ) && capture_.isOpened();
    }

    // grab all data in memory for benchmark
    bool read( VideoStream& stream ) {
        cv::Mat m;
        if ( capture_.read( m ) ) {
            count_++;
            stream << std::move( m );
            return true;
        }
        return false;
    }
    
    bool read( VideoStream& stream, boost::asio::io_service& pool ) {

        if ( id_.find( std::this_thread::get_id() ) == id_.end() )
            id_.insert( std::this_thread::get_id() );
        
        pool.post( [&](){ read( stream, pool ); } );        
        std::lock_guard< std::mutex > lock( mutex_ );
        cv::Mat m;
        if ( capture_.read( m ) ) {
            count_++;
            stream << std::move( m );
            return true;
        }
        std::cout << "read\t " << id_.size() << " threads involved.\t" << count_ << " images read." << std::endl;
        stream.stop();
        return false;
    }
};

class VideoConsumer {
    size_t count_;
    std::set< std::thread::id > id_;
    std::function< void( const cv::Mat& ) > process_;
public:
    VideoConsumer() : count_( 0 ) {}

    inline size_t count() const { return count_; }

    void register_process( std::function< void( const cv::Mat& ) > f ) { process_ = f; }

    bool consume( VideoStream& stream, boost::asio::io_service& pool, bool streaming ) {

        if ( id_.find( std::this_thread::get_id() ) == id_.end() )
            id_.insert( std::this_thread::get_id() );        

        cv::Mat m;
        if ( ! stream.move( m ) ) {
            pool.stop();
            return false;
        }

        if ( ! streaming && stream.count() == 0 ) {
            pool.stop();
            return false;
        }

        ++count_;
        pool.post( [&](){ consume( stream, pool, streaming ); } );

        if ( process_ )
            process_( m );

        return true;
    }
};

void
thread_pool_test( const std::string& file, af::Window * wnd, advision::cuda_algo algo, int nThreads )
{
    VideoStream stream;
    VideoProvider reader;
    VideoConsumer consumer;

    boost::asio::io_service io_service;
    boost::asio::io_service::work work( io_service );

    size_t numAverage(0);
    cv::Mat_< float > avg, gs;

    consumer.register_process( [&]( const cv::Mat& m ){
            if ( numAverage++ == 0 ) {
                cv::Mat_< uchar > gray;
                cv::cvtColor( m, gray, cv::COLOR_BGR2GRAY );
                gray.convertTo( gs, advision::mat_< float, 1 >::type_value, 8 * 1.0/255 );
            }
            // auto a = advision::transform_<af::array>()( m );
            advision::ApplyColorMap()( gs, 1.0, algo );
        });

#if 1 // cash all data in advance
    if ( reader.open( file ) ) {
        while ( reader.read( stream ) )
            ;
        io_service.post( [&](){ consumer.consume( stream, io_service, false ); });
    }

#else
    
    if ( reader.open( file ) ) {
        io_service.post( [&](){ consumer.consume( stream, io_service, true ); });
        io_service.post( [&](){ reader.read( stream, io_service ); });
    }
#endif
    std::cout << "reader has " << stream.count() << " images." << std::endl;
    
    std::vector< std::thread > threads;
    if ( nThreads == 0 )
        nThreads = 1;
    nThreads = std::min( nThreads, 8 );

    while ( nThreads-- ) {
        threads.emplace_back( [&](){ io_service.run(); } );
    }
    std::cout << "Total: " << threads.size() << " threads to be getting involved";

    auto tp = std::chrono::high_resolution_clock::now();    

    for ( auto& t: threads )
        t.join();

    auto duration = std::chrono::high_resolution_clock::now() - tp;
    std::cout << consumer.count() << " images processed in "
              << std::chrono::duration_cast< std::chrono::duration< double > >( duration ).count() << " s\n"
              << consumer.count() / std::chrono::duration_cast< std::chrono::duration< double > >( duration ).count() << " fps"
              << std::endl;
}

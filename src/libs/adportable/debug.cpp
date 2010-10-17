//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#include "debug.h"
#include <fstream>
#include <iomanip>

using namespace adportable;

namespace adportable {
    namespace internal {

        class logfile {
            static logfile * instance_;
            std::string filename_;
            logfile() : filename_( "debug.log" ) {}
            ~logfile() {}
        public:
            static logfile * instance() {
                if ( instance_ == 0 ) {
                    instance_ = new logfile();
                    atexit( dispose );
                }
                return instance_;
            }
            static void dispose() {
                delete instance_;
            }
            const std::string& filename() const { return filename_; }
            void filename( const std::string& filename ) {
                filename_ = filename;
            }
        };
    }
}

internal::logfile * internal::logfile::instance_ = 0;


debug::debug(void)
{
}

debug::~debug(void)
{
    using namespace internal;
    std::ofstream of( logfile::instance()->filename().c_str(), std::ios_base::out | std::ios_base::app );
    //of << std::fixed << std::setprecision(3) << double(tic_) / 1000 << " " << o_.str() << std::endl;
    of << o_.str() << std::endl;
}

void
debug::initialize( const std::string& name )
{
    internal::logfile::instance()->filename( name );
}

debug&
debug::operator << ( const std::string& text )
{
    o_ << text;
    return *this;
}

debug&
debug::operator << ( int n )
{
    o_ << n;
    return *this;
}

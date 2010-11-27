#pragma once
#include <sstream>
#include <string>

class logger {
    std::ostringstream buf_;
public:
    logger(void);
    ~logger(void);
    static void print( const char * text );
    
    logger & dump( size_t, const char * p );
    logger & operator << ( const char * text );
    logger & operator << ( const wchar_t * text );
    logger & operator << ( const std::string& text );
    logger & operator << ( const std::wstring& text );
    logger & operator << ( int );
    logger & operator << ( unsigned int );
    logger & operator << ( unsigned char );
    logger & operator << ( unsigned short );
    logger & operator << ( unsigned long );
};

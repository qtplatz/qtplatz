#ifndef CALLBACK_H
#define CALLBACK_H

#include <ace/INET_Addr.h>

class Callback {
public:
    virtual void operator()(const char * pbuf, ssize_t, const ACE_INET_Addr& ) { }
};


#endif // CALLBACK_H

#ifndef ADCONTROLLER_GLOBAL_H
#define ADCONTROLLER_GLOBAL_H

#if defined(ADCONTROLLER_LIBRARY)
#  define ADCONTROLLERSHARED_EXPORT __declspec(dllexport)
#else
#  define ADCONTROLLERSHARED_EXPORT __declspec(dllimport)
#endif

#endif // ADCONTROLLER_GLOBAL_H

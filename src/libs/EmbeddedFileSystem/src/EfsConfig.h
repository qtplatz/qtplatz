//////////////////////////////////////////////////////////////////
///
/// (C) 2007: Yuriy Soroka <ysoroka@scalingweb.com>
///	      Anton Fedoruk <afedoruk@scalingweb.com>
///
//////////////////////////////////////////////////////////////////


#ifndef __EfsConfig_h__
#define __EfsConfig_h__

#include <QMutex>
#include <QMutexLocker>

#define AFS_MULTITHREADED 1
#define AFS_DEBUG 1

#if AFS_MULTITHREADED
#define AFS_MULTITHREAD_SUPPORT mutable QMutex mutex_;
#define AFS_MULTITHREAD_SUPPORT_INIT mutex_( QMutex::Recursive ),
#define AFS_LOCK_THREAD QMutexLocker lock( &mutex_ );
#else
#define AFS_MULTITHREAD_SUPPORT 
#define AFS_MULTITHREAD_SUPPORT_INIT 
#define AFS_LOCK_THREAD 
#endif // AFS_MULTITHREADED

#endif // __EfsConfig_h__

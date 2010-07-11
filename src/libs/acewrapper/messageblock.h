// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef MESSAGEBLOCK_H
#define MESSAGEBLOCK_H

#include <boost/noncopyable.hpp>
#include <ace/Message_Block.h>

namespace acewrapper {

   template<class MB> class scoped_mblock_ptr : boost::noncopyable {
	 MB *& mb_;
      public:
	 explicit scoped_mblock_ptr(MB *& mb) : mb_(mb) {}
	 ~scoped_mblock_ptr() { dispose(); }
	 operator MB * () { return mb_; }
      private:
	 inline void dispose() {
	    MB * tmp = mb_;
	    mb_ = 0;
	    if ( tmp )
	       tmp->release();
	 }
   };
   
   ///////////////////////////
   
   class MessageBlock {
      public:
	 MessageBlock();
#if 0
	 static ACE_Message_Block * create( const Msg& mh, size_t size ) {
	    ACE_Message_Block * mb = new ACE_Message_Block( sizeof(mh) + size );
	    if ( mb ) {
	       *(reinterpret_cast<Msg *>(mb->wr_ptr())) = mh;
	       mb->wr_ptr( sizeof( mh ) );
	    }
	    return mb;
	 }
	 
	 template<typename T> static ACE_Message_Block * create( const Msg& mh, const T& t ) {
	    ACE_Message_Block * mb = new ACE_Message_Block( sizeof(mh) + sizeof( T ) );
	    if ( mb ) {
	       *(reinterpret_cast<Msg *>(mb->wr_ptr())) = mh;
	       mb->wr_ptr( sizeof( mh ) );
	       
	       *(reinterpret_cast<T *>(mb->wr_ptr())) = t;
	       mb->wr_ptr( mb->wr_ptr() + sizeof(T) );
	    }
	    return mb;
	 }
	 
	 static Msg& peek_message( ACE_Message_Block * mb ) {
	    return *( reinterpret_cast<Msg *>( mb->base() ) );
	 }
	 
	 static const Msg& peek_message( const ACE_Message_Block * mb) {
	    return *( reinterpret_cast<Msg *>( mb->base() ) );
	 }
	 
	 template<typename T> static T& peek_data( ACE_Message_Block * mb ) {
	    return *( reinterpret_cast<T *>( mb->base() + sizeof(Msg) ) );
	 }
	 
	 template<typename T> static bool readq( ACE_Message_Block * mb, T& t ) {
	    if ( mb ) {
	       t = *( reinterpret_cast<T *>( mb->rd_ptr() ) );
	       mb->rd_ptr( mb->rd_ptr() + sizeof(T) );
	       return true;	  
	    }
	    return false;
	 }
#endif
   };

}

#endif // MESSAGEBLOCK_H

// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Copyright (C) MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

namespace tofcontroller {

    template<class T> class marshal {
    public:
        static const T& get( ACE_Message_Block * mb ) {
            T& t = *reinterpret_cast<T*>( mb->rd_ptr() );
            mb->rd_ptr( sizeof(T) );
            return t;
        }

        static ACE_Message_Block * put( const T& t, unsigned long msg_type = 0  ) {
            ACE_Message_Block * mb = new ACE_Message_Block( sizeof(T) );
            *reinterpret_cast<T*>( mb->wr_ptr() ) = t;
            mb->wr_ptr( sizeof(T) );
            if ( msg_type >= ACE_Message_Block::MB_USER )
                mb->msg_type( msg_type );
            return mb;
        }
    };
}

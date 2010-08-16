// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

namespace tofcontroller {

    template<class T> class marshal {
    public:
        static T get( ACE_Message_Block * mb ) {
            TAO_InputCDR in( mb );
            T t;
            in >> t;
            return t;
        }
        static ACE_Message_Block * put( const T& t, unsigned long msg_type = 0  ) {
            TAO_OutputCDR out;
            out << t;
            ACE_Message_Block * mb = out.begin()->duplicate();
            if ( msg_type >= ACE_Message_Block::MB_USER )
                mb->msg_type( msg_type );
            return mb;
        }
    };

}

// -*- C++ -*-
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Science Liaison / Advanced Instrumentation Project
*
** Contact: toshi.hondo@scienceliaison.com
**
** Commercial Usage
**
** Licensees holding valid ScienceLiaison commercial licenses may use this
** file in accordance with the ScienceLiaison Commercial License Agreement
** provided with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and ScienceLiaison.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

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

// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2011 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
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

#include <tao/CDR.h>

namespace adcontroller {

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

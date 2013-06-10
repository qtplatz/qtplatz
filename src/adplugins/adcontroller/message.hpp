// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
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
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// MS-Cheminformatics LLC / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

class ACE_OutputCDR;
class ACE_InputCDR;
class ACE_Time_Value;

namespace adcontroller {

    class Message {
    public:
        Message(unsigned long srcId = 0, unsigned long dstId = 0
                , unsigned long cmdId = 0, unsigned long seqId = 0 );
        ~Message(void);
        unsigned long srcId_;
        unsigned long dstId_;
        unsigned long cmdId_;
        unsigned long seqId_;
    };
    
    ACE_OutputCDR& operator << ( ACE_OutputCDR&, const Message& m );
    ACE_InputCDR& operator >> ( ACE_InputCDR&, Message& m );
    ACE_OutputCDR& operator << ( ACE_OutputCDR&, const ACE_Time_Value& m );
    ACE_InputCDR& operator >> ( ACE_InputCDR&, ACE_Time_Value& );
    
    enum eNotify {
        Notify_Timeout = 512,
        Notify_EventLog,
        Notify_Last
    };
    
    enum eCommand {
        Command_None = 1024,
        Command_Last
    };
    
}

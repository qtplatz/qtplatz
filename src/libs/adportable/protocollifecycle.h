// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#pragma once

namespace adportable {

  namespace protocol {

    namespace constants {
      enum LifeCycleState {
	LCS_CLOSED
	, LCS_LISTEN
	, LCS_SYN_RCVD
	, LCS_SYN_SENT
	, LCS_ESTABLISHED
	, LCS_CLOSE_WAIT
      };

      enum LifeCycleCommand {
	LCC_HELO   = unsigned long ( 'H' << 24 | 'E' << 16 | 'L' << 8 | 'O' )
	, LCC_REST = unsigned long ( 'R' << 24 | 'E' << 16 | 'S' << 8 | 'T' )
	, LCC_INQq = unsigned long ( 'I' << 24 | 'N' << 16 | 'Q' << 8 | '?' )
	, LCC_CONq = unsigned long ( 'C' << 24 | 'O' << 16 | 'N' << 8 | '?' )
	, LCC_RSTq = unsigned long ( 'R' << 24 | 'S' << 16 | 'T' << 8 | '?' )
	, LCC_RSTr = unsigned long ( 'R' << 24 | 'S' << 16 | 'T' << 8 | '@' )
	, LCC_ERR  = unsigned long ( 'E' << 24 | 'R' << 16 | 'R' << 8 | 'O' )
	, LCC_ERST = unsigned long ( 'E' << 24 | 'R' << 16 | 'S' << 8 | 'T' )
      };
    }

    class LifeCycle {
    public:
      LifeCycle();

    };

  }
  
}


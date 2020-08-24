/* To launch this program from within Mathematica use:
 *   In[1]:= link = Install["addtwo"]
 *
 * Or, launch this program from a shell and establish a
 * peer-to-peer connection.  When given the prompt Create Link:
 * type a port name. ( On Unix platforms, a port name is a
 * number less than 65536.  On Mac or Windows platforms,
 * it's an arbitrary word.)
 * Then, from within Mathematica use:
 *   In[1]:= link = Install["portname", LinkMode->Connect]
 */

#include "wstp.h"

extern int addtwo( int i, int j );

int
addtwo( int i, int j )
{
	return i+j;
}

int
main(int argc, char* argv[])
{
	return WSMain(argc, argv);
}

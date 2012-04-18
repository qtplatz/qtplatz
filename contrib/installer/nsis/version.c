#include <stdio.h>

static char __product_revision__[] = "$Revision: 1.0 $";
static char __product_release_ident__[] = "$Id: c938e6c8207b91ebc66ebe930d65c93643e0688a $";

#ifndef INCLUDE_ONLY

main(int ac, char * av[])
{
  int c, n;
  char version[256];

  --ac;
  ++av;

  memset(version, 0, sizeof(version));
  n = 0;
  while ( (c = getchar()) != EOF && n < sizeof(version) ) {
	version[n++] = (c != '\n') ? c : 0;

  }

  while ( ac-- ) {
	printf(*av);
	if ( ac )
	  putchar( ' ' );
	++av;
  }

  printf("\"%s\"", version);
}
#endif

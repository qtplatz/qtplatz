#include <stdio.h>

/*
static char __product_revision__[] = "$Revision: 1.0 $";
static char __product_release_ident__[] = "$Id: c938e6c8207b91ebc66ebe930d65c93643e0688a $";
*/

main(int ac, char * av[])
{
    int c, n;
    int v[4];
    int batchfile = 0;

    --ac;
    ++av;
    if ( ac ) {
        if ( strcmp( av[0], "-b" ) == 0 )
            batchfile = 1;
    }

    memset(v, 0, sizeof(v));
    n = 0;
    while ( (c = getchar()) != EOF && !isdigit(c) )
        ;
    ungetc( c, stdin );
    while ( (c = getchar()) != EOF ) {
        if ( isdigit( c ) ) 
            v[n] = v[n] * 10 + (c - '0');
        else
            ++n;
        if ( n >= 4 )
            break;
    }
    if ( batchfile )
        printf("copy qtplatz.msi qtplatz_x64-%d.%d.%d.%d.msi", v[0], v[1], v[2], v[3] );
    else
        printf("<Include>\n<?define ProductVersion =\"%d.%d.%d.%d\" ?>\n</Include>", v[0], v[1], v[2], v[3]);
}


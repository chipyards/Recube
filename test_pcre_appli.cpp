#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pcre.h>
#include "pcreux.h"

extern "C" {
/* gasp classique --> stderr par defaut */
FILE * logfil=stderr;

void gasp( const char *fmt, ... )  /* fatal error handling */
{
  va_list  argptr;
  fprintf( logfil, "ERROR : " );
  va_start( argptr, fmt );
  vfprintf( logfil, fmt, argptr );
  va_end( argptr );
  fprintf( logfil, "\n" );
  fclose(logfil);
  exit(1);
}
}

void usage(void)
{
printf("usage : appli input_file regex\n");
exit(1);
}

#define QXML	(1<<16)

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
FILE * srcfil;
FILE * dstfil = stdout;
char xmlbuf[QXML];
int qxml = 0;
pcreux * lareu;
int mcnt;

if	( argc < 3 )
	usage();

srcfil = fopen( argv[1], "r" );
if	( srcfil == NULL )
	gasp("echec ouverture %s", argv[1] );

qxml = fread( xmlbuf, 1, QXML, srcfil );

if	( qxml <= 0 )
	gasp("rien lu dans %s", argv[1] );
if	( qxml == QXML )
	gasp("%s trop gros (%d bytes)", argv[1], qxml );

printf("lu %d bytes\n", qxml ); 

lareu = new pcreux( argv[2], PCRE_NEWLINE_ANY | PCRE_MULTILINE | PCRE_DOTALL );
lareu->letexte = xmlbuf;
lareu->lalen = qxml;
lareu->start = 0;

while	( ( mcnt = lareu->matchav() > 0 ) )
	{
	printf("m=%d (%d:%d)\n", mcnt, lareu->ovector[0], lareu->ovector[1] );
	for	( int i = lareu->ovector[2]; i < lareu->ovector[3]; ++i )
		{
		putchar( xmlbuf[i] );
		}
	printf("\n\n");
	}
	
return 0;
}
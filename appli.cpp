/* compilation sous MinGW :
  Image has the following dependencies:
    KERNEL32.dll
    msvcrt.dll
    libpcre-1.dll
    libgcc_s_dw2-1.dll
    libstdc++-6.dll
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <pcre.h>
#include "pcreux.h"

using namespace std;
#include <string>
//#include <sstream>
#include <vector>
#include <map>

extern "C" {
void gasp( const char *fmt, ... );  /* fatal error handling */
}

#include "xmlpe.h"
#include "charvec.h"
#include "projfile.h"

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
printf( "\nusage : recube {options} fichier_projet\n"
	"   option -n : nom de projet (Eclipse)\n"
	"   option -p : chemin pour les drivers\n"
	"exemple 1 (Keil) :\n"
	"   recube -pR:/LLGDB/CubeF1 U:\\TP_stm32\\GPIO\\monTP.uvproj\n"
	"exemple 2 (Eclipse) :\n"
	"   recube -pR:/LLGDB/CubeL4 U:\\TP_stm32\\monTP1\\.project\n"
	"exemple 3 (Eclipse) :\n"
	"   recube -pR:/LLGDB/CubeL4 -nmonTP2 U:\\TP_stm32\\monTP1\\.project\n\n"
	);
fflush(stdout); getchar(); exit(1);
}

/* ============================== the main ===================================== */
int main( int argc, char ** argv )
{
int retval;
const char * new_driver_path = NULL;	// exemple R:/LLGDB/CubeF1/ ou R:\LLGDB\CubeF1
char fixed_driver_path[256];		// exemple R:/LLGDB/CubeF1/ apres correction
const char * new_proj_name = NULL;
const char * start_path = NULL;		// exemple U:\TP\GPIO\tp_gpoi.uvproj
const char * ext;			// exemple .uvproj
char proj_base[256];			// exemple U:\TP\GPIO\tp_gpoi

// etape 1 : identifier les arguments
for	( int i = 1; i < argc; ++i )
	{
	if	( argv[i][0] == '-' )
		{
		switch	( argv[i][1] )
			{
			case 'p' : new_driver_path = argv[i] + 2;
				break;
			case 'n' : new_proj_name = argv[i] + 2;
				break;
			default  : usage();
			}
		}
	else	start_path = argv[i];
	}

if	( start_path == NULL )
	usage();

// etape 2 : mise en conformite de new_driver_path
if	( new_driver_path )
	{
	char c = 0; unsigned int i, len = strlen( new_driver_path );
	if	( len >= ( sizeof(fixed_driver_path) - 2 ) )
		usage();
	for	( i = 0; i < len; ++i )
		{
		c = new_driver_path[i];
		if	( c == '\\' )
			c = '/';		// c'est surtout Eclipse qui prefere les slashes
		fixed_driver_path[i] = c;
		}
	if	( ( c != '/' ) && ( c != '\\' ) )
		fixed_driver_path[i++] = '/';	// ajouter / a la fin si necessaire
	fixed_driver_path[i] = 0;
	new_driver_path = fixed_driver_path;
	}

// etape 3 : extraire base et extension du chemin du projet
pcreux * lareu = new pcreux("^(.*)([.][a-z]+)$");
retval = lareu->matchav( start_path, strlen(start_path) );
if	( retval <= 1 )
	usage();
ext = start_path + lareu->ovector[4];
snprintf( proj_base, sizeof(proj_base), "%.*s", lareu->ovector[3], start_path );

// printf("[%s] [%s]\n", proj_base, ext );
delete lareu;
lareu = new pcreux(".cproject|.project|.uvprojx|.uvoptx");
retval = lareu->matchav( ext, strlen(ext) );
if	( retval < 1 )
	usage();

// fflush(stdout); getchar(); return 0;

projfile * leproj;

if	( ext[1] == 'u' )
	{
	leproj = new projfile();
	retval = leproj->process_file( proj_base, ".uvprojx", new_driver_path, NULL );
	delete leproj;

	leproj = new projfile();
	retval = leproj->process_file( proj_base, ".uvoptx", new_driver_path, NULL );
	delete leproj;	}
else	{
	leproj = new projfile();
	retval = leproj->process_file( proj_base, ".cproject", new_driver_path, NULL );
	delete leproj;

	leproj = new projfile();
	retval = leproj->process_file( proj_base, ".project", new_driver_path, new_proj_name );
	delete leproj;
	}

fflush(stdout); getchar();
return 0;
}

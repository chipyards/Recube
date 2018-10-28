// poor people's char vector

#define INIT_ALLOC 1024
 
class charvec {
public :
char * data;
int capa;
int size;

// constructeur
charvec() :  data(NULL), capa(0), size(0) {};

// destructeur
~charvec() { 
  if	( data )
	free( data );
  }

// methodes
void push( int c ) {
  if	( size >= capa )
	{
	if	( capa == 0 )
		{
		capa = INIT_ALLOC;
		data = (char *)malloc( capa );
		}
	else	{
		capa *= 2;
		data = (char *)realloc( (void *)data, capa );
		}
	if	( data == NULL )
		gasp("echec alloc memoire %d", capa );
	}
  data[size++] = c;
  }
};


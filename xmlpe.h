/* parseur XML version E
 */

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>

/* equivalence stack vs vector
	stack:	top	push		pop
	vector:	back	push_back	pop_back
 "the only reason to use stack is to restrict operation"
 (no iteration, no index)
 */

using namespace std;

/* une DTD (Document Type Definition) ----------------------- */

class DTDelem {
public :
map < string, int > attrib;	// pour le moment, valeur int non utilisee
};

class DTD {
public :
map < string, DTDelem > elem;
};

/* un element XML ----------------------------------- */

class xelem {
public:

// data
string tag;	// the tag name
string inner;	// the text between the tags
map < string, string > attr;	// the attributes, by names

// constructeurs
xelem() {};
explicit xelem( const string & thetag ) {
  tag = thetag;
  }

// accesseurs
void xmlout( ostream & os );	// emettre du xml...

};	// class xelem

/* un fichier XML ---------------------------------- */
class xmlobj {
public:
// data
const char * filepath;
ifstream is;
int curchar;
int curlin;
int e;			// etat du parseur
vector < xelem > stac;	// stack of parent elements
DTD * pDTD;		// null, ou pointeur sur une DTD
// constructors
xmlobj() : curchar(0), curlin(0), e(0), pDTD(NULL)  {};
explicit xmlobj( const char* fnam, DTD * votreDTD = NULL ) : is(fnam) {
  filepath = fnam; pDTD = votreDTD; curlin = 0; e = 0;
  }

// parser
/* step returns  :	0 for EOF
			1 for "start element"
			2 for "end element"
 */
int step();

}; 	// class xmlobj


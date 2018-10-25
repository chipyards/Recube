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
int start_pos0;		// position suivant le premier '<'
int start_pos1;		// position du premier '>' (ou -1 si empty element)
int end_pos0;		// position suivant le 2nd '<' (ou -1 si empty element)
int end_pos1;		// position du dernier '>'
// constructeurs
xelem() : start_pos0(-1), start_pos1(-1), end_pos0(-1), end_pos1(-1) {};
explicit xelem( const string & thetag ) : start_pos0(-1), start_pos1(-1), end_pos0(-1), end_pos1(-1) {
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
int curchar;	// position du prochain char
int curlin;	// numero de la prochaine ligne
int lt_pos;	// position du dernier '<'
string nam;	// nom de tag ou d'attribut courant
string val;	// valeur d'attribut courant

int e;			// etat du parseur
int inner_flag;		// indique la presence d'un contenu non blanc pour inner
vector < xelem > stac;	// stack of parent elements
DTD * pDTD;		// null, ou pointeur sur une DTD
// constructors
xmlobj() : curchar(0), curlin(0), e(0), pDTD(NULL)  {};
explicit xmlobj( const char* fnam, DTD * votreDTD = NULL ) : is(fnam) {
  filepath = fnam; pDTD = votreDTD; curchar = 0; curlin = 0; e = 0;
  }
// cette fonction parse un caractere et rend zero sauf si on a une fin de tag (ou une erreur)
int proc1char( int c );
// cette fonction parse le stream en s'arretant à chaque fin de tag
int step();

}; 	// class xmlobj


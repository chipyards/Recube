/* parseur XML version EB
   - maintient une pile de l'ascendance de l'element courant
   - teste l'existence des noms d'attributs dans un objet DTD s'il existe
     ( mais ne lit pas la DTD du XML, l'objet DTD doit etre cree par l'application )
   - derive du parseur en C de VirVol 1 a 14
   - derive du parseur version C de JADE
   - derive du parseur version D de TREE (Lilas4)
   - supporte CDATA (mais recupere pas)
   - recupere le texte 'inner' entre start-tag et end-tag (si non-blanc)
   - compte les lignes et les caracteres
   - step differentie plus clairement empty-element-tag et start-tag
   - old bug fix : manquait un else au case 2 :
   - si le texte inner est fractionne entre les sous elements, les segments sont concatenes (new in EB)
   - compiler avec -Wno-misleading-indentation
 */
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include "xmlpe.h"

using namespace std;

// xelem method bodies
void xelem::xmlout( ostream & os )
{	// emettre du xml...
os << "<" << tag;
map<string,string>::iterator p;
for ( p = attr.begin(); p != attr.end(); p++ )
    {
    os << " " << p->first << "=\"" << p->second << "\"";
    }
/*
if   ( text.size() ) os << ">\n" << text << "\n</" << tag << ">\n";
else                 os << " />\n";
*/
os << ">\n";
}

// xmlobj methodes proc1char() et step()
/* le parseur : lit le texte jusqu'a pouvoir retourner :
   0 r.a.s.
   1 nouvel element : fin de start-tag (tous attributs inclus, contenu exclu)
     accessible au sommet de la pile (this->stac)
   2 fin de end-tag de l'element courant
     (encore accessible au sommet de la pile, mais en instance d'etre depile)
   3 fin d'empty element tag (tous attributs inclus) accessible au sommet de la pile
   9 EOF
   <0 si erreur

etats :
-1 : hors tag, depilage immediat requis
 0 : hors tag
 1 : vu <, attente nom
 2 : nom en cours
 3 : espace apres nom, attente nom ou / ou >
 4 : nom attrib en cours
 5 : vu =, attente quote
 6 : vu ", val en cours
 7 : vu / en fin d'empty tag, attente >
 10: vu </, attente nom end tag en cours
 11: vu nom end tag, attente >

 20: vu <?, attente fin PI soit ?
 21: vu ?, attente >

 30: vu <!, attente - ou [
     (hyp. : ENTITY, DOCTYPE, ATTLIST, ELEMENT ou NOTATION n'ont pas de > a l'interieur,
     mais <!-- et [CDATA[ oui... )
 31: vu <! mais ce n'est ni comment ni CDATA, alors attendre >

 40: vu <!-, attente -
 41: vu <!-- attente -
 42: vu <!-- -, attente -
 43: vu <!-- --, attente >

 50: vu <![, attente C
 51: vu <![C, attente D
 52: vu <![CD, attente A
 53: vu <![CDA, attente T
 54: vu <![CDAT, attente A
 55: vu <![CDATA, attente [
 56: vu <![CDATA[, attente ]
 57: vu <![CDATA[..], attente ]
 58: vu <![CDATA[..]], attente >

 70: inner en cours, attente <

BUGs : (pas graves)
	- ne devrait pas tolerer blanc entre < et nom tag
	- devrait tolerer blanc entre nom attr et =
 */

// cette fonction parse un caractere et rend zero sauf si on a une fin de tag (ou une erreur)
int xmlobj::proc1char( int c )
{
++curchar;
if	( c == 10 )
	curlin++;
if	( e == -1 )	// depilage eventuel avant lecture char
	{
	if	( stac.size() )
		stac.pop_back();
	else	return(-667);
	e = 0;
	}
switch	( e )
	{
	case 0:	if	( c == '<' )
			{
			e = 1;
			lt_pos = curchar;
			nam = "";
			}
		else	{
			if	( stac.size() )
				{
				val = char(c);
				inner_flag = ( c > ' ' );
				e = 70;
				}
			}			break;
	case 1:	if	( c == '/' )
			{
			e = 10;
			nam = "";
			}
		else if	( c == '?' ) e = 20;
		else if	( c == '!' ) e = 30;
		else if	( c > ' ' )
			{
			e = 2;
			nam = char(c);
			}			break;
	case 2:	if	( ( c == '>' ) || ( c == '/' ) || ( c <= ' ' ) )
			{
			// printf(" element %s\n", nam.c_str() );
			if	( pDTD )
				{
				if	( pDTD->elem.count(nam) == 0 )
					return( -1983 );
				}
			stac.push_back( xelem(nam) );
			if	( c == '>' )			// fin de start-tag
				{
				e = 0;
				stac.back().start_pos0 = lt_pos;
				stac.back().start_pos1 = curchar - 1;
				return(1);
				}
			else if	( c == '/' )
				e = 7;
			else	e = 3;
			}
		else	nam += char(c);		break;
	case 3:	if	( c == '>' )
			{
			e = 0;				// fin de start-tag
			stac.back().start_pos0 = lt_pos;
			stac.back().start_pos1 = curchar - 1;
			return(1);
			}
		else if	( c == '/' )
			e = 7;
		else if	( c > ' ' )
			{
			e = 4;
			nam = char(c);
			}			break;
	case 4:	if	( c == '=' )
			{
			e = 5;
			// printf("   element %s attribut %s\n", stac.back().tag.c_str(), nam.c_str() );
			if	( pDTD )
				{
				if	( pDTD->elem[stac.back().tag].attrib.count(nam) == 0 )
					return( -1984 );
				}
			stac.back().attr[nam] = "";
			}
		else if	( c > ' ' )
			{
			nam += char(c);
			}
		else	return(-400);		break;
	case 5:	if	( c == '"' )
			{
			e = 6;
			val = "";
			}
		else if	( c > ' ' )
			return(-500);		break;
	case 6:	if	( c == '"' )
			{
			e = 3;
			stac.back().attr[nam] = val;
			}
		else	{
			val += char(c);
			}			break;
	case 7:	if	( c == '>' )
			{
			e = -1;				// fin d'empty element tag
			stac.back().start_pos0 = lt_pos;
			stac.back().end_pos1 = curchar - 1;
			return(3);
			}
		else	return(-700);		break;
	case 10: if	( c == '>' )
			{
			e = -1;				// fin de end-tag
			stac.back().end_pos0 = lt_pos;
			stac.back().end_pos1 = curchar - 1;
			if	( nam != stac.back().tag )
				return(-1001);
			return(2);
			}
		else if	( c > ' ' )
			{
			nam += char(c);
			}
		else	e = 11;			break;
	case 11: if	( c == '>' )
			{
			e = -1;				// fin de end-tag
			stac.back().end_pos0 = lt_pos;
			stac.back().end_pos1 = curchar - 1;
			if	( nam != stac.back().tag )
				return(-1001);
			return(2);
			}
		else if	( c > ' ' )
			return(-1100);		break;
	case 20: if	( c == '?' )
			e = 21;			break;
	case 21: if	( c == '>' )
			e = 0;
		else	e = 20;			break;
	case 30: if	( c == '-' )
			e = 40;
		else if	( c == '[' )
			e = 50;
		else	e = 31;			break;
	case 31: if	( c == '>' )
			e = 0;
	case 40: if	( c == '-' )
			e = 41;
		else	return(-4000);		break;
	case 41: if	( c == '-' )
			e = 42;			break;
	case 42: if	( c == '-' )
			e = 43;
		else	e = 41;			break;
	case 43: if	( c == '>' )
			e = 0;
                else	e = 41;			break;
	case 50: if	( c == 'C' )
			e = 51;
		else	e = 31;			break;
	case 51: if	( c == 'D' )
			e = 52;
		else	e = 31;			break;
	case 52: if	( c == 'A' )
			e = 53;
		else	e = 31;			break;
	case 53: if	( c == 'T' )
			e = 54;
		else	e = 31;			break;
	case 54: if	( c == 'A' )
			e = 55;
		else	e = 31;			break;
	case 55: if	( c == '[' )
			e = 56;
		else	e = 31;			break;
	case 56: if	( c == ']' )
			e = 57;			break;
	case 57: if	( c == ']' )
			e = 58;
		else	e = 56;			break;
	case 58: if	( c == '>' )
			e = 0;
                else if	( c == ']' )
			e = 58;
		else	e = 56;			break;
	case 70: if	( c == '<' )
			{
			e = 1;
			lt_pos = curchar;
			if	( inner_flag )
				stac.back().inner += val;
			}
		else	{
			val += char(c);
			if	( c > ' ' )
				inner_flag |= 1;
			}			break;
	default: return(-666);
	}
// printf("c='%c'  e=%d\n", c, e );
return 0;
}

// cette fonction parse le stream en s'arretant à chaque fin de tag
int xmlobj::step()
{
int c;	// caractere courant, int pour percevoir EOF
int retval;

while	( ( c = is.get() ) != EOF )
	{
	if ( c == 0x7F ) return(9);	// fin de recette dans les logs Frevo 8
	retval = proc1char( c );
	if	( retval )
		return retval;
	}
// si on est ici on a atteint la fin du fichier
return(9);
}

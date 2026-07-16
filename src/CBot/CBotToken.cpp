//////////////////////////////////////////////////////////////////
// Gestion des Tokens
// le texte d'un programme est d'abord transform�
// en une suite de tokens pour facilit� l'interpr�tation

// il faudra traiter le seul cas d'erreur possible
// qui est un caract�re ill�gal dans une string


#include "CBot.h"

CBotStringArray CBotToken::m_ListKeyWords;
int				CBotToken::m_ListIdKeyWords[200];
CBotStringArray	CBotToken::m_ListKeyDefine;
long			CBotToken::m_ListKeyNums[MAXDEFNUM];

// constructeurs
CBotToken::CBotToken()
{
	m_next	= NULL;
	m_prev  = NULL;

	m_type = TokenTypVar;			// � priori un nom d'une variable
	m_IdKeyWord = -1;
}

CBotToken::CBotToken(const CBotToken* pSrc)
{
	m_next		= NULL;
	m_prev		= NULL;

	m_Text.Empty();
	m_Sep.Empty();

	m_type		= 0;
	m_IdKeyWord = 0;

	m_start		= 0;
	m_end		= 0;

	if ( pSrc != NULL )
	{
		m_Text		= pSrc->m_Text;
		m_Sep		= pSrc->m_Sep;

		m_type		= pSrc->m_type;
		m_IdKeyWord = pSrc->m_IdKeyWord;

		m_start		= pSrc->m_start;
		m_end		= pSrc->m_end;
	}
}

CBotToken::CBotToken(const CBotString& mot, const CBotString& sep, int start, int end)
{
	m_Text	= mot;					// mot trouv� comme token
	m_Sep	= sep;					// s�parateurs qui suivent
	m_next	= NULL;
	m_prev	= NULL;
	m_start	= start;
	m_end	= end;

	m_type = TokenTypVar;			// � priori un nom d'une variable
	m_IdKeyWord = -1;
}

CBotToken::CBotToken(const char* mot, const char* sep)
{
	m_Text	= mot;
	if ( sep != NULL ) m_Sep = sep;
	m_next	= NULL;
	m_prev	= NULL;

	m_type = TokenTypVar;			// � priori un nom d'une variable
	m_IdKeyWord = -1;
}

CBotToken::~CBotToken()
{
	delete	m_next;					// r�cursif
	m_next = NULL;
}

void CBotToken::Free()
{
	m_ListKeyDefine.SetSize(0);
}

const CBotToken& CBotToken::operator=(const CBotToken& src)
{
	if (m_next != NULL) delete(m_next);
	m_next		= NULL;
	m_prev		= NULL;

	m_Text		= src.m_Text;
	m_Sep		= src.m_Sep;

	m_type		= src.m_type;
	m_IdKeyWord = src.m_IdKeyWord;

	m_start		= src.m_start;
	m_end		= src.m_end;
	return *this;
}


int CBotToken::GivType()
{
	if (this == NULL) return 0;
	if (m_type == TokenTypKeyWord) return m_IdKeyWord;
	return m_type;
}

long CBotToken::GivIdKey()
{
	return m_IdKeyWord;
}

CBotToken* CBotToken::GivNext()
{
	if (this == NULL) return NULL;
	return		m_next;
}

CBotToken* CBotToken::GivPrev()
{
	if (this == NULL) return NULL;
	return		m_prev;
}

void CBotToken::AddNext(CBotToken* p)
{
	CBotToken*	n = new CBotToken(p);
	CBotToken*	pt = this;

	while ( pt->m_next != NULL ) pt = pt->m_next;

	pt->m_next = n;
	n->m_prev = pt;
}


CBotString& CBotToken::GivString()
{
	return	m_Text;
}

CBotString& CBotToken::GivSep()
{
	return	m_Sep;
}

void CBotToken::SetString(const char* name)
{
	m_Text = name;
}


int CBotToken::GivStart()
{
	if (this == NULL) return -1;
	return m_start;
}

int CBotToken::GivEnd()
{
	if (this == NULL) return -1;
	return m_end;
}

void CBotToken::SetPos(int start, int end)
{
	m_start = start;
	m_end	= end;
}

BOOL CharInList(const char c, const char* list)
{
	int		i = 0;

	while (TRUE)
	{
		if (c == list[i++]) return TRUE;
		if (list[i] == 0) return FALSE;
	}
}

BOOL Char2InList(const char c, const char cc, const char* list)
{
	int		i = 0;

	while (TRUE)
	{
		if (c == list[i++] &&
			cc == list[i++]) return TRUE;

		if (list[i] == 0) return FALSE;
	}
}

static char*	sep1 = " \r\n\t,:()[]{}-+*/=;><!~^|&%.";
static char*	sep2 = " \r\n\t";							// s�parateurs pures
static char*	sep3 = ",:()[]{}-+*/=;<>!~^|&%.";			// s�parateurs op�rationnels
static char*	num	 = "0123456789";						// le point (unique) est test� s�par�ment
static char*	hexnum	 = "0123456789ABCDEFabcdef";
static char*	nch	 = "\"\r\n\t";							// refus� dans les chaines

//static char*	duo	 = "+=-=*=/===!=<=>=++--///**/||&&";// les op�rateurs doubles

// cherche le prochain token dans une phrase
// ne doit pas commencer par des s�parateurs
// qui sont pris avec le token pr�c�dent
CBotToken*	CBotToken::NextToken(char* &program, int& error, BOOL first)
{
	CBotString		mot;				// le mot trouv�
	CBotString		sep;				// les s�parateurs qui le suivent
	char			c;
	BOOL			stop = first;

	if (*program == 0) return NULL;

	c   = *(program++);					// prochain caract�re

	if (!first)
	{
		mot = c;							// construit le mot
		c   = *(program++);					// prochain caract�re

		// cas particulier pour les cha�nes de caract�res
		if ( mot[0] == '\"' )
		{
			while (c != 0 && !CharInList(c, nch))
			{
				mot += c;
				c   = *(program++);					// prochain caract�re
				if ( c == '\\' )
				{
					c   = *(program++);					// prochain caract�re
					if ( c == 'n' ) c = '\n';
					if ( c == 'r' ) c = '\r';
					if ( c == 't' ) c = '\t';
					mot += c;
					c   = *(program++);					// prochain caract�re
				}
			}
			if ( c == '\"' )
			{
				mot += c;							// cha�ne compl�te
				c   = *(program++);					// prochain caract�re
			}
			stop = TRUE;
		}

		// cas particulier pour les nombres
		if ( CharInList(mot[0], num ))
		{
			BOOL	bdot = FALSE;	// trouv� un point ?
			BOOL	bexp = FALSE;	// trouv� un exposant ?

			char*	liste = num;
			if (mot[0] == '0' && c == 'x')			// valeur hexad�cimale ?
			{
				mot += c;
				c   = *(program++);					// prochain caract�re
				liste = hexnum;
			}
cw:
			while (c != 0 && CharInList(c, liste))
			{
cc:				mot += c;
				c   = *(program++);					// prochain caract�re
			}
			if ( liste == num )						// pas pour les exad�cimaux
			{
				if ( !bdot && c == '.' ) { bdot = TRUE; goto cc; }
				if ( !bexp && ( c == 'e' || c == 'E' ) )
				{
					bexp = TRUE;
					mot += c;
					c   = *(program++);					// prochain caract�re
					if ( c == '-' ||
						 c == '+' ) goto cc;
					goto cw;
				}

			}
			stop = TRUE;
		}

		if (CharInList(mot[0], sep3))				// un s�parateur op�rationnel ?
		{
			CBotString	motc = mot;
			while (motc += c, c != 0 && GivKeyWords(motc)>0)	// cherche l'op�rande le plus long possible
			{
				mot += c;							// construit le mot
				c = *(program++);					// prochain caract�re
			}

			stop = TRUE;
		}
	}



	while (TRUE)
	{
		if (stop || c == 0 || CharInList(c, sep1))
		{
			if (!first && mot.IsEmpty()) return NULL;	// fin de l'analyse
bis:
			while (CharInList(c, sep2))
			{
				sep += c;							// tous les s�parateurs qui suivent
				c = *(program++);
			}
			if (c == '/' && *program == '/')		// un commentaire dans le tas ?
			{
				while( c != '\n' && c != 0 )
				{
					sep += c;
					c = *(program++);				// prochain caract�re
				}
				goto bis;
			}

			if (c == '/' && *program == '*')		// un commentaire dans le tas ?
			{
				while( c != 0 && (c != '*' || *program != '/'))
				{
					sep += c;
					c = *(program++);				// prochain caract�re
				}
				if ( c != 0 )
				{
					sep += c;
					c = *(program++);				// prochain caract�re
					sep += c;
					c = *(program++);				// prochain caract�re
				}
				goto bis;
			}

			program--;

			CBotToken*	token = new CBotToken(mot, sep);

			if (CharInList( mot[0], num )) token->m_type = TokenTypNum;
			if (mot[0] == '\"')  token->m_type = TokenTypString;
			if (first) token->m_type = 0;

			token->m_IdKeyWord = GivKeyWords(mot);
			if (token->m_IdKeyWord > 0) token->m_type = TokenTypKeyWord;
			else GivKeyDefNum(mot, token) ;			// traite les DefineNum

			return token;
		}

		mot += c;						// construit le mot
		c = *(program++);				// prochain caract�re
	}
}

CBotToken* CBotToken::CompileTokens(const char* program, int& error)
{
	CBotToken		*nxt, *prv, *tokenbase;
	char*			p = (char*) program;
	int				pos = 0;

	error = 0;
	prv = tokenbase = NextToken(p, error, TRUE);

	if (tokenbase == NULL) return NULL;

	tokenbase->m_start	= pos;
	pos += tokenbase->m_Text.GivLength();
	tokenbase->m_end	= pos;
	pos += tokenbase->m_Sep.GivLength();

	char* pp = p;
	while (NULL != (nxt = NextToken(p, error)))
	{
		prv->m_next = nxt;				// ajoute � la suite
		nxt->m_prev = prv;
		prv = nxt;						// avance

		nxt->m_start	= pos;
/*		pos += nxt->m_Text.GivLength();	// la cha�ne peut �tre plus courte (BOA supprim�s)
		nxt->m_end	= pos;
		pos += nxt->m_Sep.GivLength();*/
		pos += (p - pp);				// taille totale
		nxt->m_end	= pos - nxt->m_Sep.GivLength();
		pp = p;
	}

	// ajoute un token comme terminateur
	// ( utile pour avoir le pr�c�dent )
	nxt = new CBotToken();
	nxt->m_type = 0;
	prv->m_next = nxt;				// ajoute � la suite
	nxt->m_prev = prv;

	return tokenbase;
}

void CBotToken::Delete(CBotToken* pToken)
{
	delete pToken;
}


// recherche si un mot fait parti des mots clefs

int CBotToken::GivKeyWords(const char* w)
{
	int		i;
	int		l = m_ListKeyWords.GivSize();

	if (l == 0)
	{
		LoadKeyWords();							// prend la liste la premi�re fois
		l = m_ListKeyWords.GivSize();
	}

	for (i = 0; i < l; i++)
	{
		if (m_ListKeyWords[i] == w) return m_ListIdKeyWords[ i ];
	}

	return -1;
}

BOOL CBotToken::GivKeyDefNum(const char* w, CBotToken* &token)
{
	int		i;
	int		l = m_ListKeyDefine.GivSize();

	for (i = 0; i < l; i++)
	{
		if (m_ListKeyDefine[i] == w)
		{
			token->m_IdKeyWord = m_ListKeyNums[i];
			token->m_type      = TokenTypDef;
			return TRUE;
		}
	}

	return FALSE;
}

// reprend la liste des mots clefs dans les ressources

void CBotToken::LoadKeyWords()
{
	CBotString		s;
	int				i, n = 0;
	
	i = TokenKeyWord;
	while (s.LoadString(i))
	{
		m_ListKeyWords.Add(s);
		m_ListIdKeyWords[n++] = i++;
	}

	i = TokenKeyDeclare;
	while (s.LoadString(i))
	{
		m_ListKeyWords.Add(s);
		m_ListIdKeyWords[n++] = i++;
	}


	i = TokenKeyVal;
	while (s.LoadString(i))
	{
		m_ListKeyWords.Add(s);
		m_ListIdKeyWords[n++] = i++;
	}

	i = TokenKeyOp;
	while (s.LoadString(i))
	{
		m_ListKeyWords.Add(s);
		m_ListIdKeyWords[n++] = i++;
	}
}

BOOL CBotToken::DefineNum(const char* name, long val)
{
	int		i;
	int		l = m_ListKeyDefine.GivSize();

	for (i = 0; i < l; i++)
	{
		if (m_ListKeyDefine[i] == name) return FALSE;
	}
	if ( i == MAXDEFNUM ) return FALSE;

	m_ListKeyDefine.Add( name );
	m_ListKeyNums[i] = val;
	return TRUE;
}

BOOL IsOfType(CBotToken* &p, int type1, int type2)
{
	if (p->GivType() == type1 ||
		p->GivType() == type2 )
	{
		p = p->GivNext();
		return TRUE;
	}
	return FALSE;
}

// idem avec un nombre ind�fini d'arguments
// il faut mettre un z�ro comme dernier argument
BOOL IsOfTypeList(CBotToken* &p, int type1, ...)
{
	int		i = type1;
	int		max = 20;
	int		type = p->GivType();

	va_list marker;
	va_start( marker, type1 );     /* Initialize variable arguments. */

	while (TRUE)
	{
		if (type == i)
		{
			p = p->GivNext();
			va_end( marker );              /* Reset variable arguments.      */
			return TRUE;
		}
		if (--max == 0 || 0 == (i = va_arg( marker, int)))
		{
		   va_end( marker );              /* Reset variable arguments.      */
		   return FALSE;
		}
	}
}


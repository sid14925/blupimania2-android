////////////////////////////////////////////////////////////////////
// interpr�teur pour le language CBot du jeu COLOBOT

// derni�re r�vision : 25/09/2001	DD


#include "resource.h"
#include "CBotDll.h"				// d�finitions publiques
#include "CBotToken.h"				// gestion des tokens

#define	STACKRUN	TRUE			// reprise de l'ex�cution direct sur une routine suspendue
#define	STACKMEM	TRUE			// pr�r�serve la m�moire pour la pile d'ex�cution
#define	MAXSTACK	100				// taille du stack r�serv�

#define	EOX			(CBotStack*)-1	// marqueur condition sp�ciale

/////////////////////////////////////////////////////////////////////
// r�sum� des classes utilis�es, d�finies ci-apr�s

class CBotCompExpr;	// une expression telle que
					// () <= ()
class CBotAddExpr;	// une expression telle que  
					// () + ()
class CBotParExpr;	// un �l�ment de base ou entre parenth�se
					// Toto.truc
					// 12.5
					// "chaine"
					// ( expression )
class CBotExprVar;	// un nom de variable tel que
					// Toto
					// chose.truc.machin
class CBotWhile;	// while (...) {...};
class CBotIf;		// if (...) {...} else {...}
class CBotDefParam;	// liste de param�tres d'une fonction




////////////////////////////////////////////////////////////////////////
// Gestion de la pile d'ex�cution
////////////////////////////////////////////////////////////////////////

// en fait, en externe, la seule chose qu'il est possible de faire
// c'est de cr�er une instance d'une pile  
// pour l'utiliser pour la routine CBotProgram::Execute(CBotStack)

class CBotStack
{
private:
    CBotStack*		m_next;
	CBotStack*		m_next2;
	CBotStack*		m_prev;
	friend class CBotInstArray;
  
	int				m_state;
	int				m_step;
	static int		m_error;
	static int		m_start;
	static int		m_end;
	static
	CBotVar*		m_retvar;					// r�sultat d'un return

	CBotVar*		m_var;						// r�sultat des op�rations
	CBotVar*		m_listVar;					// les variables d�clar�es � ce niveau

	BOOL			m_bBlock;					// fait partie d'un bloc (variables sont locales � ce bloc)
	BOOL			m_bOver;					// limites de la pile ?
//	BOOL			m_bDontDelete;				// sp�cial, ne pas d�truire les variables au delete
	CBotProgram*	m_prog;						// les fonctions d�finies par user

	static
	int				m_initimer;
	static
	int				m_timer;
	static
	CBotString		m_labelBreak;
	static
	void*			m_pUser;

	CBotInstr*		m_instr;					// l'instruction correspondante
	BOOL			m_bFunc;					// une entr�e d'une fonction ?
	CBotCall*		m_call;						// point de reprise dans un call extern
	friend class	CBotTry;

public:
#if	STACKMEM
	static
	CBotStack*		FirstStack();
	void			Delete();
#endif
					CBotStack(CBotStack* ppapa);
					~CBotStack();
	BOOL			StackOver();

	int				GivError(int& start, int& end);
	int				GivError();						// rend le num�ro d'erreur retourn�

	void			Reset(void* pUser);				// plus d'erreur, timer au d�but
	void			SetType(CBotTypResult& type);	// d�termine le type
	int				GivType(int mode = 0);			// donne le type de valeur sur le stack
	CBotTypResult	GivTypResult(int mode = 0);		// donne le type complet de valeur sur le stack

//	void			AddVar(CBotVar* p, BOOL bDontDelete=FALSE);			// ajoute une variable locale
	void			AddVar(CBotVar* p);									// ajoute une variable locale
//	void			RestoreVar(CBotVar* pVar);

	CBotVar*		FindVar(CBotToken* &p, BOOL bUpdate = FALSE,
										   BOOL bModif  = FALSE);		// trouve une variable
	CBotVar*		FindVar(CBotToken& Token, BOOL bUpdate = FALSE,
											  BOOL bModif  = FALSE);
	CBotVar*		FindVar(const char* name);
	CBotVar*		FindVar(long ident, BOOL bUpdate = FALSE,
										BOOL bModif  = FALSE);

	CBotVar*		CopyVar(CBotToken& Token, BOOL bUpdate = FALSE);	// trouve et rend une copie


	CBotStack*		AddStack(CBotInstr* instr = NULL, BOOL bBlock = FALSE);	// �tend le stack
	CBotStack*		AddStackEOX(CBotCall* instr = NULL, BOOL bBlock = FALSE);	// �tend le stack
	CBotStack*		RestoreStack(CBotInstr* instr = NULL);
	CBotStack*		RestoreStackEOX(CBotCall* instr = NULL);

	CBotStack*		AddStack2(BOOL bBlock = FALSE);						// �tend le stack
	BOOL			Return(CBotStack* pFils);							// transmet le r�sultat au dessus
	BOOL			ReturnKeep(CBotStack* pFils);						// transmet le r�sultat sans r�duire la pile
	BOOL			BreakReturn(CBotStack* pfils, const char* name = NULL);
																		// en cas de break �ventuel
	BOOL			IfContinue(int state, const char* name);
																		// ou de "continue"
	
	BOOL			IsOk();

	BOOL			SetState(int n, int lim = -10);						// s�lectionne un �tat
	int				GivState();											// dans quel �tat j'�re ?
	BOOL			IncState(int lim = -10);							// passe � l'�tat suivant
	BOOL			IfStep();											// faire du pas � pas ?
	BOOL			Execute();  

	void			SetVar( CBotVar* var );
	void			SetCopyVar( CBotVar* var );
	CBotVar*		GivVar();
	CBotVar*		GivCopyVar();
	CBotVar*		GivPtVar();
	BOOL			GivRetVar(BOOL bRet);
	long			GivVal();

	void			SetStartError(int pos);
	void			SetError(int n, CBotToken* token = NULL);
	void			SetPosError(CBotToken* token);
	void			ResetError(int n, int start, int end);
	void			SetBreak(int val, const char* name);

	void			SetBotCall(CBotProgram* p);
	CBotProgram*	GivBotCall(BOOL bFirst = FALSE);
	void*			GivPUser();
	BOOL			GivBlock();


//	BOOL			ExecuteCall(CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype);
	BOOL			ExecuteCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotTypResult& rettype);
	void			RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar);

	BOOL			SaveState(FILE* pf);
	BOOL			RestoreState(FILE* pf, CBotStack* &pStack);

	static
	void			SetTimer(int n);

	void			GetRunPos(const char* &FunctionName, int &start, int &end);
	CBotVar*		GivStackVars(const char* &FunctionName, int level);

	int				m_temp;
};

// les routines inline doivent �tre d�clar�es dans le fichier .h

inline BOOL CBotStack::IsOk()
{
    return (m_error == 0);
}

inline int CBotStack::GivState()
{
    return m_state;
}

inline int CBotStack::GivError()
{
	return m_error;
}

////////////////////////////////////////////////////////////////////////
// Gestion de la pile de compilation
////////////////////////////////////////////////////////////////////////


class CBotCStack
{
private:
    CBotCStack*		m_next;
	CBotCStack*		m_prev;

	static
	int				m_error;
	static
	int				m_end;
	int				m_start;

	CBotVar*		m_var;						// r�sultat des op�rations

	BOOL			m_bBlock;					// fait partie d'un bloc (variables sont locales � ce bloc)
	CBotVar*		m_listVar;

	static
	CBotProgram*	m_prog;						// liste des fonctions compil�es
	static
	CBotTypResult	m_retTyp;
//	static
//	CBotToken*		m_retClass;

public:
					CBotCStack(CBotCStack* ppapa);
					~CBotCStack();

	BOOL			IsOk();
	int				GivError();
	int				GivError(int& start, int& end);
												// rend le num�ro d'erreur retourn�

	void			SetType(CBotTypResult& type);// d�termine le type
	CBotTypResult	GivTypResult(int mode = 0);	// donne le type de valeur sur le stack
	int				GivType(int mode = 0);		// donne le type de valeur sur le stack
	CBotClass*		GivClass();					// donne la classe de la valeur sur le stack

	void			AddVar(CBotVar* p);			// ajoute une variable locale
	CBotVar*		FindVar(CBotToken* &p);		// trouve une variable
	CBotVar*		FindVar(CBotToken& Token);
	BOOL			CheckVarLocal(CBotToken* &pToken);
	CBotVar*		CopyVar(CBotToken& Token);	// trouve et rend une copie

	CBotCStack*		TokenStack(CBotToken* pToken = NULL, BOOL bBlock = FALSE);
	CBotInstr*		Return(CBotInstr* p, CBotCStack* pParent);	// transmet le r�sultat au dessus
	CBotFunction*	ReturnFunc(CBotFunction* p, CBotCStack* pParent);	// transmet le r�sultat au dessus
	
	void			SetVar( CBotVar* var );
	void			SetCopyVar( CBotVar* var );
	CBotVar*		GivVar();

	void			SetStartError(int pos);
	void			SetError(int n, int pos);
	void			SetError(int n, CBotToken* p);
	void			ResetError(int n, int start, int end);

	void			SetRetType(CBotTypResult& type);
	CBotTypResult	GivRetType();

//	void			SetBotCall(CBotFunction* &pFunc);
	void			SetBotCall(CBotProgram* p);
	CBotProgram*	GivBotCall();
	CBotTypResult	CompileCall(CBotToken* &p, CBotVar** ppVars, long& nIdent);
	BOOL			CheckCall(CBotToken* &pToken, CBotDefParam* pParam);

	BOOL			NextToken(CBotToken* &p);
};


extern BOOL SaveVar(FILE* pf, CBotVar* pVar);


/////////////////////////////////////////////////////////////////////
// classes d�finissant une instruction
class CBotInstr
{
private:
	static
	CBotStringArray
				m_labelLvl;
protected:
	CBotToken	m_token;				// conserve le token
	CBotString	name;					// debug
	CBotInstr*	m_next;					// instructions cha�n�es
	CBotInstr*	m_next2b;				// seconde liste pour d�finition en cha�ne
	CBotInstr*	m_next3;				// troisi�me liste pour les indices et champs
	static
	int			m_LoopLvl;
	friend class	CBotClassInst;
	friend class	CBotInt;
	friend class	CBotListArray;

public:
				CBotInstr();
				virtual
				~CBotInstr();

	DllExport//debug
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	static
	CBotInstr*	CompileArray(CBotToken* &p, CBotCStack* pStack, CBotTypResult type, BOOL first = TRUE);

	virtual
	BOOL		Execute(CBotStack* &pj);
	virtual
	BOOL		Execute(CBotStack* &pj, CBotVar* pVar);
	virtual
	void		RestoreState(CBotStack* &pj, BOOL bMain);

	virtual
	BOOL		ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
	virtual
	BOOL		ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep, BOOL bExtend);
	virtual
	void		RestoreStateVar(CBotStack* &pile, BOOL bMain);

	virtual
	BOOL		CompCase(CBotStack* &pj, int val);

	void		SetToken(CBotToken* p);
	void		SetToken(CBotString* name, int start=0, int end=0);
	int			GivTokenType();
	CBotToken*	GivToken();

	void		AddNext(CBotInstr* n);
	CBotInstr*	GivNext();
	void		AddNext3(CBotInstr* n);
	CBotInstr*	GivNext3();

	static
	void		IncLvl(CBotString& label);
	static
	void		IncLvl();
	static
	void		DecLvl();
	static
	BOOL		ChkLvl(const CBotString& label, int type);

	BOOL		IsOfClass(CBotString name);
};

class CBotWhile : public CBotInstr
{
private:
	CBotInstr*	m_Condition;		// la condition
	CBotInstr*	m_Block;			// les instructions
	CBotString	m_label;			// une �tiquette s'il y a

public:
				CBotWhile();
				~CBotWhile();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotDo : public CBotInstr
{
private:
	CBotInstr*	m_Block;			// les instructions
	CBotInstr*	m_Condition;		// la condition
	CBotString	m_label;			// une �tiquette s'il y a

public:
				CBotDo();
				~CBotDo();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotFor : public CBotInstr
{
private:
	CBotInstr*	m_Init;				// intruction initiale
	CBotInstr*	m_Test;				// la condition de test
	CBotInstr*	m_Incr;				// instruction pour l'incr�ment
	CBotInstr*	m_Block;			// les instructions
	CBotString	m_label;			// une �tiquette s'il y a

public:
				CBotFor();
				~CBotFor();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotBreak : public CBotInstr
{
private:
	CBotString	m_label;			// une �tiquette s'il y a

public:
				CBotBreak();
				~CBotBreak();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotReturn : public CBotInstr
{
private:
	CBotInstr*	m_Instr;			// le param�tre � retourner

public:
				CBotReturn();
				~CBotReturn();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotSwitch : public CBotInstr
{
private:
	CBotInstr*	m_Value;			// value � chercher
	CBotInstr*	m_Block;			// les instructions

public:
				CBotSwitch();
				~CBotSwitch();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotCase : public CBotInstr
{
private:
	CBotInstr*	m_Value;			// valeur � comparer

public:
				CBotCase();
				~CBotCase();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
	BOOL		CompCase(CBotStack* &pj, int val);
};

class CBotCatch : public CBotInstr
{
private:
	CBotInstr*	m_Block;			// les instructions
	CBotInstr*	m_Cond;				// la condition
	CBotCatch*	m_next;				// le catch suivant
	friend class CBotTry;

public:
				CBotCatch();
				~CBotCatch();
	static
	CBotCatch*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		TestCatch(CBotStack* &pj, int val);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
	void		RestoreCondState(CBotStack* &pj, BOOL bMain);
};

class CBotTry : public CBotInstr
{
private:
	CBotInstr*	m_Block;			// les instructions
	CBotCatch*	m_ListCatch;		// les catches
	CBotInstr*	m_FinalInst;		// instruction finale

public:
				CBotTry();
				~CBotTry();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotThrow : public CBotInstr
{
private:
	CBotInstr*	m_Value;			// la valeur � envoyer

public:
				CBotThrow();
				~CBotThrow();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotStartDebugDD : public CBotInstr
{
private:

public:
				CBotStartDebugDD();
				~CBotStartDebugDD();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
};


class CBotIf : public CBotInstr
{
private:
	CBotInstr*	m_Condition;		// la condition
	CBotInstr*	m_Block;			// les instructions
	CBotInstr*	m_BlockElse;		// les instructions

public:
				CBotIf();
				~CBotIf();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


// d�finition d'un nombre entier

class CBotInt : public CBotInstr
{
private:
	CBotInstr*	m_var;				// la variable � initialiser
	CBotInstr*	m_expr;				// la valeur � mettre, s'il y a
///	CBotInstr*	m_next;				// plusieurs d�finitions encha�n�es

public:
				CBotInt();
				~CBotInt();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont = FALSE, BOOL noskip = FALSE);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// d�finition d'un tableau

class CBotInstArray : public CBotInstr
{
private:
	CBotInstr*	m_var;				// la variable � initialiser
	CBotInstr*	m_listass;			// liste d'assignations pour le tableau
	CBotTypResult
				m_typevar;			// type d'�l�ments
//	CBotString	m_ClassName;

public:
				CBotInstArray();
				~CBotInstArray();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


// d�finition d'une liste d'assignation pour un tableau
// int [ ] a [ ] = ( ( 1, 2, 3 ) , ( 3, 2, 1 ) ) ;

class CBotListArray : public CBotInstr
{
private:
	CBotInstr*	m_expr;				// expression pour un �l�ment
									// les autres sont cha�n�s avec CBotInstr::m_next3;
public:
				CBotListArray();
				~CBotListArray();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, CBotTypResult type);
	BOOL		Execute(CBotStack* &pj, CBotVar* pVar);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotEmpty : public CBotInstr
{
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// d�finition d'un bool�en

class CBotBoolean : public CBotInstr
{
private:
	CBotInstr*	m_var;				// la variable � initialiser
	CBotInstr*	m_expr;				// la valeur � mettre, s'il y a

public:
				CBotBoolean();
				~CBotBoolean();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont = FALSE, BOOL noskip=FALSE);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


// d�finition d'un nombre r�el

class CBotFloat : public CBotInstr
{
private:
	CBotInstr*	m_var;				// la variable � initialiser
	CBotInstr*	m_expr;				// la valeur � mettre, s'il y a

public:
				CBotFloat();
				~CBotFloat();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont = FALSE, BOOL noskip=FALSE);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// d�finition d'un el�ment string

class CBotIString : public CBotInstr
{
private:
	CBotInstr*	m_var;				// la variable � initialiser
	CBotInstr*	m_expr;				// la valeur � mettre, s'il y a

public:
				CBotIString();
				~CBotIString();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, BOOL cont = FALSE, BOOL noskip=FALSE);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// d�finition d'un el�ment dans une classe quelconque

class CBotClassInst : public CBotInstr
{
private:
	CBotInstr*	m_var;				// la variable � initialiser
	CBotClass*	m_pClass;			// r�f�rence � la classe
	CBotInstr*	m_Parameters;		// les param�tres � �valuer pour le constructeur
	CBotInstr*	m_expr;				// la valeur � mettre, s'il y a
	BOOL		m_hasParams;		// il y a des param�tres ?
	long		m_nMethodeIdent;

public:
				CBotClassInst();
				~CBotClassInst();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, CBotClass* pClass = NULL);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotCondition : public CBotInstr
{
private:

public:

	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
};


// left op�rande
// n'accepte que les expressions pouvant �tre � gauche d'une assignation

class CBotLeftExpr : public CBotInstr
{
private:
	long		m_nIdent;

public:
				CBotLeftExpr();
				~CBotLeftExpr();
	static
	CBotLeftExpr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pStack, CBotStack* array);

	BOOL		ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep);
	void		RestoreStateVar(CBotStack* &pile, BOOL bMain);
};


// gestion des champs d'une instance

class CBotFieldExpr : public CBotInstr
{
private:
	friend class CBotExpression;

public:
				CBotFieldExpr();
				~CBotFieldExpr();
//	static
//	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep, BOOL bExtend);
	void		RestoreStateVar(CBotStack* &pj, BOOL bMain);
};

// gestion des index dans les tableaux

class CBotIndexExpr : public CBotInstr
{
private:
	CBotInstr*	 m_expr;					// expression pour le calcul de l'index
	friend class CBotLeftExpr;
	friend class CBotExprVar;

public:
				CBotIndexExpr();
				~CBotIndexExpr();
//	static
//	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotCStack* &pile);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep, BOOL bExtend);
	void		RestoreStateVar(CBotStack* &pj, BOOL bMain);
};

// une expression du genre
// x = a;
// x * y + 3;

class CBotExpression : public CBotInstr
{
private:
	CBotLeftExpr*	m_leftop;			// �l�ment de gauche
	CBotInstr*		m_rightop;			// �l�ment de droite

public:
				CBotExpression();
				~CBotExpression();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pStack);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotListExpression : public CBotInstr
{
private:
	CBotInstr*	m_Expr;				// la 1�re expression � �valuer

public:
				CBotListExpression();
				~CBotListExpression();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pStack);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotLogicExpr : public CBotInstr
{
private:
	CBotInstr*	m_condition;		// test � �valuer
	CBotInstr*	m_op1;				// �l�ment de gauche
	CBotInstr*	m_op2;				// �l�ment de droite
	friend class CBotTwoOpExpr;

public:
				CBotLogicExpr();
				~CBotLogicExpr();
//	static
//	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pStack);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};



class CBotBoolExpr : public CBotInstr
{
private:

public:
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
};



// une expression �ventuellement entre parenth�ses ( ... )
// il n'y a jamais d'instance de cette classe
// l'objet retourn� �tant le contenu de la parenth�se
class CBotParExpr : public CBotInstr
{
private:

public:
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
};

// expression unaire  
class CBotExprUnaire : public CBotInstr
{
private:
	CBotInstr*	m_Expr;				// l'expression � �valuer
public:
				CBotExprUnaire();
				~CBotExprUnaire();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pStack);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// toutes les op�rations � 2 op�randes

class CBotTwoOpExpr : public CBotInstr
{
private:
	CBotInstr*	m_leftop;			// �l�ment de gauche
	CBotInstr*	m_rightop;			// �l�ment de droite
public:
				CBotTwoOpExpr();
				~CBotTwoOpExpr();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, int* pOperations = NULL);
	BOOL		Execute(CBotStack* &pStack);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};




// un bloc d'instructions { .... }
class CBotBlock : public CBotInstr
{
private:

public:
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, BOOL bLocal = TRUE);
	static
	CBotInstr*	CompileBlkOrInst(CBotToken* &p, CBotCStack* pStack, BOOL bLocal = FALSE);
};


// le contenu d'un bloc d'instructions ... ; ... ; ... ; ... ;
class CBotListInstr : public CBotInstr
{
private:
	CBotInstr*	m_Instr;			// les instructions � faire

public:
				CBotListInstr();
				~CBotListInstr();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, BOOL bLocal = TRUE);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotInstrCall : public CBotInstr
{
private:
	CBotInstr*	m_Parameters;		// les param�tres � �valuer
//	int			m_typeRes;			// type du r�sultat
//	CBotString	m_RetClassName;		// class du r�sultat
	CBotTypResult
				m_typRes;			// type complet du r�sultat
	long		m_nFuncIdent;		// id de la fonction

public:
				CBotInstrCall();
				~CBotInstrCall();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// un appel d'une m�thode

class CBotInstrMethode : public CBotInstr
{
private:
	CBotInstr*	m_Parameters;		// les param�tres � �valuer
//	int			m_typeRes;			// type du r�sultat
//	CBotString	m_RetClassName;		// class du r�sultat
	CBotTypResult
				m_typRes;			// type complet du r�sultat

	CBotString	m_NomMethod;		// nom de la m�thode
	long		m_MethodeIdent;		// identificateur de la m�thode
//	long		m_nThisIdent;		// identificateur pour "this"
	CBotString	m_ClassName;		// nom de la classe

public:
				CBotInstrMethode();
				~CBotInstrMethode();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, CBotVar* pVar);
	BOOL		Execute(CBotStack* &pj);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, BOOL bStep, BOOL bExtend);
	void		RestoreStateVar(CBotStack* &pj, BOOL bMain);
};

// expression donnant un nom de variable

class CBotExprVar : public CBotInstr
{
private:
	long		m_nIdent;
	friend class CBotPostIncExpr;
	friend class CBotPreIncExpr;

public:
				CBotExprVar();
				~CBotExprVar();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack, int privat=PR_PROTECT);
	static
	CBotInstr*	CompileMethode(CBotToken* &p, CBotCStack* pStack);

	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
	BOOL		ExecuteVar(CBotVar* &pVar, CBotStack* &pile, CBotToken* prevToken, BOOL bStep);
	BOOL		Execute2Var(CBotVar* &pVar, CBotStack* &pj, CBotToken* prevToken, BOOL bStep);
	void		RestoreStateVar(CBotStack* &pj, BOOL bMain);
};

class CBotPostIncExpr : public CBotInstr
{
private:
	CBotInstr*	m_Instr;
	friend class CBotParExpr;

public:
				CBotPostIncExpr();
				~CBotPostIncExpr();
//	static
//	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotPreIncExpr : public CBotInstr
{
private:
	CBotInstr*	m_Instr;
	friend class CBotParExpr;

public:
				CBotPreIncExpr();
				~CBotPreIncExpr();
//	static
//	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotLeftExprVar : public CBotInstr
{
private:
public:
	CBotTypResult
				m_typevar;			// type de variable d�clar�e
	long		m_nIdent;			// identificateur unique pour cette variable

public:
				CBotLeftExprVar();
				~CBotLeftExprVar();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotExprBool : public CBotInstr
{
private:

public:
				CBotExprBool();
				~CBotExprBool();

	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


class CBotExprNull : public CBotInstr
{
private:

public:
				CBotExprNull();
				~CBotExprNull();

	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotExprNan : public CBotInstr
{
private:

public:
				CBotExprNan();
				~CBotExprNan();

	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

class CBotNew : public CBotInstr
{
private:
	CBotInstr*	m_Parameters;		// les param�tres � �valuer
	long		m_nMethodeIdent;
//	long		m_nThisIdent;
	CBotToken	m_vartoken;

public:
				CBotNew();
				~CBotNew();

	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};

// expression repr�sentant un nombre

class CBotExprNum : public CBotInstr
{
private:
	int			m_numtype;					// et le type de nombre
	long		m_valint;					// valeur pour un int
	float		m_valfloat;					// valeur pour un float

public:
				CBotExprNum();
				~CBotExprNum();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};



// expression repr�sentant une chaine de caract�res

class CBotExprAlpha : public CBotInstr
{
private:

public:
				CBotExprAlpha();
				~CBotExprAlpha();
	static
	CBotInstr*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL		Execute(CBotStack* &pj);
	void		RestoreState(CBotStack* &pj, BOOL bMain);
};


#define	MAX(a,b)	((a>b) ? a : b)


// classe pour la gestion des nombres entier (int)
class CBotVarInt : public CBotVar
{
private:
	int			m_val;			// la valeur
	CBotString	m_defnum;		// le nom si donn� par DefineNum
	friend class CBotVar;

public:
				CBotVarInt( const CBotToken* name );
//				~CBotVarInt();

	void		SetValInt(int val, const char* s = NULL);
	void		SetValFloat(float val);
	int			GivValInt();
	float		GivValFloat();
	CBotString	GivValString();

	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);


	void		Add(CBotVar* left, CBotVar* right);	// addition
	void		Sub(CBotVar* left, CBotVar* right);	// soustraction
	void		Mul(CBotVar* left, CBotVar* right);	// multiplication
	int			Div(CBotVar* left, CBotVar* right);	// division
	int			Modulo(CBotVar* left, CBotVar* right);	// reste de division
	void		Power(CBotVar* left, CBotVar* right);	// puissance

	BOOL		Lo(CBotVar* left, CBotVar* right);
	BOOL		Hi(CBotVar* left, CBotVar* right);
	BOOL		Ls(CBotVar* left, CBotVar* right);
	BOOL		Hs(CBotVar* left, CBotVar* right);
	BOOL		Eq(CBotVar* left, CBotVar* right);
	BOOL		Ne(CBotVar* left, CBotVar* right);

	void		XOr(CBotVar* left, CBotVar* right);
	void		Or(CBotVar* left, CBotVar* right);
	void		And(CBotVar* left, CBotVar* right);

	void		SL(CBotVar* left, CBotVar* right);
	void		SR(CBotVar* left, CBotVar* right);
	void		ASR(CBotVar* left, CBotVar* right);

	void		Neg();
	void		Not();
	void		Inc();
	void		Dec();

	BOOL		Save0State(FILE* pf);
	BOOL		Save1State(FILE* pf);

};

// classe pour la gestion des nombres r�els (float)
class CBotVarFloat : public CBotVar
{
private:
	float		m_val;		// la valeur

public:
				CBotVarFloat( const CBotToken* name );
//				~CBotVarFloat();

	void		SetValInt(int val, const char* s = NULL);
	void		SetValFloat(float val);
	int			GivValInt();
	float		GivValFloat();
	CBotString	GivValString();

	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);


	void		Add(CBotVar* left, CBotVar* right);	// addition
	void		Sub(CBotVar* left, CBotVar* right);	// soustraction
	void		Mul(CBotVar* left, CBotVar* right);	// multiplication
	int 		Div(CBotVar* left, CBotVar* right);	// division
	int			Modulo(CBotVar* left, CBotVar* right);	// reste de division
	void		Power(CBotVar* left, CBotVar* right);	// puissance

	BOOL		Lo(CBotVar* left, CBotVar* right);
	BOOL		Hi(CBotVar* left, CBotVar* right);
	BOOL		Ls(CBotVar* left, CBotVar* right);
	BOOL		Hs(CBotVar* left, CBotVar* right);
	BOOL		Eq(CBotVar* left, CBotVar* right);
	BOOL		Ne(CBotVar* left, CBotVar* right);

	void		Neg();
	void		Inc();
	void		Dec();

	BOOL		Save1State(FILE* pf);
};


// classe pour la gestion des cha�nes (String)
class CBotVarString : public CBotVar
{
private:
	CBotString	m_val;		// la valeur

public:
				CBotVarString( const CBotToken* name );
//				~CBotVarString();

	void		SetValString(const char* p);
	CBotString	GivValString();

	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);

	void		Add(CBotVar* left, CBotVar* right);	// addition

	BOOL		Lo(CBotVar* left, CBotVar* right);
	BOOL		Hi(CBotVar* left, CBotVar* right);
	BOOL		Ls(CBotVar* left, CBotVar* right);
	BOOL		Hs(CBotVar* left, CBotVar* right);
	BOOL		Eq(CBotVar* left, CBotVar* right);
	BOOL		Ne(CBotVar* left, CBotVar* right);

	BOOL		Save1State(FILE* pf);
};

// classe pour la gestion des boolean
class CBotVarBoolean : public CBotVar
{
private:
	BOOL		m_val;		// la valeur

public:
				CBotVarBoolean( const CBotToken* name );
//				~CBotVarBoolean();

	void		SetValInt(int val, const char* s = NULL);
	void		SetValFloat(float val);
	int			GivValInt();
	float		GivValFloat();
	CBotString	GivValString();

	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);

	void		And(CBotVar* left, CBotVar* right);
	void		Or(CBotVar* left, CBotVar* right);
	void		XOr(CBotVar* left, CBotVar* right);
	void		Not();
	BOOL		Eq(CBotVar* left, CBotVar* right);
	BOOL		Ne(CBotVar* left, CBotVar* right);

	BOOL		Save1State(FILE* pf);
};


// classe pour la gestion des instances de classe
class CBotVarClass : public CBotVar
{
private:
	static
	CBotVarClass*	m_ExClass;		// liste des instances existantes � un moment donn�
	CBotVarClass*	m_ExNext;		// pour cette liste g�n�rale
	CBotVarClass*	m_ExPrev;		// pour cette liste g�n�rale

private:
	CBotClass*		m_papa;			// la d�finition de la classe
	CBotVar*		m_pVar;			// contenu
	friend class	CBotVar;		// mon papa est un copain
	friend class	CBotVarPointer;	// et le pointeur aussi
	int				m_CptUse;		// compteur d'utilisation
	long			m_ItemIdent;	// identificateur (unique) de l'instance
	BOOL			m_bConstructor;	// set si un constructeur a �t� appel�

public:
				CBotVarClass( const CBotToken* name, CBotTypResult& type );
				~CBotVarClass();

	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);
	void		SetClass(CBotClass* pClass);
	CBotClass*	GivClass();
	CBotVar*	GivItem(const char* name);	// rend un �l�ment d'une classe selon son nom (*)
	CBotVar*	GivItem(int n, BOOL bExtend);
	CBotVar*	GivItemList();

	CBotString	GivValString();

	BOOL		Save1State(FILE* pf);
	void		Maj(void* pUser, BOOL bContinue);

	void		IncrementUse();				// une r�f�rence en plus
	void		DecrementUse();				// une r�f�rence en moins

	CBotVarClass*  
				GivPointer();
	void		SetItemList(CBotVar* pVar);

	void		SetIdent(long n);
	
	static
	CBotVarClass*
				CBotVarClass::Find(long id);


//	CBotVar*	GivMyThis();

	BOOL		Eq(CBotVar* left, CBotVar* right);
	BOOL		Ne(CBotVar* left, CBotVar* right);

	void		ConstructorSet();
};


// classe pour la gestion des pointeurs � une instances de classe
class CBotVarPointer : public CBotVar
{
private:
	CBotVarClass*
				m_pVarClass;				// contenu
	CBotClass*	m_papa;						// la classe pr�vue pour ce pointeur
	friend class CBotVar;			// mon papa est un copain

public:
				CBotVarPointer( const CBotToken* name, CBotTypResult& type );
				~CBotVarPointer();

	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);
	void		SetClass(CBotClass* pClass);
	CBotClass*	GivClass();
	CBotVar*	GivItem(const char* name);	// rend un �l�ment d'une classe selon son nom (*)
	CBotVar*	GivItemList();

	CBotString	GivValString();
	void		SetPointer(CBotVar* p);
	CBotVarClass*
				GivPointer();

	void		SetIdent(long n);			// associe un num�ro d'identification (unique)
	long		GivIdent();					// donne le num�ro d'identification associ�
	void		ConstructorSet();

	BOOL		Save1State(FILE* pf);
	void		Maj(void* pUser, BOOL bContinue);

	BOOL		Eq(CBotVar* left, CBotVar* right);
	BOOL		Ne(CBotVar* left, CBotVar* right);
};


// classe pour les tableaux

#define	MAXARRAYSIZE	9999

class CBotVarArray : public CBotVar
{
private:
	CBotVarClass*
				m_pInstance;				// instance g�rant le tableau

	friend class CBotVar;					// papa est un copain

public:
				CBotVarArray( const CBotToken* name, CBotTypResult& type );
				~CBotVarArray();

	void		SetPointer(CBotVar* p);
	CBotVarClass*
				GivPointer();
	
	void		Copy(CBotVar* pSrc, BOOL bName=TRUE);
	CBotVar*	GivItem(int n, BOOL bGrow=FALSE);	// rend un �l�ment selon son index num�rique
												// agrandi le tableau si n�cessaire si bExtend
//	CBotVar*	GivItem(const char* name);		// rend un �l�ment selon son index lit�ral
	CBotVar*	GivItemList();					// donne le premier �l�ment de la liste

	CBotString	GivValString();					// donne le contenu du tableau dans une cha�ne

	BOOL		Save1State(FILE* pf);
};


extern CBotInstr* CompileParams(CBotToken* &p, CBotCStack* pStack, CBotVar** ppVars);

extern BOOL TypeCompatible( CBotTypResult& type1, CBotTypResult& type2, int op = 0 );
extern BOOL TypesCompatibles( CBotTypResult type1, CBotTypResult type2 );

extern BOOL WriteWord(FILE* pf, WORD w);
extern BOOL ReadWord(FILE* pf, WORD& w);
extern BOOL ReadLong(FILE* pf, long& w);
extern BOOL WriteFloat(FILE* pf, float w);
extern BOOL WriteLong(FILE* pf, long w);
extern BOOL ReadFloat(FILE* pf, float& w);
extern BOOL WriteString(FILE* pf, CBotString s);
extern BOOL ReadString(FILE* pf, CBotString& s);
extern BOOL WriteType(FILE* pf, CBotTypResult type);
extern BOOL ReadType(FILE* pf, CBotTypResult& type);

extern float GivNumFloat( const char* p );

#ifdef	_DEBUG
extern void DEBUG( const char* text, int val, CBotStack* pile );
#endif

///////////////////////////////////////////
// classe pour les appels de routines (externes)

class CBotCall
{
private:
	static
	CBotCall*	m_ListCalls;
	static
	void*		m_pUser;
	long		m_nFuncIdent;

private:
	CBotString	m_name;
	BOOL		(*m_rExec) (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser) ;
	CBotTypResult
				(*m_rComp) (CBotVar* &pVar, void* pUser)	;
	CBotCall*	m_next;

public:
				CBotCall(const char* name,  
						 BOOL rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),  
						 CBotTypResult rCompile (CBotVar* &pVar, void* pUser));
				~CBotCall();

	static
	BOOL		AddFunction(const char* name,  
							BOOL rExec (CBotVar* pVar, CBotVar* pResult, int& Exception, void* pUser),  
							CBotTypResult rCompile (CBotVar* &pVar, void* pUser));

	static
	CBotTypResult
				CompileCall(CBotToken* &p, CBotVar** ppVars, CBotCStack* pStack, long& nIdent);
	static
	BOOL		CheckCall(const char* name);

//	static
//	int			DoCall(CBotToken* &p, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);
	static
	int			DoCall(long& nIdent, CBotToken* token, CBotVar** ppVars, CBotStack* pStack, CBotTypResult& rettype);
#if	STACKRUN
	BOOL		Run(CBotStack* pStack);
	static
	BOOL		RestoreCall(long& nIdent, CBotToken* token, CBotVar** ppVar, CBotStack* pStack);
#endif

	CBotString	GivName();
	CBotCall*	Next();
	
	static void	SetPUser(void* pUser);
	static void	Free();
};

// classe g�rant les m�thodes d�clar�es par AddFunction sur une classe

class CBotCallMethode
{
private:
	CBotString	m_name;
	BOOL		(*m_rExec) (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception);
	CBotTypResult
				(*m_rComp) (CBotVar* pThis, CBotVar* &pVar);
	CBotCallMethode*	m_next;
	friend class CBotClass;
	long		m_nFuncIdent;

public:
				CBotCallMethode(const char* name,  
						 BOOL rExec (CBotVar* pThis, CBotVar* pVar, CBotVar* pResult, int& Exception),  
						 CBotTypResult rCompile (CBotVar* pThis, CBotVar* &pVar));
				~CBotCallMethode();

	CBotTypResult
				CompileCall(const char* name, CBotVar* pThis,  
							CBotVar** ppVars, CBotCStack* pStack,
							long& nIdent);

	int			DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotVar* &pResult, CBotStack* pStack, CBotToken* pFunc);

	CBotString	GivName();
	CBotCallMethode*	Next();
	void		AddNext(CBotCallMethode* p);
	
};

// une liste de param�tres

class CBotDefParam
{
private:
	CBotToken		m_token;		// nom du param�tre
	CBotString		m_typename;		// nom du type
	CBotTypResult	m_type;			// type de param�tre
	CBotDefParam*	m_next;			// param�tre suivant
	long			m_nIdent;

public:
					CBotDefParam();
					~CBotDefParam();
	static
	CBotDefParam*	Compile(CBotToken* &p, CBotCStack* pStack);
	BOOL			Execute(CBotVar** ppVars, CBotStack* &pj);
	void			RestoreState(CBotStack* &pj, BOOL bMain);

	void			AddNext(CBotDefParam* p);
	int				GivType();
	CBotTypResult	GivTypResult();
	CBotDefParam*	GivNext();

	CBotString		GivParamString();
};


// une d�claration de fonction

class CBotFunction : CBotInstr
{
private:
	// gestion d'une liste (static) de fonctions publiques
	static
	CBotFunction*	m_listPublic;
	CBotFunction*	m_nextpublic;
	CBotFunction*	m_prevpublic;
	friend class	CBotCStack;
//	long			m_nThisIdent;
	long			m_nFuncIdent;
	BOOL			m_bSynchro;		// m�thode synchronis�e ?

private:
	CBotDefParam*	m_Param;		// liste des param�tres
	CBotInstr*		m_Block;		// le bloc d'instructions
	CBotFunction*	m_next;
	CBotToken		m_retToken;		// si retourne un CBotTypClass
	CBotTypResult	m_retTyp;		// type complet du r�sultat

	BOOL			m_bPublic;		// fonction publique
	BOOL			m_bExtern;		// fonction extern
	CBotString		m_MasterClass;	// nom de la classe qu'on d�rive
	CBotProgram*	m_pProg;
	friend class CBotProgram;
	friend class CBotClass;

	CBotToken		m_extern;		// pour la position du mot "extern"
	CBotToken		m_openpar;
	CBotToken		m_closepar;
	CBotToken		m_openblk;
	CBotToken		m_closeblk;
public:
					CBotFunction::CBotFunction();
					CBotFunction::~CBotFunction();
	static
	CBotFunction*	Compile(CBotToken* &p, CBotCStack* pStack, CBotFunction* pFunc, BOOL bLocal = TRUE);
	static
	CBotFunction*	Compile1(CBotToken* &p, CBotCStack* pStack, CBotClass*	pClass);
	BOOL			Execute(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance = NULL);
	void			RestoreState(CBotVar** ppVars, CBotStack* &pj, CBotVar* pInstance = NULL);

	void			AddNext(CBotFunction* p);
	CBotTypResult	CompileCall(const char* name, CBotVar** ppVars, long& nIdent);
	CBotFunction*	FindLocalOrPublic(long& nIdent, const char* name, CBotVar** ppVars, CBotTypResult& TypeOrError, BOOL bPublic = TRUE);

	int				DoCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken);
	void			RestoreCall(long& nIdent, const char* name, CBotVar** ppVars, CBotStack* pStack);
	int				DoCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotToken* pToken, CBotClass* pClass);
	void			RestoreCall(long& nIdent, const char* name, CBotVar* pThis, CBotVar** ppVars, CBotStack* pStack, CBotClass* pClass);
	BOOL			CheckParam(CBotDefParam* pParam);

	static
	void			AddPublic(CBotFunction* pfunc);

	CBotString		GivName();
	CBotString		GivParams();
	BOOL			IsPublic();
	BOOL			IsExtern();
	CBotFunction*	Next();

	BOOL			GetPosition(int& start, int& stop, CBotGet modestart, CBotGet modestop);
};



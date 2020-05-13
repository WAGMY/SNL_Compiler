#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "stdio.h"
#include "stdlib.h"
#include <QVector>
#include <QMap>
#include <QSet>
#include <QString>

typedef enum
{
    ENDFILE,	ERROR,

    PROGRAM,	PROCEDURE,	TYPE,	VAR,		IF,
    THEN,		ELSE,		FI,		WHILE,		DO,
    ENDWH,		BEGIN,		END,	READ,		WRITE,
    ARRAY,		OF,			RECORD,	RETURN,

    INTEGER_T,	CHAR_T,

    ID,			INTC_VAL,		CHARC_VAL,

    ASSIGN,		EQ,			LT,		PLUS,		MINUS,
    TIMES,DIVIDE,		LPAREN,	RPAREN,		DOT,
    COLON,		SEMI,		COMMA,	LMIDPAREN,	RMIDPAREN,
    UNDERRANGE,

    Program,	      ProgramHead,	    ProgramName,	DeclarePart,
    TypeDec,        TypeDeclaration,	TypeDecList,	TypeDecMore,
    TypeId,	      TypeName,			BaseType,	    StructureType,
    ArrayType,      Low,	            Top,            RecType,
    FieldDecList,   FieldDecMore,	    IdList,	        IdMore,
    VarDec,	      VarDeclaration,	VarDecList,		VarDecMore,
    VarIdList,	  VarIdMore,		ProcDec,		ProcDeclaration,
    ProcDecMore,    ProcName,		    ParamList,		ParamDecList,
    ParamMore,      Param,		    FormList,		FidMore,
    ProcDecPart,    ProcBody,	    	ProgramBody,	StmList,
    StmMore,        Stm,				AssCall,		AssignmentRest,
    ConditionalStm, StmL,			    LoopStm,		InputStm,
    InVar,          OutputStm,		ReturnStm,		CallStmRest,
    ActParamList,   ActParamMore,		RelExp,			OtherRelE,
    Exp,			  OtherTerm,		Term,           OtherFactor,
    Factor,         Variable,			VariMore,		FieldVar,
    FieldVarMore,   CmpOp,			AddOp,          MultOp
} LexType;

extern QMap<LexType, QString> lexName;


class Token{
public:
    Token(int line ,LexType lex, QString sem){
        this->line=line;
        this->lex=lex;
        this->sem=sem;
    }
    Token(int line,std::string s)
    {
        this->line=line;
        this->sem = QString::fromStdString(s);
    }
    int getLine() const{return line;}
    QString getLexName() const{return lexName[lex];}
    LexType getLex() const{return lex;}
    QString getSem() const{return sem;}
    Token *next;

private:
    int line;
    LexType lex;
    QString sem;
};


typedef enum {ProK,PheadK,DecK,TypeK,VarK,ProcDecK,StmLK,StmtK,ExpK}
              NodeKind;

extern QMap<NodeKind,QString> nodekind_map;

typedef enum {ArrayK,CharK,IntegerK,RecordK,IdK}  DecKind;
extern QMap<DecKind,QString> deckind_map;


typedef enum {IfK,WhileK,AssignK,ReadK,WriteK,CallK,ReturnK} StmtKind;
extern QMap<StmtKind,QString> stmtkind_map;


typedef enum {OpK,ConstK,VariK} ExpKind;
extern QMap<ExpKind,QString> expkind_map;


typedef enum {IdV,ArrayMembV,FieldMembV} VarKind;
extern QMap<VarKind,QString> varkind_map;


typedef enum {Void,Integer,Boolean} ExpType;
extern QMap<ExpType,QString> exptype_map;

typedef enum {valparamType,varparamType} ParamType;
extern QMap<ParamType,QString> paramtype_map;

#define MAXCHILDREN 3


struct symbtable;

typedef struct treeNode

   {
     struct treeNode * child[MAXCHILDREN];
     struct treeNode * sibling;
     int lineno;
     NodeKind nodekind;
     union
     {
         DecKind  dec;
         StmtKind stmt;
         ExpKind  exp;
     } kind;

     int idnum;

     char name[10][10];

     struct symbtable * table[10];

      char type_name[10];
     struct
     {
        struct
            {
                int low;
                int up;
                DecKind   childtype;
            }ArrayAttr;

        struct
            {
                ParamType  paramt;
            }ProcAttr;

        struct
            {
                LexType op;
                int val;
                VarKind  varkind;
                ExpType type;
            }ExpAttr;


     } attr;
}TreeNode;

extern QSet<LexType> NTSet;
extern QSet<LexType> TTSet;
extern int lineno;
 #endif




















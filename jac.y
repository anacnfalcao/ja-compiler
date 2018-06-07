%{
#include <stdio.h>
#include "ast.h"
#include "sym_tab.h"

int yylex(void);
void yyerror(const char *s);


%}

%union{
    struct data * pos; // position and str 
    struct n* n;
}


%token  CLASS  ELSE IF PUBLIC STATIC STRING WHILE DO OCURV CCURV OBRACE CBRACE OSQUARE CSQUARE SEMI COMMA RESERVED
%token <pos> BOOL BOOLLIT DOUBLE INT ID REALLIT DECLIT STRLIT VOID DOTLENGTH PARSEINT AND OR LT GT EQ NEQ LEQ GEQ PLUS MINUS STAR DIV MOD NOT ASSIGN RETURN PRINT
%type <n> Program ProgramBody FieldDecl VarDecl VarDeclPlus Type MethodDecl MethodHeader FormalParams FormalParamsPlus MethodBody  VarFieldDecl VarFieldDeclPlus VarDeclStatement Statement Expr Expression ParseArgs ExprPlus OptionalExprPlus MethodInvocation Assignment OptionalExpr OptionalAMP ExprSTRLIT OptionalStatement

%right ASSIGN
%left OR
%left AND
%left EQ NEQ
%left LT GT LEQ GEQ
%left PLUS MINUS
%left STAR DIV MOD
%right NOT
%nonassoc IFX
%nonassoc ELSE

%%
Start: Program 										{tree = create_node(PROGRAM_NODE,NULL, 1, $1); }
Program: CLASS ID OBRACE ProgramBody CBRACE 		{$$ = add_brother(create_leaf (ID_NODE, $2), $4); }
ProgramBody: %empty									{$$ = NULL;}
			| FieldDecl ProgramBody 				{$$ = add_brother($1, $2);}
			| MethodDecl ProgramBody 				{$$ = add_brother($1, $2);}
			| SEMI ProgramBody						{$$ = $2;}
			;

FieldDecl: PUBLIC STATIC VarFieldDecl 				{$$ = $3;}
			| error SEMI							{$$ = NULL;flag_ast_error = 1;}		
			;

VarFieldDecl: Type ID VarFieldDeclPlus SEMI				{if (flag_ast_error) {$$ = NULL;} else {$$ = add_brother(create_node(FIELDDECL_NODE,NULL, 2 , $1,  create_leaf(ID_NODE, $2)),add_type($3,-1,$1->type));}}
VarFieldDeclPlus: %empty								{$$ = NULL;}
			| COMMA ID VarFieldDeclPlus					{$$ = add_brother(create_node (FIELDDECL_NODE ,NULL,2,create_leaf(-1, NULL),create_leaf(ID_NODE, $2)), $3); }
			;

Type:  BOOL 											{$$ = create_leaf(BOOL_NODE, NULL);}
	 | INT 												{$$ = create_leaf(INT_NODE, NULL);}
	 | DOUBLE											{$$ = create_leaf(DOUBLE_NODE, NULL);}
	 ;

MethodDecl: PUBLIC STATIC MethodHeader MethodBody 		{$$ = create_node(METHODDECL_NODE,NULL,2 , $3,$4);}

MethodHeader:  Type ID OCURV FormalParams CCURV			{$$ = create_node(METHODHEADER_NODE,NULL,3,$1, create_leaf(ID_NODE, $2),create_node(METHODPARAMS_NODE,NULL,1,$4));}
			 | VOID ID OCURV FormalParams CCURV			{$$ = create_node(METHODHEADER_NODE,NULL,3, create_leaf(VOID_NODE, NULL), create_leaf(ID_NODE, $2),create_node(METHODPARAMS_NODE,NULL,1,$4));}
			 ;

FormalParams: %empty									{$$ = NULL ;}
			  | Type ID FormalParamsPlus				{$$ = add_brother(create_node(PARAMDECL_NODE,NULL,2,$1,create_leaf(ID_NODE, $2) ),$3);}
			  | STRING OSQUARE CSQUARE ID 				{$$ = create_node(PARAMDECL_NODE,NULL, 1,  add_brother(create_leaf (STRINGARRAY_NODE,NULL), create_leaf(ID_NODE, $4)));}
			  ;
FormalParamsPlus: %empty								{$$= NULL;}
				  | COMMA Type ID FormalParamsPlus		{$$ = add_brother(create_node(PARAMDECL_NODE,NULL,2,$2,create_leaf(ID_NODE, $3) ),$4);}
				  ;

MethodBody: OBRACE VarDeclStatement CBRACE				{$$ = create_node (METHODBODY_NODE,NULL, 1, $2);}
VarDeclStatement: %empty 								{$$ = NULL;}
				  | VarDecl VarDeclStatement			{$$ = add_brother($1,$2);}
	              | Statement VarDeclStatement			{$$ = add_brother($1,$2);}
	              ;
VarDecl: Type ID VarDeclPlus SEMI						{ if (flag_ast_error){ $$ = NULL; } else {$$ = add_brother(create_node(VARDECL_NODE,NULL, 2 , $1,  create_leaf(ID_NODE, $2)),add_type($3,-1,$1->type));}}
VarDeclPlus: %empty										{$$ = NULL;}
			| COMMA ID VarDeclPlus						{/*$$ = add_brother(create_node (VARDECL_NODE,NULL, 1,create_leaf(ID_NODE, $2)), $3);*/ $$ = add_brother(create_node (VARDECL_NODE ,NULL,2,create_leaf(-1, NULL),create_leaf(ID_NODE, $2)), $3); }
			;

Statement: OBRACE OptionalStatement CBRACE								{if ($2 != NULL && $2->next != NULL ){$$=create_node(BLOCK_NODE,NULL,1, $2);}else {$$ = $2;}/*Se tiver dois statements forma um block*/}
		   | IF OCURV Expression CCURV Statement %prec IFX				{if ($5 == NULL || $5->next){$5 = create_node (BLOCK_NODE,NULL,1,$5);}; $$ = create_node(IF_NODE,NULL,3, $3, $5, create_leaf(BLOCK_NODE,NULL));}
		   | IF OCURV Expression CCURV Statement	ELSE Statement  	{if ($5 == NULL || $5->next){$5 = create_node (BLOCK_NODE,NULL,1,$5);}; if ($7 == NULL || $7->next) {$7 = create_node (BLOCK_NODE,NULL, 1, $7);} $$ = create_node(IF_NODE,NULL,3, $3, $5, $7);}
		   | WHILE OCURV Expression CCURV Statement 					{if ($5 == NULL || $5->next != NULL ) {$5 = create_node(BLOCK_NODE,NULL,1, $5);};$$ = create_node(WHILE_NODE,NULL,2,$3,$5);}
		   | DO Statement WHILE OCURV Expression CCURV SEMI 			{if ($2 == NULL || $2->next != NULL ) {$2 = create_node(BLOCK_NODE,NULL,1, $2);};$$ = create_node(DOWHILE_NODE,NULL,2,$2,$5);}
		   | PRINT OCURV ExprSTRLIT CCURV SEMI							{$$ = create_node(PRINT_NODE,$1, 1,$3);}
		   | OptionalAMP SEMI											{$$ = $1;}
		   | RETURN OptionalExpr SEMI									{$$ = create_node (RETURN_NODE,$1, 1, $2);}
		   | error SEMI													{$$ = NULL;flag_ast_error = 1;}	
		   ;


OptionalStatement: %empty 										{$$ = NULL;}
				   |Statement OptionalStatement 				{$$ = add_brother ($1, $2);}
				   ;
ExprSTRLIT:  Expression 										{$$ = $1;}
			| STRLIT 											{$$ = create_leaf (STRLIT_NODE, $1);}
			;
OptionalAMP: %empty 											{$$ = NULL;}
			| Assignment 										{$$ = $1;}
			| MethodInvocation 									{$$ = $1;}
			| ParseArgs 										{$$ = $1;}
			;
OptionalExpr: %empty 											{$$ = NULL;}
			  | Expression 										{$$ = $1;}
			  ;

Assignment: ID ASSIGN Expression 								{$$ = create_node (ASSIGN_NODE,$2, 2 , create_leaf(ID_NODE, $1), $3);}

MethodInvocation: ID OCURV OptionalExprPlus CCURV  				{$$ = create_node(CALL_NODE,NULL, 2, create_leaf(ID_NODE, $1), $3);}
				| ID OCURV error CCURV							{$$ = NULL;flag_ast_error = 1;}	
				;
OptionalExprPlus: %empty 										{$$ = NULL;}
				 | Expression ExprPlus 							{$$ = add_brother($1,$2);}
				 ;

ExprPlus: %empty 												{$$ = NULL;}
		  | COMMA Expression ExprPlus 							{$$ = add_brother($2, $3);}
		  ;

ParseArgs: PARSEINT OCURV ID OSQUARE Expression CSQUARE CCURV 		{$$ = create_node (PARSEARGS_NODE,$1, 2,create_leaf(ID_NODE, $3),$5);}
		 | PARSEINT OCURV error CCURV								{$$ = NULL;flag_ast_error = 1;}	
		 ;	

Expression: Assignment   						{$$ = $1;}
	 | Expr 									{$$ = $1;}
	 ;

Expr:  MethodInvocation 						{$$ = $1;}
	 | ParseArgs 								{$$ = $1;}
	 | PLUS Expr 	%prec NOT					{$$ = create_node(PLUS_NODE,$1,1,$2);}
	 | MINUS Expr 	%prec NOT 					{$$ = create_node(MINUS_NODE,$1,1,$2);}
	 | NOT Expr 								{$$ = create_node(NOT_NODE,$1, 1,$2);}
	 | ID DOTLENGTH 							{$$ = create_node(LENGTH_NODE,$2, 1, create_leaf(ID_NODE,$1));}
	 | ID  										{$$ = create_leaf(ID_NODE, $1);}
	 | OCURV Expression CCURV 					{$$ = $2;}
	 | BOOLLIT 									{$$ = create_leaf(BOOLLIT_NODE, $1);}
	 | DECLIT 									{$$ = create_leaf(DECLIT_NODE, $1);}
	 | REALLIT 									{$$ = create_leaf(REALLIT_NODE, $1);}
	 | Expr AND Expr 							{$$ = create_node(AND_NODE,$2,2,$1,$3);}
	 | Expr OR Expr 							{$$ = create_node(OR_NODE,$2,2,$1,$3);}
	 | Expr EQ Expr 							{$$ = create_node(EQ_NODE,$2,2,$1,$3);}
	 | Expr GEQ Expr 							{$$ = create_node(GEQ_NODE,$2,2,$1,$3);}
	 | Expr GT Expr 							{$$ = create_node(GT_NODE,$2,2,$1,$3);}
	 | Expr LEQ Expr 							{$$ = create_node(LEQ_NODE,$2,2,$1,$3);}
	 | Expr LT Expr 							{$$ = create_node(LT_NODE,$2,2,$1,$3);}
	 | Expr NEQ Expr 							{$$ = create_node(NEQ_NODE,$2,2,$1,$3);}
	 | Expr PLUS Expr    						{$$ = create_node(ADD_NODE,$2,2,$1,$3);}
	 | Expr MINUS Expr 							{$$ = create_node(SUB_NODE,$2,2,$1,$3);}
	 | Expr STAR Expr 							{$$ = create_node(MUL_NODE,$2,2,$1,$3);}
	 | Expr DIV Expr  							{$$ = create_node(DIV_NODE,$2,2,$1,$3);}
	 | Expr MOD Expr 							{$$ = create_node(MOD_NODE,$2,2,$1,$3);}
	 | OCURV error CCURV						{$$ = NULL;flag_ast_error = 1;}
	;

%%
%{
	#include "y.tab.h"
	#include "ast.h"
	#include "sym_tab.h"
	#include "intermediate_code.h"

	int flag_lex;
	int flag_meta1;
	int flag_meta2;
	int flag_meta3;
	int flag_print_ast;
	int flag_print_sym_tab;
	long int line;
	long int col;
	long int unterminated_c,unterminated_l;
	void printToken(char* text, int c); // this func prints tokens, adds col 
	void printToken2(char* text, char *token,int c); // this func prints tokens, adds col
	void create_lex_node (char* value);

%}
declit						0|[1-9]("_"*[0-9])*
realnum						[0-9]("_"*[0-9])*
exp							[eE][\+\-]?{realnum}
keywords					("++"|"--"|"null"|"abstract"|"assert"|"case"|"catch"|"char"|"const"|"enum"|"extends"|"final"|"finally"|"float"|"for"|"goto"|"import"|"implements"|"instanceof"|"interface"|"native"|"protected"|"new"|"package"|"private"|"break"|"try"|"byte"|"throw"|"long"|"native"|"throws"|"case"|"continue"|"default"|"short"|"super"|"switch"|"synchronized"|"this"|"Integer"|"System"|"transient"|"volatile"|"strictfp")
blankspace					[\t\f ]
lineterminators				\r\n|\n|\r
singleLineComment			"//"[^\n\r]*
%X comment strerror

%%
\"([^\n\r"\\]|\\[nrft"]|\\\\)*\"		{printToken2("STRLIT",yytext,yyleng);	if (!flag_meta1 ){create_lex_node(strdup(yytext)); return STRLIT;}}
\"										{BEGIN strerror;col+=yyleng;unterminated_l=line;unterminated_c=col-1;}
<strerror>\"							{BEGIN 0;col+=yyleng;}
<strerror>\\[nrft"\\]					{col+=yyleng;}
<strerror>{lineterminators}				{printf("Line %ld, col %ld: unterminated string literal\n",unterminated_l,unterminated_c);BEGIN 0;line++;col=1;}
<strerror>\\.|\\						{printf("Line %ld, col %ld: invalid escape sequence (%s)\n",line,col,yytext);col+=yyleng;}
<strerror>.								{col+=yyleng;}
<strerror><<EOF>>						{printf("Line %ld, col %ld: unterminated string literal\n",unterminated_l,unterminated_c);col+=yyleng; yyterminate(); }


boolean						{printToken( "BOOL", yyleng); 				if (!flag_meta1) return BOOL;} 
true|false					{printToken2( "BOOLLIT", yytext, yyleng);	if (!flag_meta1) {create_lex_node(strdup(yytext)); return BOOLLIT;}}
class						{printToken( "CLASS", yyleng); 				if (!flag_meta1) return CLASS;}
do							{printToken( "DO", yyleng); 				if (!flag_meta1) return DO;}
"."length					{printToken( "DOTLENGTH", yyleng); 			if (!flag_meta1){create_lex_node (NULL); return DOTLENGTH;}}
double						{printToken( "DOUBLE", yyleng);				if (!flag_meta1) return DOUBLE;}
else						{printToken( "ELSE", yyleng);				if (!flag_meta1) return ELSE;}
if 							{printToken( "IF", yyleng);					if (!flag_meta1) return IF;}
int 						{printToken( "INT", yyleng);				if (!flag_meta1) return INT;}
Integer"."parseInt			{printToken( "PARSEINT", yyleng);			if (!flag_meta1){create_lex_node (NULL); return PARSEINT;}}
System"."out"."println		{printToken( "PRINT", yyleng);				if (!flag_meta1){create_lex_node (NULL); return PRINT;}}
public						{printToken( "PUBLIC", yyleng);				if (!flag_meta1) return PUBLIC;}
return 						{printToken( "RETURN", yyleng);				if (!flag_meta1){create_lex_node (NULL);return RETURN;}}
static						{printToken( "STATIC", yyleng);				if (!flag_meta1) return STATIC;}
String						{printToken( "STRING", yyleng);				if (!flag_meta1) return STRING;}
void						{printToken( "VOID", yyleng);				if (!flag_meta1) return VOID;}
while						{printToken( "WHILE", yyleng);				if (!flag_meta1) return WHILE;}
\(							{printToken( "OCURV", yyleng);				if (!flag_meta1) return OCURV;}
")"							{printToken( "CCURV", yyleng);				if (!flag_meta1) return CCURV;}
"{"							{printToken( "OBRACE", yyleng);				if (!flag_meta1) return OBRACE;}
"}"							{printToken( "CBRACE", yyleng);				if (!flag_meta1) return CBRACE;}
"["							{printToken( "OSQUARE", yyleng);			if (!flag_meta1) return OSQUARE;}
"]"							{printToken( "CSQUARE", yyleng);			if (!flag_meta1) return CSQUARE;}
"&&"						{printToken( "AND", yyleng);				if (!flag_meta1){create_lex_node (NULL); return AND;}}
"||"						{printToken( "OR", yyleng);					if (!flag_meta1){create_lex_node (NULL); return OR;}}
"<"							{printToken( "LT", yyleng);					if (!flag_meta1){create_lex_node (NULL); return LT;}}
">"							{printToken( "GT", yyleng);					if (!flag_meta1){create_lex_node (NULL); return GT;}}
"=="						{printToken( "EQ", yyleng);					if (!flag_meta1){create_lex_node (NULL); return EQ;}}
"!="						{printToken( "NEQ", yyleng);				if (!flag_meta1){create_lex_node (NULL); return NEQ;}}
"<="						{printToken( "LEQ", yyleng);				if (!flag_meta1){create_lex_node (NULL); return LEQ;}}
">="						{printToken( "GEQ", yyleng);				if (!flag_meta1){create_lex_node (NULL); return GEQ;}}
"+"							{printToken( "PLUS", yyleng);				if (!flag_meta1){create_lex_node (NULL); return PLUS;}}
"-"							{printToken( "MINUS", yyleng);				if (!flag_meta1){create_lex_node (NULL); return MINUS;}}
"*"							{printToken( "STAR", yyleng);				if (!flag_meta1){create_lex_node (NULL); return STAR;}}
"/"							{printToken( "DIV", yyleng);				if (!flag_meta1){create_lex_node (NULL); return DIV;}}
"%"							{printToken( "MOD", yyleng);				if (!flag_meta1){create_lex_node (NULL); return MOD;}}
"!"							{printToken( "NOT", yyleng);				if (!flag_meta1){create_lex_node (NULL); return NOT;}}
"="							{printToken( "ASSIGN", yyleng);				if (!flag_meta1){create_lex_node (NULL); return ASSIGN;}}
";"							{printToken( "SEMI", yyleng);		 		if (!flag_meta1) return SEMI;}
","							{printToken( "COMMA", yyleng);				if (!flag_meta1) return COMMA;}
{keywords}					{printToken2( "RESERVED", yytext, yyleng);	if (!flag_meta1) return RESERVED;}

(({realnum}"."?{exp})|({realnum}?"."{realnum}{exp}?)|{realnum}".")		{printToken2("REALLIT",yytext, yyleng);	if (!flag_meta1){create_lex_node(strdup(yytext)); return REALLIT;}}
{declit}								{printToken2( "DECLIT",yytext, yyleng);		if (!flag_meta1 ){ create_lex_node(strdup(yytext));return DECLIT;}}
[a-zA-Z_$][a-zA-Z0-9_$]*				{printToken2( "ID", yytext, yyleng);		if (!flag_meta1 ){ create_lex_node(strdup(yytext)); return ID;}}


{blankspace}							{col++;}
{lineterminators}						{line++;col=1;}


{singleLineComment}				;
"/*"							{BEGIN comment;col+=yyleng; unterminated_l=line; unterminated_c=col-2;}
<comment>"*/"					{BEGIN 0;col+=yyleng;}
<comment>{lineterminators}		{line++;col=1;}	
<comment><<EOF>>				{printf("Line %ld, col %ld: unterminated comment\n",unterminated_l,unterminated_c);col+=yyleng;yyterminate();}
<comment>.						{col+=yyleng;}

<<EOF>>							{col+=yyleng; yyterminate();}
.								{printf("Line %ld, col %ld: illegal character (%s)\n",line,col,yytext);col+=yyleng;}

%%

int main(int argc, char * argv []){
	
	flag_lex=0;
	flag_meta1=0;
	flag_meta2=0;
	flag_meta3=0;
	flag_print_ast = 0;
	flag_ast_error =0;
	semantic_error = 0;
	flag_print_sym_tab = 0;
	line=1;
	col=1;

	if (argc==2){ // meta 1 == -1 && meta 1 == -l
		if (!strcmp(argv[1],"-1") || !strcmp(argv[1],"-l")){
			if (!strcmp(argv[1],"-l"))	// meta1 == -1 -> imprimir tokens
				flag_lex=1;
			flag_meta1 = 1;
			yylex();
			return 0;
		}

		if (!strcmp(argv[1],"-t")){ // print parse tree
			flag_print_ast = 1;
			flag_meta2 = 1;
		}
		if (!strcmp(argv[1],"-2")){
			flag_meta2 = 1;
		}

		if (!strcmp(argv[1],"-s")){ // print symb tab and annotated ast
			flag_print_sym_tab = 1;
			flag_meta3 = 1;
		}
		if (!strcmp(argv[1],"-3")){
			flag_meta3 = 1;
		}
	}
	tree = create_tree();
	yyparse();


	if (flag_print_ast ) // não imprimir a arvore caso exista um erro de sintaxe
		print_tree (tree,0);


	if ( flag_meta2 || flag_ast_error){  // print syntax errors and stop
		destroy_tree(tree);
		yylex_destroy();
		return 0;
	}

	create_sym_tab(tree); // creates sym tab and annotates tree


	if (flag_print_sym_tab){
		print_sym_tab(tab);
		print_tree (tree,0);
	}

	if (flag_meta3 || semantic_error){
		destroy_tree(tree);
		destroy_sym_tab(tab);
		yylex_destroy();
		return 0;
	}


	// DO META 4 STUFF

	initialize_counters();
	initialize_global_names();
	generate_code_global();

	destroy_tree(tree);
	destroy_sym_tab(tab);
	yylex_destroy();


	return 0; 
	
}

void yyerror (const char *s) {
	printf ("Line %ld, col %ld: %s: %s\n", line, col-yyleng, s, yytext);
	flag_ast_error = 1;
}

int yywrap(){
	return 1;
}

void printToken(char* text, int c){
	if (flag_lex){
		printf ("%s\n", text);
	}
	col+=c;
}

void printToken2(char* text, char *token,int c){
	if (flag_lex){
		printf ("%s(%s)\n", text, token);
	}
	col+=c;
}

void create_lex_node (char* value){
	lex_data * d = (lex_data*) malloc (sizeof(lex_data));
	d->line= line;
	d->col = col - yyleng;
	d->value = value;

	yylval.pos = d;
}

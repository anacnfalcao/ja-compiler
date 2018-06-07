#ifndef AST_H
#define AST_H

typedef struct  n {
	char *value;
	int type; // type of the node
	int token_type; // type of the annotated tree
	long int line;
	long int col;
	struct _sym* var_sym_tab; // points to the var in the sym tab; also used to indicate the llvm name
	char *temp_llvm_name; // if it's a temporary llvm name ---> this is used for operators, strings...
	struct n* child;
	struct n* next;
}node; // this represents an AST node

typedef struct data {
	long int line;
	long int col;
	char* value;	
}lex_data;

typedef enum {
		PROGRAM_NODE, FIELDDECL_NODE, VARDECL_NODE, METHODDECL_NODE, METHODHEADER_NODE, METHODPARAMS_NODE, PARAMDECL_NODE, METHODBODY_NODE, BLOCK_NODE, DOWHILE_NODE, IF_NODE, PRINT_NODE, RETURN_NODE, WHILE_NODE, ASSIGN_NODE, CALL_NODE, PARSEARGS_NODE, OR_NODE, AND_NODE, EQ_NODE, NEQ_NODE, LT_NODE, GT_NODE, LEQ_NODE, GEQ_NODE, ADD_NODE, SUB_NODE, MUL_NODE, DIV_NODE, MOD_NODE, NOT_NODE, MINUS_NODE, PLUS_NODE, LENGTH_NODE, BOOL_NODE, BOOLLIT_NODE, DOUBLE_NODE, DECLIT_NODE, ID_NODE, INT_NODE, REALLIT_NODE, STRLIT_NODE, VOID_NODE, STRINGARRAY_NODE 
}type_enum;


node* tree;

int flag_ast_error;


node* create_tree();

node* create_node(int type, lex_data* l, int n_children, ...);

node* create_leaf(int type, lex_data* l);

node* add_brother(node* b1, node* b2);

void destroy_tree(node* n);

node* add_type (node* n, int t1, int t2);

void transfer_data (lex_data* l, node *n);

node * add_line_col(node *n, node *s);

#endif
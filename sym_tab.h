#ifndef SYM_TAB_H
#define SYM_TAB_H

#include "ast.h"

typedef struct _sym {
	char * name;
	char * llvm_name; // llvm name of methods and vars
	int var_type; // a var or method can have types: void, int, float , bool, String[] or undef
	int node_type; // class, method, var, param
	struct _sym * scope;
	struct _sym * next;
}sym_tab;

typedef enum _var_enum {
	VOID_TYPE, INT_TYPE, DOUBLE_TYPE, BOOL_TYPE, STRINGARRAY_TYPE, STRING_TYPE, UNDEF_TYPE, CLASS_TYPE
}var_type_enum;


typedef enum _node_enum {
	CLASS_NODE, METHOD_NODE, VAR_NODE, PARAM_NODE	
}node_type_enum;


sym_tab* tab;

int semantic_error;

void create_sym_tab(node* root);

sym_tab* create_global_sym_tab (node* n);

sym_tab* create_params_sym_tab (node *n);

sym_tab* create_method_sym_tab(node* n, sym_tab * method);

sym_tab* create_node_sym_tab (char* name, int var_type, int node_type);

int search_node (sym_tab * method, char* search_name);

void destroy_sym_tab (sym_tab * s);

int get_type_tab(node* n); 

void print_sym_tab(sym_tab* root);

void print_method_tab(sym_tab* method);

void print_sym_args(sym_tab *args);

void print_tree(node* n, int n_dot);

void print_params(sym_tab * s);

void annotate_node (node * n, sym_tab * env);

void annotate_sub_tree (node * n, sym_tab * env);

int annotate_call (node *n);

sym_tab* get_next_env (sym_tab *t);

int search_var (node *n, sym_tab *env);

int numeric_operations(node *n, sym_tab * env);

int check_declit_bounds (node* n);

int check_reallit_bounds (node * n);

int check_already_defined_var (node *n, sym_tab * env);

int check_already_defined_method (sym_tab *begining, sym_tab *t, node* n);

int check_already_defined_global_var (sym_tab *begining, sym_tab * s);

int compare_methods(sym_tab * sym_method , node * node_method, char * method_id);

int isBool (node *n);

void removeChar(char *str, char garbage);

int isNumeric (node* n);

/***********************************************************************************************
									PRINT ERRORS
***********************************************************************************************/
void print_error_find_symbol(int line, int col, char *symb ); 

void print_error_find_symbol_func(int line, int col, node *n );

void print_error_incompatible_type(int line, int col, char *type, char *token);

void print_error_number_bounds (int line, int col, char* token);

void print_error_operator(int line, int col, char* token, char *type);

void print_error_operators(int line, int col,char* token, char *type1, char *type2);

void print_error_ambiguous_reference(int line, int col, node* symb);

void print_error_defined_symbol(int line, int col, char* symb);

void print_error_defined_symbol_func(int line, int col,  node *n);

#endif
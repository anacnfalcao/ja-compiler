#ifndef IT_H
#define IT_H

#include "sym_tab.h"
#include "ast.h"

typedef enum _llvm_enum {
	VOID_LLVM, INT_LLVM, DOUBLE_LLVM, BOOL_LLVM, STRINGARRAY_LLVM
}llvm_type_enum;

typedef enum arithmetic_llvm_op {
	ADD_LLVM, SUB_LLVM, MUL_LLVM, DIV_LLVM, MOD_LLVM
}arithmetic_llvm_op;

typedef enum _relational_llvm_op{
	GT_LLVM, LT_LLVM, LEQ_LLVM, GEQ_LLVM, EQ_LLVM, NEQ_LLVM
}relational_llvm_op;

typedef struct _str_node {
	char * str;
	char * llvm_name;
	int size;
	struct _str_node * next;
}str_node;

typedef struct _param_node {
	char * llvm_name;
	struct _param_node * next;
}param_node;

str_node * str_list;

long count_vars;
long count_labels;
long reserverd_label;
long count_str;
int flag_call_main;

void initialize_global_names();

void initialize_counters();

void generate_code_global();

void generate_global_var (node * n);

void generate_func (node * n);

void generate_params (sym_tab *params);

void alloca_params(sym_tab *s);

void generate_main ();

void add_str_to_list (node *n);

str_node * create_node_str (char * str);

void generate_local_var (node * n);

void print_str_list ();

void generate_func_body(node * n, sym_tab * func, int flag_dont_do_next);

void print_llvm (node * n);

void return_llvm (node * n, sym_tab *s);

void assign_llvm (node *n );

void minus_llvm (node *n);

void plus_llvm (node *n);

void reallit_llvm(node * n);

void  call_llvm (node *n);

param_node * add_node_param (char * str, param_node * head);

void length_llvm (node * n);

void parseargs_llvm (node * n);

char *get_arg(node * n, int flag_convert);

void numeric_operations_llvm (node * n, int op);

void compare_operations_llvm (node *n, int op);

void not_llvm (node * n);

void mod_llvm (node * n);

void if_llvm (node* n, sym_tab * func);

void while_llvm (node* n, sym_tab * func);

void do_while_llvm (node* n, sym_tab * func);

void or_llvm (node* n, sym_tab * func);

void and_llvm (node* n, sym_tab * func);

int token_type_to_llvm_type (node * n);

void declare_functions();


#endif
#include "sym_tab.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#define MAX_INT_JAVA 2147483648
char *var_type_str [] = {"void", "int", "double", "boolean", "String[]", "String", "undef"};

char *type_str [] = {"Program", "FieldDecl", "VarDecl", "MethodDecl", "MethodHeader", "MethodParams", "ParamDecl", "MethodBody", "Block", "DoWhile", "If", "Print", "Return", "While","Assign", "Call", "ParseArgs", "Or", "And", "Eq", "Neq", "Lt", "Gt", "Leq", "Geq", "Add", "Sub", "Mul", "Div", "Mod", "Not", "Minus", "Plus", "Length", "Bool", "BoolLit", "Double", "DecLit", "Id", "Int", "RealLit", "StrLit", "Void", "StringArray"};



void create_sym_tab(node* root){
	sym_tab* temp_sym_tab;
	node* temp_node = root->child->next;

	tab = create_global_sym_tab(root);	//inicializar tabela simbolos
	temp_sym_tab =tab->next ;


	while(temp_node != NULL && temp_sym_tab!=NULL){

		if (temp_node->type == METHODDECL_NODE && temp_sym_tab->node_type==METHOD_NODE && !compare_methods(temp_sym_tab, temp_node, temp_node->child->child->next->value)){ // method
				temp_sym_tab->scope = create_method_sym_tab(temp_node,temp_sym_tab); //criar tabela simbolos de 1 metodo
				temp_sym_tab = temp_sym_tab ->next;

		}
		if (temp_node->type == FIELDDECL_NODE && temp_sym_tab->node_type == VAR_NODE && !strcmp(temp_node->child->next->value, temp_sym_tab->name)){

			temp_sym_tab = temp_sym_tab ->next;
		}

		
		temp_node = temp_node->next;
	}

}
//adicionar a tabela de simbolos os nos das var globais + cabecalho dos metodos
sym_tab* create_global_sym_tab (node* n){
	sym_tab* class;
	sym_tab* prev;
	sym_tab* temp_sym_tab;

	node* temp_node = n->child;

	class = create_node_sym_tab (temp_node->value, CLASS_TYPE, CLASS_NODE);
	temp_sym_tab = class;
	while(temp_node != NULL){
		if (temp_node->type == FIELDDECL_NODE ){ // var
			temp_sym_tab->next = create_node_sym_tab(temp_node->child->next->value, get_type_tab(temp_node->child) ,VAR_NODE);

			if (check_already_defined_global_var(class->next, temp_sym_tab->next) ){
				print_error_defined_symbol(temp_node->child->next->line, temp_node->child->next->col, temp_node->child->next->value);
				prev = temp_sym_tab->next;
				temp_sym_tab->next =NULL;
				destroy_sym_tab(prev->scope);
				free(prev);

			}else {
				temp_sym_tab = temp_sym_tab ->next;
				temp_node->child->next->var_sym_tab = temp_sym_tab;

			}


		}else if (temp_node->type == METHODDECL_NODE){ // method
			temp_sym_tab->next = create_node_sym_tab(temp_node->child->child->next->value, get_type_tab(temp_node->child->child) ,METHOD_NODE);
			temp_sym_tab->next->scope = create_params_sym_tab(temp_node);

			if (check_already_defined_method(class->next, temp_sym_tab->next,temp_node) ){
				print_error_defined_symbol_func(temp_node->child->child->next->line, temp_node->child->child->next->col, temp_node );


				prev = temp_sym_tab->next;
				temp_sym_tab->next =NULL;
				destroy_sym_tab(prev->scope);
				free(prev);

			}else {
				temp_sym_tab = temp_sym_tab ->next;
				temp_node->child->var_sym_tab = temp_sym_tab;
			}

		}

		temp_node = temp_node->next;

	}

	return class;
}

sym_tab* create_params_sym_tab (node *n){
	sym_tab * params = NULL; //params aponta para o primeiro no do scope do metodo
	// read params
	sym_tab * temp = NULL;
	node * param_decl;
	param_decl = n->child->child->next->next->child;

	if (param_decl!=NULL){ // read the first param to inicialize params
		params = create_node_sym_tab (param_decl->child->next->value,get_type_tab(param_decl->child),PARAM_NODE);
		param_decl = param_decl->next;
	}

	temp=params;

	while (param_decl!=NULL){ // read the next params
	//	if (check_already_defined_var(param_decl->child->next,params)){
			temp->next = create_node_sym_tab (param_decl->child->next->value,get_type_tab(param_decl->child),PARAM_NODE);
			temp = temp->next;
	//	}
		param_decl = param_decl->next;
	}
	return params;
}


sym_tab* create_method_sym_tab(node* n, sym_tab * t){
	node * method_body_child;
	sym_tab * temp = t->scope;


	if (temp!=NULL) {
		while (temp->next !=NULL){
			temp = temp->next;
		}
	}

	method_body_child = n->child->next->child;
	while(method_body_child!=NULL){
		if (method_body_child->type == VARDECL_NODE && check_already_defined_var (method_body_child->child->next, t->scope)){
			if (temp == NULL){
				temp  = create_node_sym_tab (method_body_child->child->next->value,get_type_tab(method_body_child->child),VAR_NODE);
				t->scope= temp; 
			}
			else{
				temp->next = create_node_sym_tab (method_body_child->child->next->value,get_type_tab(method_body_child->child),VAR_NODE);
				temp = temp->next;
			}
			method_body_child->child->next->var_sym_tab = temp;
		}else if (method_body_child->type != VARDECL_NODE) {
			//annotate subtree handles stuff like ifs and whiles and do whiles and calls


			if (method_body_child->type == CALL_NODE ){ // avoid to annotate id if symbol not found, error would print twice
				annotate_sub_tree(method_body_child->child->next, t);
				annotate_node (method_body_child, t);	//anota arvore do proprio no
			}else if (method_body_child->type == IF_NODE || method_body_child->type == WHILE_NODE){
				if (method_body_child->child->type!=CALL_NODE)
					annotate_sub_tree(method_body_child->child->child,t);

				annotate_node(method_body_child->child, t); 

				annotate_node (method_body_child, t);	//anota arvore do proprio no

				annotate_sub_tree(method_body_child->child->next,t);
			}else{
				annotate_sub_tree (method_body_child->child, t);	//anota arvore dos filhos
				annotate_node (method_body_child, t);	//anota arvore do proprio no
			}
			
		}

		method_body_child = method_body_child -> next;
	} 

	return t->scope;
}


sym_tab* create_node_sym_tab (char* name, int var_type, int node_type){
	sym_tab * temp = (sym_tab*) malloc (sizeof(sym_tab));

	temp->name = name;
	temp->llvm_name=NULL;
	temp->var_type = var_type; 
	temp->node_type = node_type;
	temp->scope = NULL;
	temp->next = NULL;

	return temp;
}

// return 0 if not found; returns 1 if found
int search_node (sym_tab * method, char* search_name){
	sym_tab * temp;

	temp = method->scope;

	while (temp!=NULL){
		if (!strcmp(temp->name,search_name))
			return 1;

		temp = temp->next;
	}
	return 0;
}


void destroy_sym_tab (sym_tab * s){
	if (s==NULL){
		return;
	}

	if (s->llvm_name!=NULL){
		free (s->llvm_name);
	}

	destroy_sym_tab(s->scope);
	destroy_sym_tab(s->next);

	free(s);
}

// return the type that must be in the sym_tab
int get_type_tab(node* n){
	switch(n->type){
		case INT_NODE: return INT_TYPE ;
		case VOID_NODE: return VOID_TYPE;
		case DOUBLE_NODE: return DOUBLE_TYPE;
		case BOOL_NODE: return BOOL_TYPE;
		case STRINGARRAY_NODE: return STRINGARRAY_TYPE;
	}
	return -1;
}


void print_sym_tab(sym_tab* root){
	sym_tab * temp = root->next;
	printf("===== Class %s Symbol Table =====\n", root->name);
	while(temp){
		if (temp->node_type == VAR_NODE){
			printf("%s\t\t%s\n",temp->name, var_type_str[temp->var_type]);
		}else if (temp->node_type == METHOD_NODE){ 
			printf("%s\t", temp->name);
			print_sym_args(temp->scope);
			printf("\t%s\n", var_type_str[temp->var_type]);
		}
		temp = temp->next;
	}

	printf("\n");

	temp = root->next;
	while (temp){
		if (temp->node_type == METHOD_NODE){
			print_method_tab(temp);
		}
		temp = temp->next;
	}

}

void print_method_tab(sym_tab* method){
	sym_tab* temp = method->scope;
	printf("===== Method %s", method->name);
	print_sym_args(temp);
	printf(" Symbol Table =====\n");

	printf("return\t\t%s\n", var_type_str[method->var_type]);

	while(temp!= NULL){
		if (temp->name!=NULL){
			printf("%s\t\t%s",temp->name,  var_type_str[temp->var_type]);		
			if (temp->node_type == PARAM_NODE){
				printf("\tparam" );
			}
			printf("\n");
		}
		temp = temp->next;
	}


	printf("\n");
}

void print_sym_args(sym_tab *args){
	printf("(");
	if (args!=NULL && args->node_type == PARAM_NODE){
		printf("%s", var_type_str[args->var_type]);
		args= args->next;
	}
	while(args!=NULL && args->node_type == PARAM_NODE){
		printf(",%s", var_type_str[args->var_type]);
		args= args->next;
	}
	printf(")");
}


void print_tree(node* n, int n_dot){
	if (flag_ast_error == 1)
		return;

	int i;
	if (n == NULL)
		return ;

	for (i=0; i< n_dot; i++){
		printf("..");
	}
	printf("%s", type_str[n->type]);
	if (n->value!=NULL)
		printf("(%s)", n->value );
	if (  n->type == ID_NODE && n->var_sym_tab!=NULL && n->var_sym_tab->node_type == METHOD_NODE ){
		printf(" - ");
		print_params(n->var_sym_tab);
	}
	else if (n->token_type != -1){
		printf(" - %s", var_type_str[n->token_type]);
	}
	printf("\n");

	print_tree (n->child, n_dot+1);
	print_tree (n->next , n_dot);
}

void print_params(sym_tab * s){
	sym_tab * temp =  s->scope;
	printf("(");
	if (temp!=NULL && temp->node_type == PARAM_NODE){
		printf("%s",var_type_str[temp->var_type]);
		temp = temp->next;
	}
	while (temp!= NULL && temp->node_type == PARAM_NODE){
		printf(",%s",var_type_str[temp->var_type]);
		temp= temp->next;
	}

	printf(")");
}

void annotate_node (node * n, sym_tab * env){
	switch (n->type){
		case ID_NODE:
			n->token_type = search_var(n, env->scope);
			break; 

		case BOOLLIT_NODE: 
			n->token_type = BOOL_TYPE;
			break;
		case DECLIT_NODE:
			n->token_type = check_declit_bounds(n);
			break;
		case REALLIT_NODE:
			n->token_type = check_reallit_bounds(n);
			break;
		case STRLIT_NODE:
			n->token_type = STRING_TYPE;
			break;

		case PLUS_NODE:
		case MINUS_NODE:
			if (n->child->token_type!=INT_TYPE && n->child->token_type!=DOUBLE_TYPE){
				if (n->type== PLUS_NODE)
					print_error_operator(n->line, n->col,"+" , var_type_str[n->child->token_type]);
				else
					print_error_operator(n->line, n->col,"-" , var_type_str[n->child->token_type]);
				n->token_type = UNDEF_TYPE;
			}else {
				n->token_type = n->child->token_type;
			}
			break;

		case MOD_NODE:
		case MUL_NODE:
		case DIV_NODE:
		case SUB_NODE:
		case ADD_NODE:
			n->token_type = numeric_operations(n, env->scope);
			break;
		case ASSIGN_NODE:
			if (!(  (n->child->token_type==DOUBLE_TYPE && isNumeric(n->child->next)) || (isBool(n->child) && isBool(n->child->next)) || (n->child->token_type == INT_TYPE && n->child->next->token_type == INT_TYPE) )){
				print_error_operators(n->line, n->col , "=",var_type_str[n->child->token_type] ,var_type_str[n->child->next->token_type]);
			
			}
			n->token_type = n->child->token_type;
			break;
		case NOT_NODE:
			if (!isBool(n->child)){
				print_error_operator(n->line, n->col, "!", var_type_str[n->child->token_type]);
			}
			n->token_type = BOOL_TYPE;
			break;
		case AND_NODE:
		case OR_NODE:
			if (!isBool(n->child) || !isBool(n->child->next) ){
				if (n->type == AND_NODE)
					print_error_operators(n->line, n->col, "&&", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				else
					print_error_operators(n->line, n->col, "||", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				
			}
			n->token_type = BOOL_TYPE;
			break;

		case EQ_NODE:
		case NEQ_NODE:
			if (!(  (isNumeric(n->child) && isNumeric(n->child->next))||(isBool(n->child) && isBool(n->child->next))   )){
				if (n->type == EQ_NODE )
					print_error_operators(n->line, n->col, "==", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				else
					print_error_operators(n->line, n->col, "!=", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				
			}
			n->token_type = BOOL_TYPE;
			break;

		case GT_NODE:
		case GEQ_NODE:
		case LEQ_NODE:
		case LT_NODE:
			if (!(isNumeric(n->child) && isNumeric(n->child->next) )){
				if (n->type == GT_NODE )
					print_error_operators(n->line, n->col, ">", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				else if (n->type == GEQ_NODE)
					print_error_operators(n->line, n->col, ">=", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				else if (n->type == LEQ_NODE)
					print_error_operators(n->line, n->col, "<=", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);
				else
					print_error_operators(n->line, n->col, "<", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);

			}

			n->token_type = BOOL_TYPE;
			
			break;

		case LENGTH_NODE: 
			if (n->child->token_type != STRINGARRAY_TYPE){
				print_error_operator(n->child->line, n->child->col + strlen(n->child->value), ".length", var_type_str[n->child->token_type]);
			}
			n->token_type = INT_TYPE;
			break;
		case PARSEARGS_NODE:
			if (!(n->child->token_type == STRINGARRAY_TYPE && n->child->next->token_type == INT_TYPE))
				print_error_operators(n->line, n->col , "Integer.parseInt", var_type_str[n->child->token_type], var_type_str[n->child->next->token_type]);

			n->token_type = INT_TYPE;
			break;

		case CALL_NODE:
			n->token_type = annotate_call (n) ;
			break;

		case IF_NODE:
			if (!isBool(n->child)){
				print_error_incompatible_type(n->child->line, n->child->col, var_type_str[n->child->token_type], "if");
			}
			break;
		case DOWHILE_NODE:
			if (!isBool(n->child->next)){
				print_error_incompatible_type(n->child->next->line, n->child->next->col, var_type_str[n->child->next->token_type], "do");
			}
			break;
		case WHILE_NODE:
			if (!isBool(n->child)){
				print_error_incompatible_type(n->child->line, n->child->col, var_type_str[n->child->token_type], "while");
			}
			break;
		case RETURN_NODE:
			if (env->var_type==VOID_TYPE){
				if (n->child!=NULL){
					print_error_incompatible_type(n->child->line, n->child->col, var_type_str[n->child->token_type], "return");
				}
			}else{
				if (n->child==NULL){
					print_error_incompatible_type(n->line, n->col, var_type_str[VOID_TYPE], "return");
				}else{
					if (!(     (isNumeric(n->child) && env->var_type==DOUBLE_TYPE )|| (isBool(n->child) && env->var_type ==BOOL_TYPE) || (n->child->token_type == INT_TYPE && env->var_type == INT_TYPE )   )){
						print_error_incompatible_type(n->child->line, n->child->col, var_type_str[n->child->token_type], "return");
					}
				}

			}

			break;
		case PRINT_NODE:
			if (n->child->token_type==VOID_TYPE || n->child->token_type==UNDEF_TYPE || n->child->token_type == STRINGARRAY_TYPE){
				print_error_incompatible_type(n->child->line, n->child->col, var_type_str[n->child->token_type], "System.out.println");
			}
			break;
		}
}

// 0 -> methods are equal ; 1 -> methods are not equal
int compare_methods(sym_tab * sym_method , node * node_method, char * method_id){
	sym_tab * sym_param;
	node * node_param;
	int is_equals;

	
	if (!strcmp(sym_method->name, method_id )){
		// compare params
		sym_param = sym_method->scope;
		node_param = node_method->child->child->next->next->child;
		is_equals=1;
		while (sym_param !=NULL && node_param!=NULL){
			if (sym_param->var_type != get_type_tab(node_param->child)){
				is_equals=0;
				break;
			}

			node_param = node_param->next;
			sym_param = sym_param->next;

			if (sym_param!=NULL && sym_param->node_type!=PARAM_NODE){
				sym_param=NULL;
			}

		}
		if (sym_param==NULL && node_param==NULL && is_equals){
			return 0; //  equals 
		}
	}

	return 1; // not equals
}




void annotate_sub_tree (node * n, sym_tab * env){
	if (n==NULL)
		return;
	if (n->type == CALL_NODE){
		annotate_sub_tree(n->child->next,env);
		annotate_node(n,env);
	}else if (n->type == IF_NODE || n->type == WHILE_NODE){
		if (n->child->type!=CALL_NODE)
			annotate_sub_tree(n->child->child,env);// condition
		annotate_node(n->child, env);	// if/while statement

		annotate_node (n, env);	//anota arvore do proprio no

		annotate_sub_tree(n->child->next,env); // body

	}else{
		annotate_sub_tree(n->child, env);
		annotate_node(n,env);
	}
	
	annotate_sub_tree(n->next, env);
}


int annotate_call (node *n){
	node *method_id = n->child;
	sym_tab *tab_temp = tab->next;
	sym_tab * prev_fit=NULL;

	sym_tab * tab_param;
	node *node_param;

	int is_perfect_fit;//matching of method name and params
	int is_fit;
	int number_fits; 

	number_fits = 0;

	while (tab_temp!=NULL){
		if (tab_temp->node_type == METHOD_NODE && !strcmp(tab_temp->name, method_id ->value)){
			// compare params
			node_param = method_id ->next;
			tab_param = tab_temp->scope;
			is_perfect_fit = 1; // if both are null, its a perfect fit 
			is_fit=0;

			if (tab_param!=NULL && tab_param->node_type != PARAM_NODE)
				tab_param = NULL;
			while (node_param!=NULL && tab_param !=NULL){
				if (node_param->token_type == tab_param->var_type){
					is_perfect_fit = 1; 
				}else if (node_param->token_type == INT_TYPE && tab_param->var_type == DOUBLE_TYPE){
					is_fit = 1;
					is_perfect_fit = 0;
				}else{
					is_perfect_fit = 0;
					is_fit = 0;
					break;
				}

				node_param = node_param->next;
				tab_param = tab_param->next;
				
				if (tab_param !=NULL && tab_param->node_type != PARAM_NODE)
					tab_param =NULL;
				
			}

			if (tab_param == NULL && node_param == NULL && is_perfect_fit && !is_fit){
				method_id->var_sym_tab = tab_temp;
				return tab_temp->var_type;
			}
		
			if (tab_param == NULL && node_param == NULL && is_fit){
				prev_fit = tab_temp;
				number_fits++;
			}

		}


		tab_temp = tab_temp -> next;
	}

	if (number_fits==1){
		method_id->var_sym_tab = prev_fit;
		return prev_fit->var_type;
	}else if (number_fits >1){
		method_id->token_type = UNDEF_TYPE;
		print_error_ambiguous_reference(n->child->line, n->child->col, n);
	}else{
		method_id->token_type= UNDEF_TYPE;
		print_error_find_symbol_func (n->child->line, n->child->col, n->child);
	}
	// ambiguous
	
	return UNDEF_TYPE;
}


int numeric_operations(node *n, sym_tab * env){
	node *child1, *child2;
	child1= n->child;
	child2= child1->next;

	
	// decide the var type
	if (!isNumeric(child1)|| !isNumeric(child2)){
		
		if (n->type==ADD_NODE) 
			print_error_operators(n->line, n->col, "+",var_type_str[child1->token_type], var_type_str[child2->token_type]);
		else if (n->type==SUB_NODE) 
			print_error_operators(n->line, n->col, "-",var_type_str[child1->token_type], var_type_str[child2->token_type]);
		else if (n->type==MUL_NODE) 
			print_error_operators(n->line, n->col, "*",var_type_str[child1->token_type], var_type_str[child2->token_type]);
		else if (n->type== DIV_NODE) 
			print_error_operators(n->line, n->col, "/",var_type_str[child1->token_type], var_type_str[child2->token_type]);
		else if (n->type==MOD_NODE) 
			print_error_operators(n->line, n->col, "%",var_type_str[child1->token_type], var_type_str[child2->token_type]);
		return UNDEF_TYPE;
	}
	else if (child1->token_type == DOUBLE_TYPE || child2->token_type == DOUBLE_TYPE)
		return DOUBLE_TYPE;
	else
		return INT_TYPE;
}


int search_var (node * n, sym_tab *env){
	sym_tab * temp = env;
	char * name_var = n->value;

	// search for var in local env
	while (temp!=NULL){
		if (temp->name!=NULL && strcmp(temp->name, name_var)==0){
			n->var_sym_tab = temp;
			return temp->var_type;
		}
		temp = temp->next;
	}

	
	//search for var in global env
	temp = tab->next;
	while (temp!=NULL){
		if (temp->node_type!= METHOD_NODE && (strcmp(temp->name, name_var)==0)){
			n->var_sym_tab = temp;
			return temp->var_type;
		}
		temp = temp->next;
	}


	// print error and return 
	print_error_find_symbol(n->line,n->col,n->value);
	return UNDEF_TYPE;
}




/********************************************************************************************
										Check Errors 
*********************************************************************************************/


int isNumeric (node* n){
	if (n->token_type == INT_TYPE || n->token_type == DOUBLE_TYPE){
		return 1;
	}
	return 0;
}
int isBool (node *n){
	if (n->token_type== BOOL_TYPE){
		return 1;
	}
	return 0;
}

/**
	This piece of code is not ours
	from: http://stackoverflow.com/questions/5457608/how-to-remove-the-character-at-a-given-index-from-a-string-in-c

**/
void removeChar(char *str, char garbage) {
    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) 
        	dst++;
    }
    *dst = '\0';
}
/****/


int check_declit_bounds (node* n){
	long long int temp;
	char* rmv_underscore;

	rmv_underscore = (char*)strdup(n->value);

	removeChar(rmv_underscore, '_');

	if(strlen(rmv_underscore) > 10){
		print_error_number_bounds(n->line, n->col, n->value);
		free(rmv_underscore);
		return INT_TYPE;
	}

	free(rmv_underscore);

	temp = atoll(n->value);

	if (temp >= MAX_INT_JAVA){
		print_error_number_bounds(n->line, n->col, n->value);
	}
	return INT_TYPE;
}

int check_reallit_bounds (node * n){
	double d;
	char* rmv_underscore;

	rmv_underscore = (char*)strdup(n->value);

	removeChar(rmv_underscore, '_');

	d = atof (rmv_underscore);

	free(rmv_underscore);

	if (isinf(d)){
		print_error_number_bounds (n->line, n->col, n->value);
		return DOUBLE_TYPE;
	}

	if (d == 0.0){ // floating point overflow
		for (int i = 0; i < strlen(n->value); i++){
			if (isdigit(n->value[i]) && n->value[i]!='0'){
				print_error_number_bounds (n->line, n->col, n->value);
				break;
			}
			if (n->value[i]=='e' || n->value[i]=='E')
				break;
		}
	}

	return DOUBLE_TYPE;
}

int check_already_defined_var (node *n, sym_tab * env){
	while (env!=NULL){
		if (env->node_type!=METHOD_NODE && env->name!=NULL && env->name!=NULL && strcmp(env->name, n->value)==0 ){
			print_error_defined_symbol(n->line, n->col, n->value);
			return 0;
		}
		env = env->next;
	}
	return 1;
}

// n -> vardecls
int check_already_defined_method (sym_tab *begining, sym_tab *t, node* n){
	sym_tab * beg_params;
	sym_tab * t_params;
	int is_equals;
	// check params
	sym_tab * temp_params = t->scope;
	node* temp_node = n->child->child->next->next->child;

	
	while (temp_params!=NULL && temp_node!=NULL && temp_params->node_type == PARAM_NODE){

		beg_params=t->scope;

		while (beg_params!=NULL && beg_params!=temp_params && beg_params->node_type == PARAM_NODE){

			if (beg_params->name!= NULL && !strcmp(beg_params->name, temp_params->name)){
				print_error_defined_symbol(temp_node->child->next->line,temp_node->child->next->col,temp_node->child->next->value);
				temp_params->name = NULL;
				break;
			}
			beg_params=beg_params->next;
		}
		temp_node=temp_node->next;
		temp_params=temp_params->next;
	}


	// check method
	while(begining!=NULL && begining!=t){
		if (begining->node_type == METHOD_NODE && !strcmp(begining->name, t->name)){
			// compare params
			beg_params = begining->scope;
			t_params = t->scope;
			is_equals=1;
			while (beg_params!=NULL && t_params !=NULL){
				if (beg_params->node_type !=PARAM_NODE){
					beg_params = NULL;
					break;
				} else if (t_params->node_type != PARAM_NODE){
					t_params = NULL;
					break;
				}

				if (beg_params->var_type != t_params->var_type){
					is_equals=0;
					break;
				}

				beg_params= beg_params->next;
				t_params=t_params->next;
			}
			if (beg_params==NULL && t_params==NULL && is_equals){ 
					return 1;
			}
		}

		begining = begining->next;
	}

	return 0;
}

// returns 1 if the global var is already defined
int check_already_defined_global_var (sym_tab *begining, sym_tab * s){
	while(begining!=NULL && begining!=s){
		if (begining->node_type == VAR_NODE && !strcmp (begining->name,s->name))
			return 1;
		begining = begining->next;
	}
	return 0;

}

/***********************************************************************************************
									PRINT ERRORS
***********************************************************************************************/

void print_error_find_symbol(int line, int col, char *symb ){
	semantic_error = 1;
	printf("Line %d, col %d: Cannot find symbol %s\n",line, col, symb);
}
void print_error_find_symbol_func(int line, int col, node *n ){
	semantic_error = 1;
	printf("Line %d, col %d: Cannot find symbol %s(",line, col, n->value);
	n=n->next;
	if (n!=NULL){
		printf("%s", var_type_str[n->token_type]);
		n=n->next;
	}
	while (n!=NULL){
		printf(",%s", var_type_str[n->token_type] );
		n = n->next;
	}
	printf(")\n");
}

void print_error_incompatible_type(int line, int col, char *type, char *token){
	semantic_error = 1;
	printf("Line %d, col %d: Incompatible type %s in %s statement\n",line, col, type, token);
}

void print_error_number_bounds (int line, int col, char* token){
	semantic_error = 1;
	printf("Line %d, col %d: Number %s out of bounds\n",line, col, token);
}

void print_error_operator(int line, int col, char* token, char *type){
	semantic_error = 1;
	printf("Line %d, col %d: Operator %s cannot be applied to type %s\n",line, col, token, type);
}

void print_error_operators(int line, int col,char* token, char *type1, char *type2){
	semantic_error = 1;
	printf("Line %d, col %d: Operator %s cannot be applied to types %s, %s\n",line, col, token, type1, type2);
}

void print_error_ambiguous_reference(int line, int col, node* symb){
	semantic_error = 1;
	printf("Line %d, col %d: Reference to method %s(",line, col, symb->child->value);
	symb = symb->child->next;
	if (symb!=NULL){
		printf("%s",var_type_str[symb->token_type]);
		symb=symb->next;
	}
	while (symb!=NULL){
		printf(",%s",var_type_str[symb->token_type] );
		symb=symb->next;
	}

	printf(") is ambiguous\n");
}

void print_error_defined_symbol(int line, int col, char* symb){
	semantic_error = 1;
	printf("Line %d, col %d: Symbol %s already defined\n",line, col, symb);
}


void print_error_defined_symbol_func(int line, int col, node *n){
	semantic_error = 1;
	printf("Line %d, col %d: Symbol %s(",line, col, n->child->child->next->value);
	n=n->child->child->next->next->child;
	if (n!=NULL){
		printf("%s", var_type_str[get_type_tab(n->child)]);
		n=n->next;
	}
	while (n!=NULL){
		printf(",%s", var_type_str[ get_type_tab(n->child)] );
		n = n->next;
	}
	printf(") already defined\n");
}

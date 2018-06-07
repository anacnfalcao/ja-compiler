#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include "ast.h"


node* create_tree(){
	return NULL;
}

// https://www.tutorialspoint.com/c_standard_library/c_macro_va_arg.htm
node* create_node(int type, lex_data* l, int n_children, ...){
	int i; 
	va_list args;
	node *first_child = NULL;
	node *temp;


	va_start(args, n_children);
	
	if (flag_ast_error == 1){
		if (l!=NULL)
			free(l);
		if (n_children!=0){
			first_child = va_arg(args, node*);
			if (first_child!=NULL)
				free(first_child);
			for(i = 1; i < n_children; i++){
				temp = va_arg(args, node*);
				if (temp!=NULL)
					free(temp);
			}
		}
		return NULL;
	}

	node* new = (node*) malloc(sizeof(node));

	if (n_children!=0){
		first_child = va_arg(args, node*);

		for(i = 1; i < n_children; i++){
			temp = va_arg(args, node*);
			add_brother(first_child, temp);
		}
	}

	new->type = type;
	new->child = first_child;
	new->next = NULL;
	new->value = NULL;
	new->var_sym_tab = NULL;
	new->temp_llvm_name = NULL;
	new->token_type = -1;
	transfer_data(l, new);

	if (new->type==CALL_NODE){
		new->col = new->child->col;
		new->line = new->child->line;
	}

	return new;
}
node* create_leaf(int type, lex_data* l){
	if (flag_ast_error == 1){
		if (l!=NULL && l->value!= NULL){
			free (l->value);
		}
		if (l!=NULL)
			free(l);
		return NULL;
	}

	node* new = (node*) malloc(sizeof(node));

	new->type = type;
	new->child = NULL;
	new->next = NULL;
	new->token_type = -1;
	new->var_sym_tab = NULL;
	new->temp_llvm_name = NULL;
	transfer_data(l, new);

	return new;
}

node* add_brother(node* b1, node* b2){
	if (flag_ast_error == 1){
		destroy_tree(b1);
		destroy_tree(b2);
		return NULL;
	}
	if (b1==NULL){
		return b2;
	}

	node* temp = b1;
	while (temp->next!= NULL){
		temp=temp->next;
	}
	temp->next = b2;

	return b1;
}

void destroy_tree(node* n){ 
	if (n == NULL)
		return ; 

	destroy_tree (n->child);
	destroy_tree (n->next);

	if (n->temp_llvm_name != NULL && n->var_sym_tab == NULL) 
		free (n->temp_llvm_name);

	if (n->value != NULL)
		free (n->value);
	free(n);
}

// swap types
node* add_type (node* n, int t1, int t2){
	if (flag_ast_error == 1)
		return NULL;
	node* temp = n;
	while (temp!= NULL){
		if (temp->child != NULL && temp->child->type == t1 ){
			temp->child->type = t2;
		}
		temp = temp->next;
	}

	return n;
}


void transfer_data (lex_data* l, node *n){
	if (l==NULL){
		n->value=NULL;
		n->line=0;
		n->value=0;
		return; 
	}
	n->line = l->line;
	n->col = l->col;
	n->value= l->value;

	free(l);
}



node * add_line_col(node *n, node *s){
	printf("%ld %ld\n", s->line, s->col );
	n->line = s->line;
	n->col = s->col;

	return n;
}
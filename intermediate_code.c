#include "intermediate_code.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#define MAIN_JAVA "@main_java"

char *type_str_llvm [] = {"void","i32","double", "i1", "i32 %argc, i8** %argv"};
char *arithmetic_op_str_llvm [] = {"add","sub","mul", "div", "rem"};
char *int_rlt_op_str_llvm[] = {"icmp sgt", "icmp slt", "icmp sle", "icmp sge", "icmp eq", "icmp ne"};
char *double_rlt_op_str_llvm[] = {"fcmp ogt", "fcmp olt", "fcmp ole", "fcmp oge", "fcmp oeq", "fcmp une"};


void initialize_global_names(){
	char *class_name = (char*) malloc (sizeof(char)* (strlen(tab->name)+2) );

	strcpy (class_name, tab->name);
	strcat(class_name, ".");

	sym_tab *temp = tab->next; // first var or func

	char buffer_count_vars [20];

	while (temp!=NULL){
		if (temp->node_type == METHOD_NODE && !strcmp(temp->name,"main") && temp->scope->node_type==PARAM_NODE && temp->scope->var_type == STRINGARRAY_TYPE ){
			temp->llvm_name = (char*) malloc (sizeof(char) * (strlen(MAIN_JAVA) +1 ));
			strcpy(temp->llvm_name, MAIN_JAVA);
			flag_call_main = 1;
		}else {
			sprintf(buffer_count_vars, "%ld", count_vars);
			temp->llvm_name = (char*) malloc (sizeof(char) * (strlen(class_name) + strlen(temp->name) + strlen(buffer_count_vars) + strlen("@") + 1));
			strcpy(temp->llvm_name, "@");
			strcat(temp->llvm_name, class_name);
			
			strcat(temp->llvm_name, buffer_count_vars);
			strcat(temp->llvm_name, temp->name);
		}
		count_vars++;
		temp = temp->next;
	}

	free(class_name);
}

void initialize_counters(){
	count_vars = 1;
	count_labels = 0;
	count_str = 1;
	flag_call_main = 0;
}

void generate_code_global(){
	node * n;

	n = tree->child->next;
	str_list = NULL;

	while (n!=NULL){
		if (n->type == FIELDDECL_NODE){
			generate_global_var (n->child->next);
		}else {
			generate_func(n);
		}
		n = n->next;
	}
	generate_main();

	print_str_list();

	declare_functions();
}

void generate_global_var (node * n){
	if (n->var_sym_tab!=NULL){
		if (n->var_sym_tab->var_type  == DOUBLE_LLVM ){
			printf("%s = global %s 0.0\n", n->var_sym_tab->llvm_name, type_str_llvm[n->var_sym_tab->var_type] );
		}else{
			printf("%s = global %s 0\n", n->var_sym_tab->llvm_name, type_str_llvm[n->var_sym_tab->var_type] );
		}
	}
}

void generate_func (node * n){
	sym_tab * func = n->child->var_sym_tab;
	printf("define %s %s", type_str_llvm[func->var_type], func->llvm_name);
	generate_params(func->scope);

	printf("{\n");

	//printf("l.0:\n");

	alloca_params(func);

	count_vars=1;
	count_labels=0;
	reserverd_label = 0;
	generate_func_body(n->child->next->child, func,0);

	if (func->var_type==VOID_TYPE){
		printf("ret void\n");
	}else if (func->var_type == INT_TYPE){
		printf("ret i32 0\n");
	}else if (func->var_type == DOUBLE_TYPE){
		printf("ret double 0.0\n");
	}else if (func->var_type == BOOL_TYPE){
		printf("ret i1 1\n");
	}

	printf("}\n");
}

void generate_params (sym_tab *params){
	printf("(");

	sym_tab *temp = params;

	while (temp!= NULL && temp->node_type==PARAM_NODE){
		if (temp->var_type!=STRINGARRAY_TYPE)
			printf("%s %%%s", type_str_llvm[temp->var_type], temp->name );
		else
			printf("%s", type_str_llvm[temp->var_type]);
		temp = temp->next;
		if (temp!=NULL && temp->node_type==PARAM_NODE)
			printf(", ");
	}
	printf(")");
}



void alloca_params(sym_tab *s){
	char *str_name = s->name;
	s = s->scope;
	while (s!=NULL && s->node_type==PARAM_NODE){
		if (s->var_type!= STRINGARRAY_TYPE){
			s->llvm_name = (char*) malloc (sizeof(char)*(strlen(str_name) + strlen(s->name) + 3));
			sprintf(s->llvm_name, "%%%s.%s" , str_name, s->name);
			printf("%s = alloca %s\n", s->llvm_name, type_str_llvm[s->var_type]);
			printf("store %s %%%s, %s* %s\n", type_str_llvm[s->var_type], s->name, type_str_llvm[s->var_type] ,s->llvm_name);
			
		}
		s = s->next;
	}

}

void generate_main (){
	printf("define i32 @main(i32 %%argc, i8** %%argv) {\n" );
	if (flag_call_main )
		printf("\tcall void %s(i32 %%argc, i8** %%argv)\n",MAIN_JAVA);
	printf("\tret i32 0\n" );
	printf("}\n");
}


str_node * create_node_str (char * str){
	char buffer_count_vars [20];
	sprintf(buffer_count_vars, "%ld", count_str);
	char * llvm_name  = (char*) malloc (sizeof(char) * (strlen("@str.") + strlen(buffer_count_vars) +1 ));
	llvm_name= strcpy(llvm_name, "@str.");
	strcat(llvm_name, buffer_count_vars);
	count_str++;

	str_node * n = (str_node *) malloc (sizeof(str_node));	
	n->llvm_name = llvm_name;
	
	n->next = NULL;


	n->str = str;
	int count_char_format; //  char format: backslash (\) 
	count_char_format = 0;
	int i;
	for (i = 0 ; i < strlen(str); i++){
		if (str[i]=='\\'){ 
			count_char_format ++;
			i++; // not count the second backslash (\) 
		}if (str[i]=='%'){ // %%
			count_char_format--;
		}
	}
	n->size = strlen(str) - count_char_format;

	return n;
}

void add_str_to_list (node *n){
	str_node * n_str;
 	
 	n_str = create_node_str (n->value);
 	n->temp_llvm_name  = n_str->llvm_name;
	

	// add to the beggining of the list
	n_str->next= str_list;
	str_list = n_str;

}

void print_str_list (){
	str_node * temp, * next;
	temp = str_list;
	while(temp!=NULL){
		next = temp->next;
		printf("%s = constant [ %d x i8] c\"",temp->llvm_name, temp->size );
		for (int i = 0; i < strlen(temp->str); i++){
			if (temp->str[i]=='\\'){
				if (temp->str[i+1]=='"'){
					printf("\\%02X", '"');
				}else if  (temp->str[i+1]=='n'){
					printf("\\%02X", '\n');
				}else if (temp->str[i+1]=='t'){
					printf("\\%02X", '\t');
				}else if (temp->str[i+1]=='\\'){
					printf("\\%02X", '\\');
				}else if (temp->str[i+1]=='f'){
					printf("\\%02X", '\f');
				}else if (temp->str[i+1]=='r'){
					printf("\\%02X", '\r');
				}

				i++;
			}else if (temp->str[i]!='"'){
				if (temp->str[i]=='%'){
					printf("%%%%");
				}else if (!isalpha(temp->str[i]) && !isdigit(temp->str[i]) && temp->str[i]!=' ' ){
					printf("\\%02X", temp->str[i]);
				}else
					printf("%c",temp->str[i] );
				
			}
			
		}
		printf("\\0A\\00");
		printf("\"\n");
		free(temp);
		temp = next;
	}
	printf("@str.int =  constant [4 x i8] c\"%%d\\0A\\00\"\n");
	printf("@str.double =  constant [7 x i8] c\"%%.16E\\0A\\00\"\n");
	printf("@str.true =  constant [6 x i8] c\"true\\0A\\00\"\n");
	printf("@str.false =  constant [7 x i8] c\"false\\0A\\00\"\n"); 

}

void generate_func_body(node * n, sym_tab * func, int flag_dont_do_next){
	if (n == NULL)
		return;

	if (n->type != IF_NODE && n->type != WHILE_NODE && n->type != DOWHILE_NODE && n->type!= OR_NODE && n->type!=AND_NODE)
		generate_func_body(n->child, func, 0);

	switch (n->type){
		case VARDECL_NODE: generate_local_var(n); break;
		case PRINT_NODE: print_llvm (n); break;
		case RETURN_NODE: return_llvm(n,func); break;
		case ASSIGN_NODE: assign_llvm(n); break;
		case PLUS_NODE: plus_llvm(n); break;
		case MINUS_NODE: minus_llvm(n); break;
		case REALLIT_NODE: reallit_llvm(n);
		case DECLIT_NODE: removeChar(n->value, '_');break;
		case CALL_NODE: call_llvm (n); break;
		case LENGTH_NODE: length_llvm(n); break;
		case PARSEARGS_NODE: parseargs_llvm (n); break;
		case ADD_NODE: numeric_operations_llvm(n,ADD_LLVM); break;
		case SUB_NODE: numeric_operations_llvm(n,SUB_LLVM); break;
		case MUL_NODE: numeric_operations_llvm(n,MUL_LLVM); break;
		case DIV_NODE: numeric_operations_llvm(n,DIV_LLVM); break;
		case LT_NODE: compare_operations_llvm(n,LT_LLVM); break;
		case GT_NODE: compare_operations_llvm(n,GT_LLVM); break;
		case EQ_NODE: compare_operations_llvm(n,EQ_LLVM); break;
		case NEQ_NODE: compare_operations_llvm(n,NEQ_LLVM); break;
		case LEQ_NODE: compare_operations_llvm(n,LEQ_LLVM); break;
		case GEQ_NODE: compare_operations_llvm(n,GEQ_LLVM); break;
		case NOT_NODE: not_llvm(n); break;
		case MOD_NODE: mod_llvm(n); break;
		case IF_NODE: if_llvm (n, func); break;
		case WHILE_NODE: while_llvm(n,func); break;
		case DOWHILE_NODE: do_while_llvm(n,func);break;
		case OR_NODE: or_llvm(n,func); break;
		case AND_NODE: and_llvm(n,func); break;
		case BOOLLIT_NODE: 
			if (!strcmp(n->value, "true")){
				strcpy(n->value, "1");
			}else{
				strcpy(n->value, "0");
			}
			break;
		case ID_NODE: 
			if (n->var_sym_tab !=NULL && n->var_sym_tab->llvm_name != NULL){
				if (n->temp_llvm_name!=NULL){
					free(n->temp_llvm_name);
				}
				n-> temp_llvm_name = strdup(n->var_sym_tab->llvm_name); 
				
			}
			break;

	}
	if (!flag_dont_do_next)
		generate_func_body(n->next, func,0);
}



void generate_local_var (node * n){
	char *llvm_name = (char* ) malloc (sizeof(char*) * (strlen("%.")+ strlen(n->child->next->value) + 1 ));
	strcpy(llvm_name,"%.");
	strcat(llvm_name,n->child->next->value);

	n->child->next->temp_llvm_name = llvm_name;
	n->child->next->var_sym_tab->llvm_name = llvm_name;

	printf("%s = alloca %s\n", llvm_name, type_str_llvm[n->child->next->var_sym_tab->var_type]);
} 

void print_llvm (node * n){
	char * str;
	if (n->child->type == STRLIT_NODE){
		add_str_to_list (n->child);
		printf("%%%ld = call i32 (i8*, ...) @printf(i8* getelementptr ([%d x i8], [%d x i8]* %s, i32 0, i32 0))\n",count_vars, str_list->size,str_list->size, n->child->temp_llvm_name);
		count_vars++;
	}else if (n->child->token_type == INT_TYPE){
		str = get_arg(n->child,0);
		printf("%%%ld = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @str.int, i32 0, i32 0), i32 %s)\n", count_vars,str);
		count_vars++;
		free(str);
	}else if (n->child->token_type == DOUBLE_TYPE){
		str = get_arg(n->child,0);
		printf("%%%ld = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @str.double, i32 0, i32 0),  double %s)\n", count_vars,str);
		count_vars++;
		free(str);
	}else{ // BOOLIT
		str = get_arg(n->child,0);
		printf("call void @print_boolean(i1 %s)\n",str);
		free(str);
		//}
	}
}

void return_llvm (node * n, sym_tab *s){
	char * str;
	if (n->child == NULL){
		printf("ret void\n");
	} else{ 
		if (s->var_type== DOUBLE_TYPE){
			str = get_arg (n->child,1);
		}else{
			str = get_arg (n->child,0);
		}
		printf("ret %s %s\n", type_str_llvm[s->var_type], str);
		free(str);
	}
	count_vars++;
}

void assign_llvm (node *n ){
	int type_llvm;
	char *str2;
	if (n->child->token_type == BOOL_TYPE || n->child->token_type == n->child->next->token_type) {
		str2 = get_arg(n->child->next, 0);
		type_llvm = token_type_to_llvm_type(n->child);
		printf("store %s %s, %s* %s\n", type_str_llvm[type_llvm], str2 ,type_str_llvm[type_llvm], n->child->temp_llvm_name);
		free(str2);

	} else { // double  = int
		str2 = get_arg(n->child->next, 1);
		printf("store double %s, double* %s\n", str2, n->child->temp_llvm_name);
		free(str2);
		
	}
	n->var_sym_tab = n->child->var_sym_tab;
	n->temp_llvm_name = strdup(n->child->temp_llvm_name);
}

void minus_llvm (node *n){
	int type_llvm = token_type_to_llvm_type (n->child);
	if (n->child->temp_llvm_name ==NULL){
		if (n->child->token_type == INT_TYPE)
			printf("%%%ld = sub %s 0, %s \n", count_vars, type_str_llvm[type_llvm], n->child->value);
		else
			printf("%%%ld = fsub %s -0.0, %s \n", count_vars, type_str_llvm[type_llvm] , n->child->value);

	} else if (n->child->var_sym_tab!=NULL){
		printf("%%%ld = load %s, %s* %s\n",count_vars, type_str_llvm[type_llvm], type_str_llvm[type_llvm], n->child->temp_llvm_name);
		count_vars++;
		if (n->child->token_type == INT_TYPE)
			printf("%%%ld = sub %s 0, %%%ld \n", count_vars, type_str_llvm[type_llvm] ,count_vars -1);
		else
			printf("%%%ld = fsub %s -0.0, %%%ld \n", count_vars, type_str_llvm[type_llvm] ,count_vars -1);
	}else{
		if (n->child->token_type == INT_TYPE)
			printf("%%%ld = sub %s 0, %s \n", count_vars, type_str_llvm[type_llvm], n->child->temp_llvm_name);
		else
			printf("%%%ld = fsub %s -0.0, %s \n", count_vars, type_str_llvm[type_llvm] , n->child->temp_llvm_name);

	}
	char buffer_count_vars [20];
	sprintf(buffer_count_vars, "%%%ld", count_vars);
	n->temp_llvm_name = (char*) malloc (sizeof(char*) * (strlen(buffer_count_vars)+1));
	strcpy(n->temp_llvm_name,buffer_count_vars); 

	count_vars++;
}

void plus_llvm (node *n){
	if (n->child->temp_llvm_name!=NULL){
		n->temp_llvm_name = (char*) malloc (sizeof(char*) * (strlen(n->child->temp_llvm_name)+1 ));
		strcpy(n->temp_llvm_name, n->child->temp_llvm_name);
		n->var_sym_tab=n->child->var_sym_tab;
		
	}
	if (n->child->value!=NULL){
		n->value = (char*) malloc (sizeof(char*) * (strlen(n->child->value)+1 ));
		strcpy(n->value,n->child->value);
	}
}

void reallit_llvm(node * n) {
	double d; int size;
	removeChar(n->value, '_');
	d = atof (n->value);
	size = strlen(n->value);
	free(n->value);
	n->value = (char* ) malloc (sizeof(char) * (18 + size + 2));
	sprintf(n->value, "%.16E",d);
}

void  call_llvm (node *n){
	sym_tab * params_sym_tab = n->child->var_sym_tab->scope;
	node * params_ast = n ->child->next;
	char buffer_count_vars [20];
	param_node* head, *temp;
	head = NULL;

	char * str;
	while (params_ast!=NULL && params_ast->token_type!=STRINGARRAY_TYPE){
		if (params_sym_tab->var_type==DOUBLE_TYPE){
			str = get_arg (params_ast,1);
		}else{
			str = get_arg (params_ast,0);
		}

		head = add_node_param(str, head);

		params_sym_tab = params_sym_tab ->next;
		params_ast = params_ast -> next;
	}
	params_sym_tab = n->child->var_sym_tab->scope;
	if (n->token_type!= VOID_TYPE){
		printf("%%%ld = call %s %s (", count_vars, type_str_llvm[token_type_to_llvm_type(n)], n->child->var_sym_tab->llvm_name);
		sprintf(buffer_count_vars, "%%%ld", count_vars);
		n->temp_llvm_name= strdup( buffer_count_vars);
		count_vars++;
	}
	else{
		printf("call void %s (", n->child->var_sym_tab->llvm_name);
	}

	if (n->child->next!= NULL && n ->child->next->token_type == STRINGARRAY_TYPE){
		printf("i32 %%argc, i8** %%argv");
	}else{
		while(head!=NULL){
			printf(" %s %s",  type_str_llvm[params_sym_tab->var_type], head->llvm_name );

			temp = head;
			head = head -> next;

			free(temp->llvm_name);
			free(temp);
			params_sym_tab = params_sym_tab ->next;

			if (head!=NULL){
				printf(",");
			}
		}
	}

	printf(")\n");

}

param_node * create_node_param(char * str){
	param_node* p = (param_node*) malloc (sizeof(param_node));
	p->llvm_name = strdup(str);
	p->next = NULL;
	return p;
}

param_node * add_node_param (char * str, param_node * head){
	if (head == NULL){
		return create_node_param(str);
	}
	param_node * temp = head;
	while (temp->next!=NULL){
		temp = temp -> next;
	}
	temp->next  = create_node_param(str);
	return head;
}

void parseargs_llvm (node * n){
	if (n->child->next->temp_llvm_name == NULL){
		printf("%%%ld = getelementptr inbounds i8*, i8** %%argv, i32 %d\n",count_vars, atoi(n->child->next->value)+1 );
		count_vars++;
	}else{
		if (n->child->next->var_sym_tab!=NULL){
			printf("%%%ld = load i32, i32* %s\n",count_vars,n->child->next->temp_llvm_name);
			count_vars++;
			printf("%%%ld = add i32 %%%ld, 1\n", count_vars, count_vars-1);
			count_vars++;
		}else{
			printf("%%%ld = add i32 %s, 1\n", count_vars,n->child->next->temp_llvm_name);
			count_vars++;
		}

		printf("%%%ld = getelementptr inbounds i8*, i8** %%argv, i32 %%%ld\n",count_vars, count_vars -1);
		count_vars++;		
	}
	printf("%%%ld = load i8*, i8 ** %%%ld\n",count_vars, count_vars-1 );
	count_vars++;
	printf("%%%ld = call i32 @atoi(i8* %%%ld)\n", count_vars, count_vars -1);
	char buffer_count_vars [20];
	sprintf(buffer_count_vars, "%%%ld", count_vars);
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;
}

void length_llvm (node * n){
	char buffer_count_vars [20];
	printf("%%%ld = sub i32 %%argc, 1\n",count_vars/*, count_vars -1*/);
	sprintf(buffer_count_vars, "%%%ld", count_vars);
	n->temp_llvm_name = strdup(buffer_count_vars);
	count_vars++;
}

// warning: what returns from this string needs to be freed
char * get_arg(node * n, int flag_convert){
	char buffer_count_vars[30];
	if ((flag_convert == 0) || (flag_convert==1 && n->token_type==DOUBLE_TYPE) ){ // no conversion
		if (n->var_sym_tab != NULL){ // var
			int type_llvm = token_type_to_llvm_type (n);
			printf("%%%ld = load %s, %s* %s\n",count_vars, type_str_llvm[type_llvm],type_str_llvm[type_llvm], n->temp_llvm_name );
			sprintf(buffer_count_vars, "%%%ld",count_vars);
			count_vars++;
			return strdup(buffer_count_vars);
		}else if (n->value!=NULL){
			return strdup(n->value);
		}else{
			return strdup(n->temp_llvm_name);
		}
	}else{ // need for conversion to double
		if (n->var_sym_tab != NULL){ // var
			printf("%%%ld = load i32, i32* %s\n",count_vars, n->temp_llvm_name );
			count_vars++;
			printf("%%%ld = sitofp i32 %%%ld to double\n",count_vars, count_vars-1 );
			sprintf(buffer_count_vars, "%%%ld",count_vars);
			count_vars++;
			return strdup(buffer_count_vars);
		}else if (n->value!=NULL){
			sprintf(buffer_count_vars, "%s.0",n->value);
			return strdup(buffer_count_vars);
		}else{
		 	printf("%%%ld = sitofp i32 %s to double\n", count_vars,  n->temp_llvm_name);
		 	sprintf(buffer_count_vars, "%%%ld",count_vars);
			count_vars++;
			return strdup(buffer_count_vars);
		}
	}
}

void numeric_operations_llvm (node * n, int op){
	char * arg1, * arg2;
	char buffer_count_vars[20];
	if (n->child->token_type == INT_TYPE && n->child->next->token_type == INT_TYPE){
		arg1 = get_arg(n->child, 0);
		arg2 = get_arg(n->child->next, 0);
		if (op!=DIV_LLVM)
			printf("%%%ld = %s %s %s, %s\n", count_vars, arithmetic_op_str_llvm[op], type_str_llvm[token_type_to_llvm_type(n->child)], arg1, arg2 );
		else
			printf("%%%ld = s%s %s %s, %s\n", count_vars, arithmetic_op_str_llvm[op], type_str_llvm[token_type_to_llvm_type(n->child)], arg1, arg2 );
		
	}else{
		arg1 = get_arg(n->child, 1);
		arg2 = get_arg(n->child->next, 1);
		printf("%%%ld = f%s double %s, %s\n", count_vars, arithmetic_op_str_llvm[op], arg1, arg2 );
	}
	sprintf(buffer_count_vars, "%%%ld",count_vars);	
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;
	free(arg1);
	free(arg2);
}

void compare_operations_llvm (node *n, int op){
	char * arg1, * arg2;
	char buffer_count_vars[20];
	if (n->child->token_type == BOOL_TYPE){ // BOOLEANS
		arg1 = get_arg(n->child, 0);
		arg2 = get_arg(n->child->next, 0);
		// convert i1 to i32 
		printf("%%%ld = zext i1 %s to i32\n", count_vars, arg1);
		count_vars++;
		printf("%%%ld = zext i1 %s to i32\n", count_vars, arg2);
		count_vars++;

		printf("%%%ld =%s i32 %%%ld, %%%ld\n", count_vars, int_rlt_op_str_llvm[op], count_vars -1, count_vars-2);
	}else if (n->child->token_type == DOUBLE_TYPE || n->child->next->token_type == DOUBLE_TYPE){ // DOUBLES
		arg1 = get_arg(n->child, 1);
		arg2 = get_arg(n->child->next, 1);
		printf("%%%ld = %s double %s, %s\n", count_vars, double_rlt_op_str_llvm[op], arg1, arg2 );
	}else{ // INTs
		arg1 = get_arg(n->child, 0);
		arg2 = get_arg(n->child->next, 0);
		// fica a faltar converter os args para int
		printf("%%%ld = %s i32 %s, %s\n", count_vars, int_rlt_op_str_llvm[op], arg1, arg2 );

	}
	
	sprintf(buffer_count_vars, "%%%ld",count_vars);	
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;
	free(arg1);
	free(arg2);
}


void not_llvm (node * n){
	char buffer_count_vars[20];
	char *str = get_arg(n->child, 0);
	printf("%%%ld = xor i1 %s, true\n",count_vars, str );

	sprintf(buffer_count_vars, "%%%ld",count_vars);	
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;
	free(str);

}

void mod_llvm (node * n){
	char buffer_count_vars[20];
	char * arg1, *arg2;
	if (n->child->token_type == INT_TYPE && n->child->next->token_type == INT_TYPE){
		arg1 = get_arg(n->child, 0);
		arg2 = get_arg(n->child->next, 0);
		printf("%%%ld = srem i32 %s, %s\n", count_vars, arg1, arg2);

	}else{
		arg1 = get_arg(n->child, 1);
		arg2 = get_arg(n->child->next, 1);
		printf("%%%ld = call double @fmod (double %s, double %s)\n", count_vars, arg1, arg2);

	}

	sprintf(buffer_count_vars, "%%%ld",count_vars);	
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;
	free(arg1);
	free(arg2);
}

void if_llvm (node* n, sym_tab * func){
	generate_func_body(n->child,func,1);
	char* str = get_arg(n->child,0);
	long label_n = count_labels;
	count_labels++;

	printf("br i1 %s, label %%if.%ld, label %%else.%ld\n",str, label_n,label_n );

	printf("if.%ld:\n", label_n );			
	generate_func_body(n->child->next,func,1);
	printf("br label %%out.%ld\n", label_n);

	printf("else.%ld:\n",label_n); 
	generate_func_body(n->child->next->next,func,1);
	printf("br label %%out.%ld\n", label_n);

	printf("out.%ld:\n",label_n); 

	free(str);
}

void while_llvm (node* n, sym_tab * func){
	long label_n = count_labels;
	count_labels++;
	printf("br label %%while_cnd.%ld \n", label_n );

	printf("while_cnd.%ld:\n", label_n);
	generate_func_body(n->child,func,1);
	char* str = get_arg(n->child,0);
	printf("br i1 %s, label %%while_body.%ld, label %%out.%ld\n",str, label_n,label_n );

	printf("while_body.%ld:\n", label_n);
	generate_func_body(n->child->next,func,1);
	printf("br label %%while_cnd.%ld \n", label_n);

	printf("out.%ld:\n",label_n); 

	free(str);
}

void do_while_llvm (node* n, sym_tab * func){
	long label_n = count_labels;
	count_labels++;

	printf("br label %%do_bdy.%ld \n", label_n );

	printf("do_bdy.%ld:\n", label_n);
	generate_func_body(n->child,func,1);
	printf("br label %%do_cnd.%ld \n", label_n);

	printf("do_cnd.%ld:\n", label_n);
	generate_func_body(n->child->next,func,1);
	char* str = get_arg(n->child->next,0);
	printf("br i1 %s, label %%do_bdy.%ld, label %%out.%ld\n",str, label_n,label_n );

	printf("out.%ld:\n",label_n); 

	free(str);

}

void or_llvm (node* n, sym_tab * func){
	long local = reserverd_label;
	reserverd_label+=3;

	char * cnd1, *cnd2;

	printf("br label %%l.%ld\n", local );

	printf("l.%ld:\n",local );
	count_labels=local;


	generate_func_body(n->child,func,1);
	cnd1 = get_arg(n->child,0);
	printf("br i1 %s, label %%l.%ld, label %%l.%ld \n",cnd1,local+2, local + 1 );

	long a  = count_labels;


	printf("l.%ld:\n",local + 1);
	count_labels=local+1;

	generate_func_body(n->child->next,func,1);
	long b = count_labels;
	cnd2 = get_arg(n->child->next,0);
	printf("br label %%l.%ld\n",local + 2);


	printf("l.%ld:\n",local+2 );
	count_labels=local+2;
	printf("%%%ld = phi i1 [true, %%l.%ld], [%s,%%l.%ld] \n",count_vars, a, cnd2,b );
	
	char buffer_count_vars[20];
	sprintf(buffer_count_vars, "%%%ld",count_vars);	
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;


	free(cnd1);
	free(cnd2);

}	

void and_llvm (node* n, sym_tab * func){
	long local = reserverd_label;
	reserverd_label+=3;

	char * cnd1, *cnd2;

	printf("br label %%l.%ld\n", local );

	printf("l.%ld:\n",local );
	count_labels=local;


	generate_func_body(n->child,func,1);
	cnd1 = get_arg(n->child,0);
	printf("br i1 %s, label %%l.%ld, label %%l.%ld \n",cnd1, local + 1 ,local+2);

	long a  = count_labels;


	printf("l.%ld:\n",local + 1);
	count_labels=local+1;

	generate_func_body(n->child->next,func,1);
	long b = count_labels;
	cnd2 = get_arg(n->child->next,0);
	printf("br label %%l.%ld\n",local + 2);


	printf("l.%ld:\n",local+2 );
	count_labels=local+2;
	printf("%%%ld = phi i1 [false, %%l.%ld], [%s,%%l.%ld] \n",count_vars, a, cnd2,b );
	
	char buffer_count_vars[20];
	sprintf(buffer_count_vars, "%%%ld",count_vars);	
	n->temp_llvm_name = strdup (buffer_count_vars);
	count_vars++;


	free(cnd1);
	free(cnd2);
}	



int token_type_to_llvm_type (node * n){
	switch (n->token_type){
		case INT_TYPE: return INT_LLVM;
		case DOUBLE_TYPE: return DOUBLE_LLVM;
		case BOOL_TYPE: return BOOL_LLVM;
	}
	return UNDEF_TYPE;
}


void declare_functions(){
	// LLVM function to decide the value of boolean string
	printf("define void @print_boolean(i1 %%x){\n" );
	printf("\t%%1 = alloca i1" );
	printf("\tstore i1 %%x, i1* %%1\n" );
	printf("\t%%2 = load i1, i1* %%1\n");
	printf("\t%%3 = icmp eq i1 %%2, 0\n");
	printf("\tbr i1 %%3, label %%4, label %%6\n");
	printf("\t%%5 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([7 x i8], [7 x i8]* @str.false, i32 0, i32 0))\n");
	printf("\tbr label %%8\n");
	printf("\t%%7 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([6 x i8], [6 x i8]* @str.true, i32 0, i32 0))\n");
	printf("\tbr label %%8\n");
	printf("\tret void\n");
	printf("}\n");


	printf("declare i32 @printf(i8*, ...)\n");

	printf("declare i32 @atoi(i8*) \n");

	printf("declare double @fmod(double, double) \n");

}

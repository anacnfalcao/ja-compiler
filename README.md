# ja-compiler V3.0
A Ja compiler created using C, lex and yacc that produces LLVM assembly language. Ja is a small version of the Java 8 language.

Ja Compiler was made by me and Ana Madeira as an academic project for the Compilers class of University of Coimbra. 

## 1 - Getting started
 1. Clone this repository into your computer.
 2. Make sure you have a C compiler installed as well as Lex, Yacc and LLVM.
 3. Compile the application using the following command: `lex jac.l; yacc -d jac.y; cc -o jac ast.c y.tab.c lex.yy.c -ll -ly`
 5. Write your Ja program in a file, let's call it `HelloWorld.ja`.
 4. Compile your program using the Ja Compiler to obtain a LLVM assembly language file: `./jac < HelloWorld.ja > out.ll`.
 5. Execute your program using the following command: `llc-3.8 out.ll;clang-3.8 -o out out.s;./out`

## 2 - What is Ja?
Ja is a small subset of the Java 8 language that only accepts a single class program with mutiple public static programs.

### 2.1 - What does a JA program looks like?
The following code is a simple program in Ja that calculates the greatest common divisor(gcd) between two numbers given as the program's arguments.
```
class gcd2 {
	public static int gcd;
	public static void main(String[] args) {
		int a, b;
		if (args.length >= 2) {
			a = Integer.parseInt(args[0]);
			b = Integer.parseInt(args[1]);
			gcd = gcd(a, b);
			System.out.println(gcd);
		} else
			System.out.println("Error: two parameters required.");
	}
	public static int gcd(int a, int b) {
		if (a == 0)
			return b;
		else {
			while (b > 0)
				if (a > b)
					a = a-b;
				else
					b = b-a;
				return a;
		}
	}
}
```

### 2.2 -Accepted tokens
Tokens that are accepted in Ja language:
```
boolean
true
false
class
do
.length
double
else
if
int
Integer.parseInt
System.out.println
public
return
static
String
void
while
(
)
{
}
[
]
&&
||
>
<
==
!=
<=
>=
+
-
*
/
!
%
=
;
,
```
### 2.2 - Reserved keywords
```
++
--
null
Integer
System
```
### 2.3 - Ja Grammar in EBNF notation
```
Program → CLASS ID OBRACE { FieldDecl | MethodDecl | SEMI } CBRACE
FieldDecl → PUBLIC STATIC Type ID { COMMA ID } SEMI
MethodDecl → PUBLIC STATIC MethodHeader MethodBody
MethodHeader → ( Type | VOID ) ID OCURV [ FormalParams ] CCURV
MethodBody → OBRACE { VarDecl | Statement } CBRACE
FormalParams → Type ID { COMMA Type ID }
FormalParams → STRING OSQUARE CSQUARE ID
VarDecl → Type ID { COMMA ID } SEMI
Type → BOOL | INT | DOUBLE
Statement → OBRACE { Statement } CBRACE
Statement → IF OCURV Expr CCURV Statement [ ELSE Statement ]
Statement → WHILE OCURV Expr CCURV Statement
Statement → DO Statement WHILE OCURV Expr CCURV SEMI
Statement → PRINT OCURV ( Expr | STRLIT ) CCURV SEMI
Statement → [ ( Assignment | MethodInvocation | ParseArgs ) ] SEMI
Statement → RETURN [ Expr ] SEMI
Assignment → ID ASSIGN Expr
MethodInvocation → ID OCURV [ Expr { COMMA Expr } ] CCURV
ParseArgs → PARSEINT OCURV ID OSQUARE Expr CSQUARE CCURV
Expr → Assignment | MethodInvocation | ParseArgs
Expr → Expr ( AND | OR ) Expr
Expr → Expr ( EQ | GEQ | GT | LEQ | LT | NEQ ) Expr
Expr → Expr ( PLUS | MINUS | STAR | DIV | MOD ) Expr
Expr → ( PLUS | MINUS | NOT ) Expr
Expr → ID [ DOTLENGTH ]
Expr → OCURV Expr CCURV
Expr → BOOLLIT | DECLIT | REALLIT
```


## 3 - Compiling and Running 
### 3.1 - Compiling
```
lex jac.l
yacc -d jac.y
cc -o jac ast.c y.tab.c lex.yy.c -ll -ly
```
### 3.2 - Using the Ja Compiler
Considering your program file is called `HelloWorld.ja`, you can execute jac using:
```
./jac < HelloWorld.ja > out.ll
```
#### 3.2.1 - Jac Flags
There are mutiple flags you can use to obtain different outputs that correspond to different compiling phases. If no flag is used, jac will produce LLVM  assembly code that can be executed with llvm.
Flags:
```
 -l			- produces a sequence of read tokens
 -1			- produces a sequence of read tokens
 -2			- produces an abstract syntax tree
 -t			- produces an abstract syntax tree with annotations
 -s			- produces a symbol table containg the methods and variables
```
Using a flag:
```
./java -l < HelloWorld.ja
```

### 3.3 - Executing your code with LLVM
```
llc-3.8 out.ll
clang-3.8 -o out out.s
./out
```

## 4 - Thanks
A special thanks to my coworker Ana Madeira, ofcourse, and our teacher, Carlos Fonseca, for teaching us the Compilers art.
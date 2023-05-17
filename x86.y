%{
  #include <math.h>
  #include <stdio.h>
  int yylex (void);
  int yyerror (char *);
%}

%token ADD SUB MUL DIV JCC MOV LEA JUMP CALL PUSH POP NOP

%%

instruction: single_instruction 
    | double_instruction 
    | triple_instruction
;

single_instruction: add { $$ = $1 }
    | sub { $$ = $1 }
    | MUL { $$ = $1 }
    | DIV { $$ = $1 }
    | JCC { $$ = $1 }
    | MOV { $$ = $1 }
    | LEA { $$ = $1 }
    | JUMP { $$ = $1 }
    | CALL { $$ = $1 }
    | PUSH { $$ = $1 }
    | POP { $$ = $1 }
    | garbage
;

double_instruction: single_instruction ' ' single_instruction
;

triple_instruction: single_instruction ' ' double_instruction
    | single_instruction ' ' single_instruction ' ' single_instruction
    | double_instruction ' ' single_instruction
;

add: ADD '-' sub
    | ADD
    | ADD garbage;

sub: SUB '-' add
    | MUL '-' sub '-' DIV
    | DIV '-' add '-' MUL
    | SUB
    | SUB garbage;

garbage: garbage '-' NOP
    | NOP
;

%%

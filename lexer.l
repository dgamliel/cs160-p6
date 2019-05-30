%option yylineno
%pointer

%{
    #include <cstdlib>
    #include <cerrno>
    #include <climits>
    #include <limits>
    #include "ast.hpp"
    #include "parser.hpp"
    
	void yyerror(const char *);
%}

/* WRITEME: Copy any definitions and start conditions from Project 5 here. */
%x comment
%%

  /* WRITEME: Copy your Flex rules and actions from Project 5 here. */
"/*"                    BEGIN(comment);
<comment>[^*\n]*        ;
<comment>"*"+[^*/\n]*   ;
<comment>\n             { }
<comment><<EOF>>        { yyerror("dangling comment"); }
<comment>"*"+"/"        BEGIN(INITIAL);

"+"               { return T_PLUS; }
"-"               { return T_MINUS; }
"*"               { return T_MULTIPLY; }
"/"               { return T_DIVIDE; }
"or"              { return T_OR; }
"and"             { return T_AND; }
">"               { return T_GREAT; }
">="              { return T_GREATEQ; }
"="               { return T_EQ; }
"equals"          { return T_EQUALS; }
"not"             { return T_NOT; }

"extends"         { return T_EXTENDS; }
"true"            { yylval.integer_ptr = new IntegerNode(1); return T_TRUE; }
"false"           { yylval.integer_ptr = new IntegerNode(0); return T_FALSE; }
"if"              { return T_IF; }
"else"            { return T_ELSE; }
"while"           { return T_WHILE; }
"do"              { return T_DO; }
"print"           { return T_PRINT; }
"return"          { return T_RETURN; }

"."               { return T_PERIOD; }
","               { return T_COMMA; }
";"               { return T_SEMICOLON; }
"("               { return T_OPENPAREN; }
")"               { return T_CLOSEPAREN; }
"{"               { return T_OPENBRACE; }
"}"               { return T_CLOSEBRACE; }
"->"              { return T_LAMBDA; }

"new"             { return T_NEW; }
"none"            { return T_NONE; }
"integer"         { return T_INTEGER; }
"boolean"         { return T_BOOLEAN; }

[a-zA-Z][a-zA-Z0-9]*  { yylval.identifier_ptr = new IdentifierNode(yytext); return T_IDENT; }
"0"|[1-9][0-9]*       { yylval.integer_ptr = new IntegerNode(atoi(yytext)); return T_LITERAL; }

[ \t\v\f\r][ \t\v\f\r]*      ;
\n                { }

.                 { yyerror("invalid character"); }

%%

int yywrap(void) {
  return 1;
}

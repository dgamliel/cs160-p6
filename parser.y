%{
    #include <cstdlib>
    #include <cstdio>
    #include <iostream>
    #include "ast.hpp"
    
    #define YYDEBUG 1
    #define YYINITDEPTH 10000
    
    int yylex(void);
    void yyerror(const char *);
    
    extern ASTNode* astRoot;
%}

%error-verbose
// %glr-parser
/* NOTE: You may use the %glr-parser directive, which may allow your parser to
         work even with some shift/reduce conflicts remianing. */

/* WRITEME: Copy your token and precedence specifiers from Project 5 here. */
%token T_OPENPAREN T_CLOSEPAREN T_OPENBRACE T_CLOSEBRACE
%token T_INTEGER T_BOOLEAN T_NEW T_NONE
%token T_PERIOD T_COMMA T_SEMICOLON
%token T_IF T_ELSE T_WHILE T_DO
%token T_TRUE T_FALSE
%token T_LAMBDA
%token T_EXTENDS
%token T_PRINT
%token T_RETURN

%token T_IDENT
%token T_LITERAL

%token T_OR
%token T_AND
%token T_GREAT T_GREATEQ T_EQ T_EQUALS
%token T_PLUS T_MINUS
%token T_MULTIPLY T_DIVIDE
%token T_NOT T_UNARYMINUS

%left T_OR
%left T_AND
%left T_GREAT T_GREATEQ T_EQUALS
%left T_PLUS T_MINUS
%left T_MULTIPLY T_DIVIDE
%precedence T_NOT T_UNARYMINUS

/* WRITEME: Copy your type specifiers from Project 5 here. */
%type <program_ptr> Start
%type <class_list_ptr> ClassList
%type <class_ptr> Class

%type <identifier_ptr> T_IDENT
%type <declaration_list_ptr> Members DeclarationList
%type <method_list_ptr> Methods
%type <method_ptr> MethodsP
%type <parameter_list_ptr> ParameterList ParametersP

%type <type_ptr> ReturnType Type
%type <methodbody_ptr> Body
%type <statement_list_ptr> Statements Block
%type <returnstatement_ptr> Return
%type <parameter_ptr> Parameters
%type <declaration_ptr> MembersP Declarations
%type <identifier_list_ptr> MembersPP DeclarationsP DeclarationsPP 

%type <expression_ptr> Expr
%type <assignment_ptr> Assignment
%type <call_ptr> MethodCallExpr
%type <methodcall_ptr> MethodCall
%type <ifelse_ptr> IfElse
%type <while_ptr> WhileLoop
%type <dowhile_ptr> DoWhile
%type <print_ptr> Print

%type <expression_list_ptr> Arguments ArgumentsP
%type <integer_ptr> T_LITERAL T_TRUE T_FALSE

%type <statement_ptr> StatementsP 

%%

/* WRITEME: This rule is a placeholder. Replace it with your grammar
            rules and actions from Project 5. */

Start : ClassList                                                             { $$ = new ProgramNode($1); astRoot = $$; }
      ;

ClassList : Class ClassList                                                   { $$ = $2; $$->push_front($1); }
          | Class                                                             { $$ = new std::list<ClassNode*>(); $$->push_front($1); }
          ;

Class : T_IDENT T_OPENBRACE Members Methods T_CLOSEBRACE                      { $$ = new ClassNode($1, NULL, $3, $4); }
      | T_IDENT T_EXTENDS T_IDENT T_OPENBRACE Members Methods T_CLOSEBRACE    { $$ = new ClassNode($1, $3, $5, $6); }
      ;

Members : Members MembersP                                                    { $$ = $1; $$->push_back($2); }
        | %empty                                                              { $$ = new std::list<DeclarationNode*>(); }          
        ;

MembersP : Type MembersPP T_SEMICOLON                                         { $$ = new DeclarationNode($1, $2); }                                  
         ;

MembersPP : T_IDENT                                                           { $$ = new std::list<IdentifierNode*>(); $$->push_back($1); }
          ;
          
Methods : MethodsP Methods                                                    { $$ = $2; $$->push_front($1); }
        | %empty                                                              { $$ = new std::list<MethodNode*>(); }
        ;

MethodsP : T_IDENT T_OPENPAREN ParameterList T_CLOSEPAREN T_LAMBDA ReturnType T_OPENBRACE Body T_CLOSEBRACE   { $$ = new MethodNode($1, $3, $6, $8); }
         ;

ParameterList : Parameters ParametersP                              { $$ = $2; $$->push_front($1); }
              | %empty                                              { $$ = new std::list<ParameterNode*>(); }
              ;

Parameters : Type T_IDENT                                           { $$ = new ParameterNode($1, $2); }
            ;

ParametersP : T_COMMA Parameters ParametersP                        { $$ = $3; $$->push_front($2); } 
           | %empty                                                 { $$ = new std::list<ParameterNode*>(); }
           ;

Body : DeclarationList Statements Return                            { $$ = new MethodBodyNode($1, $2, $3); }
     | DeclarationList Statements                                   { $$ = new MethodBodyNode($1, $2, NULL); }
     ;

Return : T_RETURN Expr T_SEMICOLON                                  { $$ = new ReturnStatementNode($2); }
       ;

DeclarationList : DeclarationList Declarations                      { $$ = $1; $$->push_back($2); }
                | %empty                                            { $$ = new std::list<DeclarationNode*>(); }
                ;

Declarations : Type DeclarationsP T_SEMICOLON                       { $$ = new DeclarationNode($1, $2); }
              ;

DeclarationsP : T_IDENT DeclarationsPP                              { $$ = $2; $$->push_front($1); }
              ;

DeclarationsPP : T_COMMA T_IDENT DeclarationsPP                     { $$ = $3; $$->push_front($2); }
               | %empty                                             { $$ = new std::list<IdentifierNode*>(); }
               ;

Statements : StatementsP Statements                                 { $$ = $2; $$->push_front($1);  }         
           | %empty                                                 { $$ = new std::list<StatementNode*>(); }
           ;

StatementsP : Assignment        { $$ = $1; }
            | MethodCallExpr    { $$ = $1; }
            | IfElse            { $$ = $1; }
            | WhileLoop         { $$ = $1; }
            | DoWhile           { $$ = $1; }
            | Print             { $$ = $1; }
            ;

Assignment : T_IDENT T_EQ Expr T_SEMICOLON                          { $$ = new AssignmentNode($1, NULL, $3); }
           | T_IDENT T_PERIOD T_IDENT T_EQ Expr T_SEMICOLON         { $$ = new AssignmentNode($1, $3, $5); }
           ;

MethodCallExpr : MethodCall T_SEMICOLON                             { $$ = new CallNode($1); }
               ;

IfElse : T_IF Expr T_OPENBRACE Block T_CLOSEBRACE                                                 { $$ = new IfElseNode($2, $4, NULL); }
       | T_IF Expr T_OPENBRACE Block T_CLOSEBRACE T_ELSE T_OPENBRACE Block T_CLOSEBRACE           { $$ = new IfElseNode($2, $4, $8); }
       ;

WhileLoop : T_WHILE Expr T_OPENBRACE Block T_CLOSEBRACE                                           { $$ = new WhileNode($2, $4); }
          ;

DoWhile : T_DO T_OPENBRACE Block T_CLOSEBRACE T_WHILE T_OPENPAREN Expr T_CLOSEPAREN T_SEMICOLON   { $$ = new DoWhileNode($3, $7); }
        ;

Print : T_PRINT Expr T_SEMICOLON                  { $$ = new PrintNode($2); }
      ;

Block : StatementsP Block                         { $$ = $2; $$->push_front($1); }
      | StatementsP                               { $$ = new std::list<StatementNode*>(); $$->push_front($1); }
      ;

Expr : Expr T_PLUS Expr                           { $$ = new PlusNode($1, $3); }
     | Expr T_MINUS Expr                          { $$ = new MinusNode($1, $3); }
     | Expr T_MULTIPLY Expr                       { $$ = new TimesNode($1, $3); }
     | Expr T_DIVIDE Expr                         { $$ = new DivideNode($1, $3); }
     | Expr T_GREAT Expr                          { $$ = new GreaterNode($1, $3); }
     | Expr T_GREATEQ Expr                        { $$ = new GreaterEqualNode($1, $3); }
     | Expr T_EQUALS Expr                         { $$ = new EqualNode($1, $3); }
     | Expr T_AND Expr                            { $$ = new AndNode($1, $3); }
     | Expr T_OR Expr                             { $$ = new OrNode($1, $3); }
     | T_NOT Expr                                 { $$ = new NotNode($2); }
     | T_MINUS Expr %prec T_UNARYMINUS            { $$ = new NegationNode($2); }
     | T_IDENT                                    { $$ = new VariableNode($1); }
     | T_IDENT T_PERIOD T_IDENT                   { $$ = new MemberAccessNode($1, $3); }
     | MethodCall                                 { $$ = $1; }
     | T_OPENPAREN Expr T_CLOSEPAREN              { $$ = $2; }
     | T_LITERAL                                  { $$ = new IntegerLiteralNode($1); }
     | T_TRUE                                     { $$ = new BooleanLiteralNode($1); }
     | T_FALSE                                    { $$ = new BooleanLiteralNode($1); }
     | T_NEW T_IDENT                              { $$ = new NewNode($2, NULL); }            
     | T_NEW T_IDENT T_OPENPAREN Arguments T_CLOSEPAREN                       { $$ = new NewNode($2, $4); }
     ;

MethodCall : T_IDENT T_OPENPAREN Arguments T_CLOSEPAREN                       { $$ = new MethodCallNode($1, NULL, $3); }
           | T_IDENT T_PERIOD T_IDENT T_OPENPAREN Arguments T_CLOSEPAREN      { $$ = new MethodCallNode($1, $3, $5); }
           ;

Arguments : ArgumentsP                            { $$ = $1; }
          | %empty                                { $$ = new std::list<ExpressionNode*>(); }
          ;

ArgumentsP : ArgumentsP T_COMMA Expr              { $$ = $1; $$->push_back($3);}
           | Expr                                 { $$ = new std::list<ExpressionNode*>(); $$->push_back($1); }
           ;

ReturnType : Type                                 { $$ = $1; }
           | T_NONE                               { $$ = new NoneNode(); }
           ;

Type : T_INTEGER                                  { $$ = new IntegerTypeNode(); }
     | T_BOOLEAN                                  { $$ = new BooleanTypeNode(); }
     | T_IDENT                                    { $$ = new ObjectTypeNode($1); }
     ;

%%

extern int yylineno;

void yyerror(const char *s) {
  fprintf(stderr, "%s at line %d\n", s, yylineno);
  exit(1);
}
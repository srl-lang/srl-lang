# SRL (Serial Run Language) - Formal EBNF Grammar Specification

This document defines the normative **Extended Backus-Naur Form (EBNF)** syntax grammar for SRL v0.2.0. Compiler implementers, parser developers, and IDE tooling authors should use this grammar as the authoritative source for syntax validation.

```ebnf
(* ============================================================================ *)
(* SRL (Serial Run Language) Formal EBNF Syntax Grammar (v0.2.1)               *)
(* ============================================================================ *)

Program         ::= { Statement } ;

Statement       ::= VarDecl
                  | FnDecl
                  | StructDecl
                  | InterfaceDecl
                  | EnumDecl
                  | IfStmt
                  | WhileStmt
                  | ForStmt
                  | ReturnStmt
                  | TryCatchStmt
                  | MatchStmt
                  | ImportStmt
                  | ExprStmt ;

VarDecl         ::= [ "const" ] "var" Identifier [ ":" Type ] [ "=" Expression ] ";" ;
FnDecl          ::= [ "constexpr" ] "fn" Identifier [ "<" TypeParams ">" ] "(" [ Parameters ] ")" [ "->" Type ] Block ;
StructDecl      ::= "struct" Identifier "{" [ Identifiers ] "}" ;
InterfaceDecl   ::= "interface" Identifier "{" { FnSignature } "}" ;
EnumDecl        ::= "enum" Identifier "{" Identifiers "}" ;

IfStmt          ::= "if" Expression Block [ "else" ( IfStmt | Block ) ] ;
WhileStmt       ::= "while" Expression Block ;
ForStmt         ::= "for" "(" ForInit ";" [ Expression ] ";" [ Expression ] ")" Block ;
ForInit         ::= VarDecl | ExprStmt | "" ;
ReturnStmt      ::= "return" [ Expression ] ";" ;
TryCatchStmt    ::= "try" Block "catch" Identifier Block ;
MatchStmt       ::= "match" Expression "{" { MatchArm } "}" ;
MatchArm        ::= ( Pattern | "_" ) "=>" ( Expression | Block ) [ "," ] ;

ImportStmt      ::= "import" "(" StringLiteral ")" ";" ;
ExprStmt        ::= Expression ";" ;
Block           ::= "{" { Statement } "}" ;

FnSignature     ::= "fn" Identifier "(" [ Parameters ] ")" ";" ;
Parameters      ::= Identifier [ ":" Type ] { "," Identifier [ ":" Type ] } ;
Identifiers     ::= Identifier { "," Identifier } ;
TypeParams      ::= Identifier { "," Identifier } ;
Type            ::= Identifier [ "<" TypeParams ">" ] ;

Expression      ::= Assignment ;
Assignment      ::= FieldAccess "=" Assignment
                  | Primary "[" Expression "]" "=" Assignment
                  | Identifier "=" Assignment
                  | LogicOr ;
FieldAccess     ::= Primary { "." Identifier } ;
LogicOr         ::= LogicAnd { "||" LogicAnd } ;
LogicAnd        ::= Equality { "&&" Equality } ;
Equality        ::= Comparison { ( "==" | "!=" ) Comparison } ;
Comparison      ::= Term { ( "<" | "<=" | ">" | ">=" ) Term } ;
Term            ::= Factor { ( "+" | "-" ) Factor } ;
Factor          ::= Unary { ( "*" | "/" | "%" ) Unary } ;
Unary           ::= ( "!" | "-" ) Unary | Primary ;

Primary         ::= NumberLiteral
                  | StringLiteral
                  | BooleanLiteral
                  | "nil"
                  | Identifier
                  | "(" Expression ")"
                  | ArrayLiteral
                  | MapLiteral
                  | CallExpr ;

CallExpr        ::= Primary "(" [ Arguments ] ")" ;
FieldExpr       ::= Primary "." Identifier ;  (* dot-access read *)
Arguments       ::= Expression { "," Expression } ;
ArrayLiteral    ::= "[" [ Arguments ] "]" ;
MapLiteral      ::= "{" [ MapEntries ] "}" ;
MapEntries      ::= MapEntry { "," MapEntry } ;
MapEntry        ::= ( StringLiteral | Identifier ) ":" Expression ;

Identifier      ::= ( Letter | "_" ) { Letter | Digit | "_" } ;
NumberLiteral   ::= Digit { Digit } [ "." Digit { Digit } ] ;
StringLiteral   ::= '"' { Character } '"' ;
BooleanLiteral  ::= "true" | "false" ;
```

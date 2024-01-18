%{
#include "../src/tree.h"
#include "../obj/tpc-2022-2023.tab.h"


/* tpcas du projet */
/* compte le numéro de la ligne */
int lineno = 1;
/* compte le numéro du caractère traité en fonction de la ligne*/
int carno = 1;

/* les options nounput, noinput, noyywrap,... sont nécessaire pour 
éviter que le compilateur ne disent qu'il y a un appel implicite de yylex()*/
/* on créé la condition de démarrage comm qui sert pour les commentaires*/
%}

%x COMM 
%option nounput
%option noinput
%option noyywrap
%option yylineno

%%
\/\* {BEGIN COMM;} /* on passe en mode commentaire*/


<COMM>. {;} /* attention il n'est pas possible d'écrire un commentaire directement
ici car le compilateur va le prendre pour une règle, il faut donc écrire :
"unerègle { une action;} " devant
*/
<COMM>\n {lineno++; carno=1;}
<COMM>\*\/ {BEGIN INITIAL;} /* on sort du mode commentaire et on retourne dnas l'état
initial */

if {carno+=strlen("if");return IF;}
else {carno+=strlen("else");return ELSE; }
return {carno+=strlen("return");return RETURN; }
while {carno+=strlen("while");return WHILE; }

(int|char) {yylval.text = (char * )malloc(sizeof(char)*(strlen(yytext)+1)); 
strcpy(yylval.text, yytext);carno+=strlen(yytext);
return TYPE; } /* on copie la chaine de caractère qui indique le type : 
soit ce sera int 
soit ce sera char 
et on la met dans yylval.
Ce texte sera ainsi enregistrée lorsqu'on ajoutera le noeud à l'arbre */ 
void {yylval.text = (char * )malloc(sizeof(char)*(strlen(yytext)+1)); 
strcpy(yylval.text, yytext);return VOID;}
[a-zA-Z_][a-zA-Z_0-9]* {yylval.text = (char * )malloc(sizeof(char)*(strlen(yytext)+1)); 
strcpy(yylval.text, yytext); carno+=strlen(yytext);
/* on copie la chaine de caractère qui indique le nom de l'identificateur
Ce texte sera ainsi enregistrée lorsqu'on affichera le nom de l'identificateur */ 
return IDENT;} /* 
à ce niveau là on ne peut pas différencier un caractère d'un identificateur, ce travail de distinction
se fera dans l'analyseur sémnantique. En attendant on les classe tous comme
identificateurs*/ /* si on tombe sur une chaine de caractère, comme ici avec IDENT, on 
ne peut pas renvoyer le tableau donc on fait un malloc qui va contenir la chaine 
de caractère*/ 
[0-9]+ {yylval.entier = atoi(yytext); carno+=strlen(yytext); return NUM;} /*un simple nombre*/
(==|!=) {yylval.text = (char * )malloc(sizeof(char)*(strlen(yytext)+1)); 
strcpy(yylval.text, yytext); carno+=strlen(yytext);
return EQ;}

(<|>|<=|>=) {yylval.text = (char * )malloc(sizeof(char)*(strlen(yytext)+1)); 
strcpy(yylval.text, yytext); carno+=strlen(yytext); return ORDER;}
[+|-] {yylval.car = yytext[0]; carno+=strlen(yytext);return ADDSUB;}
[*|\/|%] {yylval.car = yytext[0]; carno+=strlen(yytext); return DIVSTAR;}
(\|\|) {carno+=strlen(yytext);return OR;}
&& {carno+=strlen(yytext);return AND;}


\/\/.* {carno+=strlen(yytext);}/* commentaire simple ligne : on ne fait rien pour tout le texte qui suit*/
\'[^']\' {yylval.car = yytext[1]; carno+=1; return CHARACTER;}/* cas de caractère qui se retrouvent entre guillemets
(début)*/
\'\\n\' {yylval.car = yytext[1];carno+=1;return CHARACTER;} 
\'\\r\' { yylval.car = yytext[1];carno+=1;return CHARACTER; } /* retour à la ligne version windows*/
\'\\t\' {yylval.car = yytext[1];carno+=1;return CHARACTER;}
\'\\.?\' {yylval.car = yytext[1];carno+=1;return CHARACTER;} /* cas de caractère qui se retrouvent entre guilemmets
(fin)*/


[\n] {lineno++; carno = 1;}
[ \t] {carno+=1;}
[\r] {lineno++; carno = 1;}
\(  {carno+=1;return yytext[0];}

\) {carno+=1;return yytext[0];}

\} {carno+=1;return yytext[0];}

\{ {carno+=1;return yytext[0];}

\; {carno+=1;return yytext[0];}

\, {carno+=1;return yytext[0];}

\= {carno+=1;return yytext[0];}

\! {carno+=1;return yytext[0];}

\: {carno+=1;return yytext[0];}

.  {carno+=1;return yytext[0];}

<<EOF>> return 0;
%%

/* yytext[0] contient le premier élément de la chaine reconnue en tant que léxème 
(=mot-clé)
si la chaine est un simple caractère alors elle renvoie le seul caractère qui la compose
*/

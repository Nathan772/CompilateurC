/*
fichier permettant la génération de l'arbre abstrait
*/

#ifndef __tree__
#define __tree__


#include "string.h"
/* tree.h */
/* voir #include if_ndef*/
/*
remarque 
on ajoute dans le typedef enum les mots-clés :
accOuvrante, accFermante,parOuvrante,parFermante, virgule,pointVirgule
même si ils ne seront pas utilisés dans l'arbre.
On est obligé de les ajouter car ils apparaissent en tant que léxème dans
le fichier ".y". Par conséquent il leur faut des équivalents même si
ils ne seront pas traités par l'arbre.
On ne les traite pas dans l'arbre car le prof a précisé qu'il ne fallait pas 
les mettre dans l'arbre.
*/
typedef enum {
  PROG,
  LValue,
  E,
  T,
  multi, /* multiplication*/
  divi, /*division*/
  id, /*équivalent "IDENT" == identificateur*/ 
  DeclVars,
  DeclVarsFonc,
  DeclFonct,
  typeRetour,
  parametre,
  type,
  declarateur,
  DeclFoncts,
  EnTeteFonct,
  Corps,
  parOuvrante,
  parFermante,
  Parametres,
  Void,
  ListTypeVar, 
  accOuvrante,
  accFermante,
  suiteInstr,
  instr,
  affectation,
  diff, /* different de */
  expression,
  If,
  Else,
  While,
  Return,
  pointVirgule,
  or,
  TB,
  and,
  FB, 
  eq, 
  comparateur,
  M, /* arrêt ici*/
  sup,
  supEq,
  inf,
  infEq,
  add,/* à partir d'ici +3 */
  sub,
  num,
  character,
  arguments,
  argument,
  ListExp,
  virgule,
  variable,
  appelVar,
  opeLogique,
  operateur,
  appelFonction,
  conditionnel

  /* list all other node labels, if any */
  /* The list must coincide with the string array in tree.c */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
} label_t;

typedef struct Node {
  label_t label;
  struct Node *firstChild, *nextSibling;
  /* indique la ligne où a été trouvée l'élément dans la fichier*/
  int line;
  /* indique le type de l'élément lorsque son expression est évaluée
  (cela sera utile pour les fonctions et les variables) pour cela on utilise
  les typeenum "MY_INT, MY_CHAR, MY_NONE,_MY_NULL,MY_VOID*/
  int typeExpr;
  /*indique s'il s'agit d'une variable ou d'une fonction
  TYPE_VAR == 0 == variable;
  TYPE_FUN == 1 == function;
  TYPE_PARAM == 2 == un paramètre
  TYPE_CONST == 3 == une constante
  TYPE_OTHER == 4 = pas une variable, ni une fonction
  
  */
  int typeIdent; 
  /*"cat" permet de savoir quel champ a été choisi dans l'union :
  TEXT == 0 == text
  ENTIER == 1 == entier
  CAR == 2 == car
  Cela permet de savoir rapidement quel champ utiliser en faisant
  if(cat == TEXT){
    printf("%s \n", node->valeur.text);
  }
  else if(cat == ENTIER)
     printf("%d \n", node->valeur.entier);
  */
  int cat;
  /* 
  Cette union (valeur) regroupe les différents types possible pour l'élément à 
  afficher.
  Par exemple :
  
  -si on tombe sur un 'a' dans le fichier ".c" à analyser
  alors ce sera un "car", car 'a' est un simple caractère
  -si on tombe sur une variable (int ou char) alors ce sera 
  "text" car c'est stocké sous forme de chaine de caractère
  -si on tombe sur un entier comme '333' alors ce sera "entier"
  car c'est stocké sous forme d'entier
  */
  union{
    char * text;
    int entier;
    char car;
  }valeur;

} Node;

Node *makeNode(label_t label, int typeIdent, int typeExpr, int cat);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node*node);
void printTree(Node *node);
/* cette fonction copie une chaine dans le champ
"valeur.text" d'un noeud*/
int copieChaineNoeud(Node * node, char * chaine);

/* cette fonction copie le type de chaine1 sous la forme
d'un entier dans le noeud "node".
Cela permet d'indiquer que la fonction ou la variable est de type "MY_INT"*/
void copieIntType(Node * node, char * chaine1);

/* transforme un type écrit sous forme de
chaine de caractère en l'entier associé
renvoie  :
MY_INT : si c'est un "int"
MY_CHAR : si c'est un "char"*/
int typeStrToInt(char * chaine);

#define FIRSTCHILD(node) node->firstChild
#define SECONDCHILD(node) node->firstChild->nextSibling
#define THIRDCHILD(node) node->firstChild->nextSibling->nextSibling

/* succession d'entiers qui permettent d'indiquer par un entier
le type associé à un noeud que ce soit pour une variable
ou pour une fonction (ce sera aussi utile dans la table des symboles)
*/
enum myType{
  MY_INT,
  MY_CHAR,
  MY_VOID,
  MY_NULL,
  MY_NONE
};

/* 
Succession d'entiers qui indiquent les types possibles pour le champ
"valeur" d'un noeud.
*/
enum typeNode{
  TEXT,
  ENTIER,
  CAR,
  NONE /* cette catégorie sert lorsqu'il n'y a rien à 
  afficher du point de l'union dans le ".y"
  C'est à dire lorsque dans le ".lex", on se contente 
  de renvoyer une valeur sans faire d'affectation*/
};

/* succession d'entier qui précise à quoi est associé un
identificateur : une variable, une fonction, un paramètre,
other (celui-là indique que ce n'est pas indentificateur)*/
enum typeIdent{
  TYPE_VAR,
  TYPE_FUN,
  TYPE_PARAM,
  TYPE_CONST,
  TYPE_OTHER
};
#endif
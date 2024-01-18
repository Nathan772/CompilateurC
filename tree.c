/*
fichier permettant la génération de l'arbre abstrait
*/
/* tree.c */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
extern int lineno;       /* from lexer */

static const char *StringFromLabel[] = {
  "PROG",
  "lValue",
  "E",
  "T",
  "*",
  "/",
  "identificateur : ",
  "DECLVARS",
  "DECLVARSFONC",
  "declfonc : ",
  "type retour : ",
  "parametre : ",
  "type : ",
  "declarateur",
  "DeclFoncts",
  "EnTeteFonct",
  "Corps",
  "(",
  ")",
  "Parametres",
  " ",/* cas où le paramètre est "void" on affichera autrement le mot "void"*/
  "ListTypeVar",
  "{",
  "}",
  "suiteInstr",
  "instr", /* désigne une instruction*/
  "=", /*indique une affectation*/
  "!=",
  "expr", /* désigne une expression*/
  "if", /* pour les mots comme : "if", "else", "while", "return", ...
  une étiquette avec leur nom suffit.
  Leur fils seront les éléments qui sont à l'intérieur de ces conditionnelles
  (voir p.33 cm numero 4 avec l'exemple d'arbre abstrait)*/
  "else",
  "while",
  "return",
  ";",
  "||",
  "TB",
  "&&",
  "FB",
  "==", 
  /*label pour une comparaison*/
  "compare : ",
  "M",
  "<",
  "<=",
  ">",
  ">=",
  "+",
  "-",
  "entier : ",
  "character : ",
  "arguments",
  "argument",
  "ListExp",
  "virgule",
  "nom variable : ",
  "nom variable : ",
  "operateur logique : ",
  "operateur : ",
  "appel fonction : ",/* label pour les appels de fonction*/
  "conditionnel : "





  /* list all other node labels, if any */
  /* The list must coincide with the label_t enum in tree.h */
  /* To avoid listing them twice, see https://stackoverflow.com/a/10966395 */
};

/*
le paramètre "typeExpr" indique le type de l'élément associé au noeud :
-si c'est une fonction c'est son type de retour.
-si c'est une variable c'est son type.

le paramètre "typeIdent" indique s'il s'agit d'une fonction ou d'une variable :
-TYPE_VAR == c'est une variable
-TYPE_FUN == c'est une fonction

le paramètre "cat" est un entier qui indique le champ qui est 
utilisé pour stocker l'élément dans l'union :

-si c'est le champ "entier" alors sa valeur est "ENTIER".
-si c'est un "caractère" alors sa valeur est "CAR".
-si c'est stocké sous forme d'une chaine de caractère alors sa valeur est "TEXT".

*/
Node *makeNode(label_t label, int typeIdent, int typeExpr, int cat) {
  Node *node = malloc(sizeof(Node));
  if (!node) {
    printf("Run out of memory\n");
    exit(1);
  }
  node->label = label;
   /* précise si c'est une variable
  ou une fonction*/
  node->typeIdent = typeIdent ;
  /* précise le type de l'expression
  lorsqu'elle est évaluée*/
  node->typeExpr = typeExpr; 
  node->cat = cat;
  /* par défaut le fils et le frère valent null*/
  node->firstChild =  NULL; 
  node->nextSibling = NULL;
  node->line=lineno;/* on ajoute la ligne où a été trouvé le léxème (token/mot-clé).*/
  return node;
}

void addSibling(Node *node, Node *sibling) {
  Node *curr = node;
  while (curr->nextSibling != NULL) {
    curr = curr->nextSibling;
  }
  curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
  if (parent->firstChild == NULL) {
    parent->firstChild = child;
  }
  else {
    addSibling(parent->firstChild, child);
  }
}

void deleteTree(Node *node) {
  if (node->firstChild) {
    deleteTree(node->firstChild);
  }
  if (node->nextSibling) {
    deleteTree(node->nextSibling);
  }
  free(node);
}

void printTree(Node *node) {
  static bool rightmost[128]; // tells if node is rightmost sibling
  static int depth = 0;       // depth of current node
  for (int i = 1; i < depth; i++) { // 2502 = vertical line
    printf(rightmost[i] ? "    " : "\u2502   ");
  }
  if (depth > 0) { // 2514 = L form; 2500 = horizontal line; 251c = vertical line and right horiz 
    printf(rightmost[depth] ? "\u2514\u2500\u2500 " : "\u251c\u2500\u2500 ");
  }
  if(node->cat == TEXT)
    printf("%s %s ", StringFromLabel[node->label], node->valeur.text); 
  else if (node->cat == CAR)
    printf("%s %c", StringFromLabel[node->label], node->valeur.car); 
  else if (node->cat == ENTIER)
    printf("%s %d", StringFromLabel[node->label], node->valeur.entier); 
  else
    printf("%s", StringFromLabel[node->label]); 

  printf("\n");
  depth++;
  for (Node *child = node->firstChild; child != NULL; child = child->nextSibling) {
    rightmost[depth] = (child->nextSibling) ? false : true;
    printTree(child);
  }
  depth--;
}

/* cette fonction copie la chaine de chaine1 dans le champ
"valeur.text" d'un noeud (node)*/
int copieChaineNoeud(Node * node, char * chaine1){
  if( (node->valeur.text = (char *) malloc(sizeof(char)*(strlen(chaine1)+1))) != NULL){
    /* on fait la copie*/
    /*printf("on copie la chaine %s \n\n", chaine1);*/
    strcpy(node->valeur.text,chaine1);
    return 1;
  }
  else{
  /* échec du malloc*/
    printf("Erreur : Echec du malloc dans la fonction copieChaineNoeud!! \n\n");
    return 0;
  }

}


/* cette fonction copie le type de chaine1 sous la forme
d'un entier dans le noeud "node".
Cela permet d'indiquer que la fonction ou la variable est de type "MY_INT"*/
void copieIntType(Node * node, char * chaine1){
  if(strcmp(chaine1,"int") ==0){
    node->typeExpr=MY_INT;
  }
  else if(strcmp(chaine1,"char") ==0){
    node->typeExpr=MY_CHAR;
  }
  else
    printf("erreur : aucun type ne correspondant à la copie de type.");
}

/* transforme un type écrit sous forme de
chaine de caractère en l'entier associé
renvoie  :
MY_INT : si c'est un "int"
MY_CHAR : si c'est un "char"*/
int typeStrToInt(char * chaine){
  if(strcmp(chaine, "int") == 0)
    return MY_INT;
  else if(strcmp(chaine, "char") ==0)
    return MY_CHAR;
  else
    printf("erreur : aucun type ne correspondant à la copie de type.");
  return MY_NONE;
}
/* fichier qui gère la génération de l'assembleur à partir de l'arbre*/

#ifndef __generationAssembleur__
#define __generationAssembleur__

#include "tree.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "analyseTree.h"

/* 

fonction qui gère les calculs
lorsqu'on tombe sur une opération
dans l'arbre 
node correspond au noeud sur lequel on est 
currFonc correspond à la fonction dans laquelle on est actuellement
*/
void compute(Node *node, TabSymb tab1, char * currFonc, FILE *fichier);

/*void newFile(FILE *fichier);*/

void endFile(FILE * fichier);

/* fonction qui génère le fichier assembleur à partir de
la racine d'un arbre*/
void generateAssembly(FILE * fichier, TabSymb tab,Node * node1);

/* fonction qui prend les variables globales de la tab des symboles et les ajoute au fichier assembleur*/
void ajoutGlobal(FILE * fichier, TabSymb tab);

/*fonction qui écrit le début du main de l'assembleur*/
void beginMainFile(FILE * fichier);

/* écrit l'entête de base pour un fichier nasm*/
void enteteBasique(FILE * fichier);

/* fichier qui gère de façon globale la génération du fichier NASM 
fichier qui va recevoir le code nasm
node le noeud sur lequel on est actuellement
currFonc : indique la fonction dans laquelle on est 
tab1 la tab des symboles
pere est le pere du node
*/
int creationNasm(FILE * fichier,Node * pere, Node * node, char * currFonc, TabSymb tab1);

/* fonction qui gère les affectations */
void handleAffectation(FILE * fichier, Node * pere,  Node * node, char * currFonc, TabSymb tab1);

/* fonction qui gère les déclaration de variable locales mais qui ne gérera pas les affectations
les affectations à la déclaration seront gérées par d'autres variables */
void handleDeclLocale(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1);

/* fonction qui gère les déclaration de variable locales mais qui ne gérera pas les affectations
les affectations à la déclaration seront gérées par d'autres variables */
void handleIf(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1);

/* 
fonction qui gère les appels de fonctions 
compteurArgs : variable qui compte le nombre d'arguments qu'on a déjà ajouté.
*/
void handleAppelFonc(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1, int * compteurArgs);

/* fonction qui gère les returns dans une fonction */

void handleReturn(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1);

/* fonction qui gère les calculs
lorsqu'on tombe sur une opération
dans l'arbre mais dans le cas spécifique des appels de fonctions
node correspond au noeud sur lequel on est 
pere correspond au pere de node
currFonc correspond à la fonction dans laquelle on est actuellement*/
void compute2(Node * pere, Node *node, TabSymb tab1, char * currFonc, FILE *fichier);


#endif


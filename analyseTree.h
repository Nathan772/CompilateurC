/*
Fichier chargé de l'analyse de l'arbre abstrait pour l'analyse sémantique
*/

#ifndef __analyseTree__
#define __analyseTree__

#include "tree.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>


#define NB_FONC_INIT 25
#define MAX_TYPE_NAME 100
#define BLOC 2
/* on autorise mille variables pour permettre au hashcode d'être sur une vaste étendue. En réalité cela servira
surtout à disperser les valeurs, les emplacements ne seront pas réellement utilisés*/
#define NB_VAR 4096
#define BUFFISIZE_INIT 10
#define MAX_ARGS 50

/*début des champs associés à la fonction, ses arguments, aux variables de la fonction*/
#define ARGS 1
#define VARIABLES 2
#define FONCTION 0

/* undefined c'est pour le début du programmme : on ne sait pas encore si on est tombé sur une variable ou une fonction*/
#define UNDEFINED 6

/*  0 est l'emplacement de la fonction nommée "globale" qui correspond à l'endroit 
où on stocke les variables globales 
(plus tard, Les 4 premières cases du tableau sont réservées aux fonctions de la librairies standard.
GLOBALES sera alors situé à la case 4*/
#define GLOBALE 0
/* line définie arbitrairement comme étant la ligne où a été définie une fonction importée (getchar, getint, etc...)*/
#define LINE_IMPORT 0
#define MAX_VAR_NAME 100
#define MAX_MSG_SIZE 300
#define MAX_NUMBER_SIZE 100




typedef struct Variable{

  /* type de la variable*/
  /* les types des variables sont des my_types */
  int type;
   /* nom de la variable*/
   /* on limite les noms des variables raisonnablement à 100 caractères*/
  char ident[MAX_VAR_NAME];

  /* catégorie précise s'il s'agit d'une fonction ou d'une variable ou si c'est indéfini*/
  int cat;

  /* indique si un type de retour a été reçu : -1 signifie non et le reste veut dire oui.
  si la fonction attendait un type retour mais qu'elle n'a rien reçu, on le fera signalé grâce à ce champ.
  si elle n'attendait rien mais qu'elle a reàu qqc : ce champ n'est pas utile car la fonction check_return gère déjà ce cas.
  */
  int type_rec;

  /* suivant de la variable*/
  struct Variable * suivant;

  /* champ indiquant où se trouve la variable au niveau du fichier C*/
  int line;

   /* adresse de stockage pour nasm, elle va de 8 en 8 peu importe le type choisi */

  int adresse;




}Variable;

typedef struct Tab_symbole{
  /* compte le nombre de fonction enregistrée dans la table des symboles.
    on donne tjrs à nb_fonc le nombre de fonction que l'on a avant l'ajout de la fonction.
    Par exemple si la table des symboles contient 1 fonction, sa valeur sera de un, ce qui parait logique, seulement on donnera la valeur 1 juste avant l'ajout de la fonction dans la table 
    et pas après*/
  int nb_fonc;
  /* indique le nombre de fonction acceptable au maximum c'est à dire la taille du tableau.*/
  int max;
  /* enregistre les variables des fonctions, les arguments et les noms des fonctions*/
  /* la première case [x][0] ,avec x un numéro de case quelconque, correspond à la fonction et son type de retour, la deuxèime case [x][2] est une liste chainée qui contient les 
  arguments et les types des arguments de la fonction , le reste [x][3,4,...,n] correspondent aux variables dans la fonction*/
  /* à noter que dans le cas d'un argument vide (VOID), on considérera que l'argument n'est pas à ajouter et que l'emplacement n'est pas à remplir : l'absence d'argument n'est pas un argument*/
  Variable **tab_fonc;
  /* champ qui vérifie qu'un main est présent
  0 : signifie non
  1 signifie oui*/
  int main;
 

}TabSymb;

typedef struct tabVariable{
  /* tableau qui contient l'ensemble des variables utilsiées lors de l'appel de la fonction 
  On partira du principe que l'utilisateur n'a pas plus de 50 (max_args) arguments
  */
  Variable tab[MAX_ARGS];
  /* indique le nombre de variable de ma fonction*/
  int nbVar;

  int nbInt;
  int nbChar;
  int nbVoid;
}tabVariable;



/* fonction qui complète les noeuds de l'arbre en leur donnant leurs type
PereType correspond au type du noeud de type "type" "étant le père du noeud le plus proche"
*/
void completerNode(Node * node, Node * pereType);

/*fonction qui remplie la table des symboles en analysant l'arbre
elle renvoie 0 en cas d'erreur et 1 si tout s'est bien passé
le noeud père correspond au noeud parent de "node" */
int remplirTabSymb(TabSymb * tab, Node * node, Node *pere);

/*
Fonction qui vérifie les affectations à la déclaration spécifiquement car les éléments données peuvent ne pas être valides
elle renvoie :
1: si tout s'est bien passé
0 : cas d'erreur 
Noeud correspond au noeud que l'on analyse et pour lequel on va vérifier qu'il est bien dans 
la tab des symboles "tab".
*/
int checkAffectationPendantRemplir(Node * leftValue, Node * node, TabSymb tab);

/* fonction qui gère la copie d'un type :
du noeud expéditeur (exp),
vers le noeud destinataire (dest)*/
void copieType(Node *nodeDest, Node * nodeExp);

/* fonction qui initalise la table des symboles, notamment en faisant les mallocs nécessaires 
pour celle-ci.*/
int init_tab_symb(TabSymb * tab);

/* 
Cette fonction indique si cette variable a déjà été ajouté pour cette fonction
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie NULL sinon
pos = fonction de référence : fonction appelante */
Variable * findVariable(TabSymb tab_symb , char * nom_var, int pos);

/* 
Cette fonction indique si cette variable a déjà été ajouté pour cette fonction
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie NULL sinon
nomFonc = fonction de la fonciton de référence : fonction appelante */
Variable * findVariable2(TabSymb tab1 , char * nomVar, char * nomFonc);

/* 
Cette fonction indique si l'emplacement d'une fonction dans la tab des symboles.
Elle renvoie -1 si la fonction n'a pas été trouvée.
Sinon elle renvoie l'emplacement où se trouve la fonction  */

Variable * findFonc(TabSymb tab_symb , char * nomFonc);

/* 
Cette fonction indique si une variable a déjà été ajouté parmi les variables globales
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie NULL sinon*/
Variable * findGlobale(TabSymb tab_symb , char * nom_var);

/* type qui permet d'indiquer si on est dans le cas d'une variable, d'une fonction ou d'un fichier par un énum en ce qui concerne la tab des symboles */
enum type_ident{

  MY_FUNCTION,
  MY_VARIABLE,
  MY_PARAMETRE,
  MY_FILE,
  MY_CONST
  
};


/*

Trouve le type d'une variable dans la table des symboles en fonction de la fonction dans 
laquelle on se situe

renvoie le type trouvé 
si la variable n'est pas dans la table, on renvoie -1

*/

int findTypeVar(TabSymb tab, char * nom_var, char * nomFonc);

/* 
Cette fonction indique l'emplacement d'une fonction dans la tab des symboles.
Elle renvoie -1 la position si la fonction n'a pas été trouvée.
Sinon elle renvoie la position en elle même où se trouve la fonction  */

int findFoncPos(TabSymb tab_symb , char * nomFonc);

/*
cette fonction 
réalloue de la place dans la tab des symboles
lorsqu'il n'y en a plus assez pour contenir toutes les fonctions*/
int realloueTabSymb(TabSymb * tab);

/* cette méthode copie le contenu d'un noeud vers une variable*/
int copieNodeVersVariable(Variable * dest, Node * source);

/* cette fonction crée un hashcode en fonction du nom d'une variable*/
/* elle renvoie une valeur qui correspond au hashcode*/
/* elle renvoie -1 si l'élément donné était une chaine vide*/
int hashCode(char * nom_var);

/* affiche le type d'une variable ou le type de retour d'une fonction*/
void show_type(int type);

/* affiche une liste chainée de variable*/
void showListeVar(Variable * var);

/* cette fonction montre le contenu de la tab des symboles*/
void showTabSymb(TabSymb tab);

/*cette fonction ajoute un noeud associé à une variable ou une fonction dans la table
des symboles si il n'y est pas déjà
elle renvoie 0 si on ajoute deux fois la même variable
1 si tout s'est bien passé.
et elle renvoie -1 si il y a un échec d'allocation
pere correspond au pere de de node*/
int addNodeInSymb(TabSymb * tab, Node * node, Node * pere);

/* ajoute une fonction pour les variables globales par défaut*/
int addGlobaleParDefaut(TabSymb * tab);

int evalExpr(Node * node);

/* vérifie que les variables et les paramètres utilisés pour
une fonction sont valides.
Vérifie aussi que les arguments lors des apples sont valides
renvoie 0 en cas de problème
renvoie 1 si tout s'est bien passé
currFonc correspond à la fonction dans laquelle
on est cela permet à la table de symbole de 
savoir quelle variable sont autorisées (
  les variables déclarées dans f1 ne sont pas utilisables dans f2
) */

int checkVariableParametresAppel(Node * node, TabSymb tab, char * currFonc );

/* cette fonction envoie un message en cas d'erreur
sémantique.
Elle précise la ligne et le caractère*/
void semError(char * msg, int line);

/*
Cette fonction donne les arguments attendus par la fonction de nom "name"
et les ajoutes dans tabVar
*/
void retrieveParametreAttendu(char * name, TabSymb tab_symb, tabVariable * tabVar);

/*
node contient le noeud associé à la fonction qui nous intéresse (fonction appelée).
Deux tableaux: 
tabArgAtt correspond au tableau des arguments attendus
tabArgRec correspond au tableau des arguments reçus 
renvoie 2 en cas de warning (int remplace char).
renvoie 1 si tout s'est bien passé.
renvoie 0 en cas d'erreur (pas suffisamment d'arguments)
*/
int compareArgsAttRecu(Node node, tabVariable tabArgAtt, tabVariable tabArgRec);

/* fonction qui vérifie le type et le nombre des arguments pour un appel de fonction.
-l'argument est un noeud, au départ ce sera le noeud associé à la fonction
-le name permet de savoir dans quel fonction on est.
-le deuxième argument est la tab des symboles cela permet d'avoir des infos
-le troisième est un tableau de variable qui contient l'ensemble des arguments utilisées
par la fonction.
sur la fonction traitée et ses arguments
renvoie 1 si tout s'est bien passé
renvoie 0 en cas d'erreur */
int checkArgFonc(Node * pere, Node * node, char * name, TabSymb tab1, tabVariable * tabVar);

/*
Fonction qui teste les appels de fonction.
Elle renvoie 2 en cas de warning (int remplace char).
renvoie 1 si tout s'est bien passé.
renvoie 0 en cas d'erreur (pas suffisamment d'arguments)
on part de la racine et on analyse tout l'arbre
currFonc indique la fonction dans laquelle on est actuellement, la fonction appelante
*/
int checkAppelFonc(Node * node, char * currFonc, TabSymb tab);

/* cette fonction envoie un warning en cas de problème sémantique
sémantique. Elle précise la ligne*/
void semWarning(char * msg, int line);

/*
Fonction qui vérifie les affectations
elle renvoie :
2:si il y a un warning
1: si tout s'est bien passé
*/
int checkAffectation(Node * pere, char * currFonc, Node * node, TabSymb tab, tabVariable *lValue , tabVariable * rValue);

/*
fonction qui analyse les rvalue et les lvalue d'une fonction et précise si il y a un problème.
renvoie 1 : tout s'est bien passé.
renvoie 2 :cas warning.
renvoie 0 : cas d'erreur (affectation de void)
*/
int CompareRLValue(tabVariable lValue , tabVariable rValue);

/* 
cette fonction met à jour la ligne
où a été trouvée une variable */
void updateLineVariable(int line, Variable * var1);

/* this function check if a main is present */
int checkIfMainIsHere(TabSymb tab_symb);

/* cette fonction envoie un message en cas d'erreur
sémantique. Elle ne précise pas de ligne*/
void semError2(char * msg);

/*
la currfonc correspond à la fonction dans laquelle on est actuellement
node correspond au noeud auquel on est
pere correspond au pere du noeud
tab_symb est la tab des symboles à remplir avec les infos
tab1 correspond à l'ensemble des valeurs données comme valeurs de retour
*/
void addReturnValue(TabSymb * tab_symb, char * currFonc, Node * node, Node * pere, tabVariable  *tab1);

/*
node correspond au noeud traité, si c'est une valeur de retour on l'ajoute à returnValues
la fonciton renvoie 0 en cas de problème
renvoie 1 si tout s'est bien passé 
*/
int retrieveReturnValues(TabSymb tab, Node * node, char * currFonc, tabVariable * returnValues);

/* fonction d'ajout de variables dans la table 
des variables */
void ajoutVariableTabVar(tabVariable * tabVar, Variable var1);

/*
renvoie le type de retour associé à un ensemble de valeur donné en retour
*/
int fromTabToReturnVal(tabVariable returnValues);


/*
Cette fonction compare le type reçu par les fonctions avec celui attendu par celle-ci.
Si il sont incompatibles elle renvoie 0.
Si il sont compatibles elle renvoie 1.
Si cela soulève un warning elle renvoie 2.

*/
int compareTypeRecToExp(TabSymb tab);


/* cette méthode copie le contenu d'un noeud vers une variable mais en prenant en compte la gestion d'@
elle renvoie 1 si tout s'est bien passé.
et elle renvoie 0 si elle renvoie uen variable avec le même nom qu'une autre.
le paramètre globale indique si oui ou non on gère une variable globale : 0 : non, 1 : oui
*/
int copieNodeVersVariableForTabSymb(Variable * dest, Node * source, int global);

/*
fait une copie profonde d'une variable vers une autre 
*/
void deepCopyVar(Variable *dest, Variable source);

void gatherTypeExpr(Variable var1, tabVariable * tabVar);

/*
Fonction qui vérifie les affectations à la déclaration spécifiquement car les éléments données peuvent ne pas être valides
elle renvoie :
1: si tout s'est bien passé
0 : cas d'erreur 
Noeud correspond au noeud que l'on analyse et pour lequel on va vérifier qu'il est bien dans 
la tab des symboles "tab".
leftvalue correspond au noeud qui doit recevoir l'affectation
Cette fonction part du présupposé qu'on démarre à partir du fils du noeud qui reçoit l'affectation.
Dans le cas contraire, elle ne sera pas fonctionnelle
*/
int checkAffectationPendantRemplirAux(Node * leftValue, Node * node, TabSymb tab);


/* ajoute les fonctions de la librairie standard */

int addLibStandard(TabSymb * tab);

/* 
Cette fonction indique si cette variable a déjà été ajouté pour cette fonction
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie NULL sinon
nomFonc = fonction de la fonciton de référence : fonction appelante */
Variable * findVariable2(TabSymb tab1 , char * nomVar, char * nomFonc);

/* donne l'@ d'une variable locale ou globale
en cherchant dans les variables locales puis globales
currFonc correspond à la fonction dans laquelle on est actuellement
tab1 est la tab des symboles
retourne -1 si la variable est introuvable
 */
int findAdresseVariable(char * nameVariable, char * currFonc , TabSymb tab1);

/* fonction qui indique si une variable est locale
1 == oui
0 == non
*/
int isLocalVariable(TabSymb tab1 , char * nomVar, char * nomFonc);

/* fonction qui indique si une variable est locale
1 == oui
0 == non
*/
int isGlobalVariable(TabSymb tab1 , char * nomVar);

/* donne l'@ d'une variable locale ou globale
en cherchant dans les variables locales puis globales
currFonc correspond à la fonction dans laquelle on est actuellement
tab1 est la tab des symboles
retourne -1 si la variable est introuvable
 */
int findAdresseVariable(char * nameVariable, char * currFonc , TabSymb tab1);

/*
fonction qui vérifie que les comparaisons sont toutes valides.
Cela revient à vérifier s'il n'y a pas d'élément de type "void fonction"
(fonction qui renvoie void) parmi les éléments de la comparaison
renvoie 0 en cas d'erreur sémantique
renvoie 1 si tout s'est bien passé
*/
int checkComparaison(Node * node, TabSymb tab1);

/* fonction auxiliaire de la fonction checkComparaison*/
int checkComparaisonAux(Node * node, Node * pere, TabSymb tab1);

/* 
Cette fonction indique si cette variable est un argument de la fonction choisie. 
Elle renvoie 1 si c'est un argument et 0 si ce n'est pas un argument */
int thisVarIsArg(TabSymb tab_symb , char * nom_var, int pos);

/* fonction qui indique le nombre total d'argument de la fonction "nomfonc"
Cela est utile pour savoir s'il y a oui ou non bourrage*/
int compteTotalArg(TabSymb tab1, char * nomfonc);

/*fonction qui complète les @ des arguments dans la tab des symboles*/
void completerAdresseArgs(TabSymb * tab1);

/* fonction qui indique le nombre total d'argument de la fonction qui est positionné à 
l'emplacement "pos" de la tab des symboles.
Cela est utile pour savoir s'il y a oui ou non bourrage*/

int compteTotalArgFromPos(TabSymb tab1, int pos);

#endif
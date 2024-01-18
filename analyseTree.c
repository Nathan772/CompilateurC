/*
Fichier chargé de l'analyse de l'arbre abstrait pour l'analyse sémantique

il faudra ajouter le compteur pour les adresses des arguments 
*/

#include "analyseTree.h"

/* on commence les @ locales à 8 car c'est 8 par rapport à rbp */
int compteurAdrLocale = 8;
/* on commence les arguments à 8 car c'est 8 par rapport à rbp */
int compteurAdrArg = 8;

/* fonction qui complète les noeuds de l'arbre donnant leurs types aux variables qui ne l'avaient pas encore.
"PereType" correspond au noeud de type "type" le plus proche du noeud traité*/

void completerNode(Node * node, Node * pereType){
    /*Node * node2;
    Node * node3;*/

    /* si on tombe sur un noeud null, on arrête le parcours*/
    if(node == NULL){
        return;
    }

    /*si on tombe sur une variable ou un paramètre
    , on copie le type que son père possède si la variable n'a pas pas encore de type
    (type == MY_NONE)*/
    if((node->label == variable ||  node->label == parametre) && node->typeExpr == MY_NONE && pereType != NULL){
        copieType(node, pereType);
    }

    if(node->label == type){
        pereType = node; /* on met à jour le noeud pèreType pour les prochains noeuds qui chercheraient leur type*/
    }

    /*on continue le parcours pour le reste*/
    /* on ne regarde pas les fils de déclVar car ce sont des rights values qui leurs sont affectés*/
    if(node->label != variable)
      completerNode(node->firstChild, pereType);
    completerNode(node->nextSibling, pereType);
}

/* fonction qui gère la copie d'un type :
du noeud expéditeur (exp),
vers le noeud destinataire (dest)*/
void copieType(Node *nodeDest, Node * nodeExp){
    if(nodeExp == NULL)
        return;
    if(strcmp(nodeExp->valeur.text, "int") == 0){
        nodeDest->typeExpr = MY_INT;
    }
    else if(strcmp(nodeExp->valeur.text, "char") == 0){
        nodeDest->typeExpr = MY_CHAR;
    }
    else if(strcmp(nodeExp->valeur.text, "void") == 0){
        nodeDest->typeExpr = MY_VOID;
    }
    else{
        nodeDest->typeExpr = MY_NONE;
    }
}
/*
cette fonction 
réalloue de la place dans la tab des symboles
lorsqu'il n'y en a plus assez pour contenir toutes les fonctions*/
int realloueTabSymb(TabSymb * tab){
  tab->max*=2;
  return 1;
}

/* cette méthode copie le contenu d'un noeud vers une variable
elle renvoie 1 si tout s'est bien passé.
et elle renvoie 0 si elle renvoie uen variable avec le même nom qu'une autre.
*/
int copieNodeVersVariable(Variable * dest, Node * source){
  
  Variable * tmp;
  Variable * next;
  /*if(strcmp(dest->ident, source->valeur.text) == 0){
    printf("Erreur : vous tentez de copier une variable avec le même nom qu'une autre variable dans la même fonction");
    return 0;
  }*/
  tmp = dest;
  /* L'emplacement est déjà utilisé, on parcourt la liste chainée et on vérifie que
  deux variables n'ont pas le même nom*/
  if(tmp->type != MY_NONE){
    
    while(tmp->suivant != NULL){
      
      if(strcmp(tmp->ident, source->valeur.text) == 0){
        printf("Erreur : vous tentez de copier une variable %s avec le même identificateur qu'une autre variable dans la même fonction", source->valeur.text);
        return 0;
      }
      
      tmp = tmp->suivant;
      
    }

    /* aucune variable n'a le nom de la variable que l'on souhaite ajouter,
    on l'ajoute donc en bout de liste chainée*/
    next =  (Variable *) malloc(sizeof(Variable));
    next->line = source->line;
    strcpy(next->ident,source->valeur.text);
    next->type = source->typeExpr;
    next->suivant = NULL;
    next->adresse = 0;
    next->type_rec = MY_VOID;
    tmp->suivant = next;
  }
  /* l'emplacement n'est pas utilisé on copie
  directement dans "dest"
  */
  else{
    dest->line = source->line;
    strcpy(dest->ident,source->valeur.text);
    dest->type = source->typeExpr;
    dest->adresse = 0;
    dest->suivant = NULL;
    dest->type_rec = MY_VOID;
  }
  return 1;

}

/* fonction qui indique le nombre total d'argument de la fonction "nomfonc"
Cela est utile pour savoir s'il y a oui ou non bourrage*/

int compteTotalArg(TabSymb tab1, char * nomfonc){
  int nbArgs;
  int posFonc;
  Variable *tmp;
  /* par défaut il y a 0 args*/
  nbArgs = 0;
  posFonc = findFoncPos(tab1, nomfonc);
   
  /* on regarde dans la partie consacrée aux arguments de la fonction*/
  
  if(posFonc != GLOBALE){

    for(tmp =  &tab1.tab_fonc[posFonc][ARGS]; tmp != NULL;  tmp=tmp->suivant){
      /* si une variable est et qu'elle n'est pas équivalente à "none", on incrémente le compteur*/
      if(strcmp(tmp->ident, "\0") != 0 && tmp->type != MY_NONE){
        /* on compte un argument en plus */
        nbArgs++; 
      }
    }
  }


  return nbArgs;

}

/* fonction qui indique le nombre total d'argument de la fonction qui est positionné à 
l'emplacement "pos" de la tab des symboles.
Cela est utile pour savoir s'il y a oui ou non bourrage*/

int compteTotalArgFromPos(TabSymb tab1, int pos){
  int nbArgs;
  int posFonc;
  Variable *tmp;
  /* par défaut il y a 0 args*/
  nbArgs = 0;
  posFonc = pos;
   
  /* on regarde dans la partie consacrée aux arguments de la fonction*/
  
  if(posFonc != GLOBALE){

    for(tmp =  &tab1.tab_fonc[posFonc][ARGS]; tmp != NULL;  tmp=tmp->suivant){
      /* si une variable est et qu'elle n'est pas équivalente à "none", on incrémente le compteur*/
      if(strcmp(tmp->ident, "\0") != 0 && tmp->type != MY_NONE){
        /* on compte un argument en plus */
        nbArgs++; 
      }
    }
  }


  return nbArgs;

}

/* cette méthode copie le contenu d'un noeud vers une variable mais en prenant en compte la gestion d'@
elle renvoie 1 si tout s'est bien passé.
et elle renvoie 0 si elle renvoie uen variable avec le même nom qu'une autre.
le paramètre globale indique si oui ou non on gère une variable globale : 2 arguments, 0 : locale, 1 : globale
*/
int copieNodeVersVariableForTabSymb(Variable * dest, Node * source, int global){
  
  Variable * tmp;
  Variable * next;
  static int compteurAdrGlobChar = 0;
  static int compteurAdrGlobInt = 0;
  /* on commence les @ locales à 8 car c'est 8 par rapport à rbp */
  /* static int compteurAdrLocale = 8; */

  /* L'emplacement est déjà utilisé, on parcourt la liste chainée et on vérifie que
  deux variables n'ont pas le même nom*/

  tmp = dest;
  if(tmp->type != MY_NONE){

    while(tmp->suivant != NULL){
      
      if(strcmp(tmp->ident, source->valeur.text) == 0){
        printf("Erreur : vous tentez de copier une variable %s avec le même identificateur qu'une autre variable dans la même fonction", source->valeur.text);
        return 0;
      }
      
      tmp = tmp->suivant;
      
    }

    /* on test le dernier cas */

    if(strcmp(tmp->ident, source->valeur.text) == 0){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, "You're trying to add twice the same variable ' ");
        strcat(msg, source->valeur.text);
        strcat(msg, "' in the same function !! ");

        semError(msg, source->line);
        return 0;
    }

    /* aucune variable n'a le nom de la variable que l'on souhaite ajouter,
    on l'ajoute donc en bout de liste chainée*/

    next =  (Variable *) malloc(sizeof(Variable));
    next->line = source->line;
    strcpy(next->ident,source->valeur.text);
    next->type = source->typeExpr;
    next->suivant = NULL;
    next->adresse = 0;
    /* On vérifie s'il s'agit d'une variable pour lui donner une vraie adresse*/
    if(source->label == variable){

      if(global == 0){
        /* on gère différemment le cas selon si c'est une variable globale ou locale*/
        /* ici cas des variables locales*/
        next->adresse = compteurAdrLocale;
        compteurAdrLocale+=8;
        
      }
    /* cas des variables globales*/
      else{
        /* selon que ce soit un entier ou un char on incrémente un compteur diffénrent
        et on utilise une valeur différente
        */
        if(next->type == MY_INT){
          next->adresse = compteurAdrGlobInt;
        }
        else{
          next->adresse = compteurAdrGlobChar;
        }
        
        /* On gère différemment selon si c'est un caractère ou un entier*/
        if(source->typeExpr == MY_CHAR)
          compteurAdrGlobChar+=1;
        else{
          compteurAdrGlobInt+=4;
        }
      }
      
    }

    /* cas où on doit déterminer l'@ d'un paramètre */
    else if(source->label == parametre){
        
        /* cas où on tombe sur les paramètres de la fonction pour l'@*/

        if(global == 2){

        /* On gère de la même manière que l'argument soit un caractère ou un entier*/

            next->adresse = compteurAdrArg;

        /* on décrémente le compteur d'@ car plus l'élément arrive en premier plus son adresse est haute */

            compteurAdrArg-=8;

        }
    }

    next->type_rec = MY_VOID;
    tmp->suivant = next;
  }

  /* l'emplacement n'est pas utilisé on copie
  directement dans "dest"
  */
    else{
      dest->line = source->line;
      strcpy(dest->ident,source->valeur.text);
      dest->type = source->typeExpr;
      dest->adresse = 0;
      dest->suivant = NULL;
      dest->type_rec = MY_VOID;
      /* on ajoute l'@ correctement si c'est une variable*/

      if(source->label == variable){

        /* cas où on est sur une variable locale */
        if(global == 0){

          /* on gère différemment le cas selon si c'est une variable globale ou locale*/
          dest->adresse = compteurAdrLocale;
          compteurAdrLocale+=8;
          
        }

        /* cas où on choisit l'@ pour une variable globale*/
        else{
          
          /* On gère différemment selon si c'est un caractère ou un entier*/
          if(source->typeExpr == MY_CHAR){

            dest->adresse = compteurAdrGlobChar;
            compteurAdrGlobChar+=1;
          }

          else{

            dest->adresse = compteurAdrGlobInt;
            compteurAdrGlobInt+=4;

          }
        }
        
      }

      else if(source->label == parametre){

        /* cas où on tombe sur les paramètres de la fonction pour l'@*/
        if(global == 2){

        /* On gère de la même manière que l'argument soit un caractère ou un entier*/

            dest->adresse = compteurAdrArg;

        /* on décrémente le compteur d'@ car plus l'élément arrive en premier plus son adresse est haute */

            compteurAdrArg-=8;

        }
      }
    }
  return 1;

}

/* fonction de réallocation*/
/* renvoie 1 si tout s'est bien passé et 0 sinon*/
int realloue_tab_symb(TabSymb *tab_symb){

  int oldmax;
  int j;
  /* on récupère l'ancienne valeur du max pour pas écraser le contenu stocké par les anciens mallocs*/


  if(tab_symb->tab_fonc == NULL) {

    printf("Impossible d'allouer le tableau est null \n");

    return 0;

  }
  oldmax = tab_symb->max;

  /* on multiplie par 2 la capacité théorique */
   tab_symb->max*= 2;

   /* garder en mémoire le oldmax pour pas écraser au dessus ?? */

  /* on multiplie par 2 le nombres de fonctions possibles et on realloue "bêtement la deuxième partie */
  if((tab_symb->tab_fonc = (Variable ** ) realloc(tab_symb->tab_fonc, (tab_symb->max)*sizeof(Variable*))) == NULL){

    printf("erreur lors de l'allocation \n");

    return 0;

  }

  /* on réalloue pour de oldmax vers le nouveau max*/
  for(j = oldmax;j<(tab_symb->max);j++){


    if(   (tab_symb->tab_fonc[j] = (Variable*) malloc(sizeof(variable) * NB_VAR )   ) == NULL )  {


      /*tab_symb->tab_fonc[j] = (variable*) malloc(sizeof(variable) * NB_VAR)) == NULL)*/

        printf("erreur lors de l'allocation \n");
        return 0;
    }

    
  }

 

  return 1;

}

/* ajoute les fonctions de la librairie standard */

int addLibStandard(TabSymb * tab){
  /* variable qui récupère les infos d'une fonction dans la tab des symboles*/
  Variable * fun;
  Node nodeFunc;
  Node nodeArg1;
  Node nodeFunc2;
  Node nodeFunc3;
  Node nodeArg2;
  Node nodeFunc4;

  /* 1ÈRE PARTIE : GET INT ET PUTINT */

  /* un noeud pour représenter "la fonction" getint*/

  nodeFunc.line =0;
  nodeFunc.cat=TEXT;
  nodeFunc.typeExpr = MY_INT;
  nodeFunc.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeFunc.valeur.text == NULL){
    return -1;
  }

  strcpy(nodeFunc.valeur.text, "getint");
  nodeFunc.label = id;
  if((fun = findFonc(*tab , "getint")) != NULL){
      char msg[MAX_MSG_SIZE];
      strcpy(msg, "You added the globale function 'getint' twice ' ");
      semError(msg, fun->line);
      return 0;
  }
  /* on ajoute cette fonction à la tab des symboles*/
  if(addNodeInSymb(tab, &nodeFunc, NULL) == 0){
    semError("Error add of the function getint has failed",0);
    return 0;
  }

  tab->tab_fonc[tab->nb_fonc-1][FONCTION].type_rec = MY_INT;

  /* un noeud pour représenter "la fonction" putint*/

  
  nodeFunc2.line =0;
  nodeFunc2.cat=TEXT;
  nodeFunc2.typeExpr = MY_VOID;

  nodeFunc2.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeFunc2.valeur.text == NULL){
    return -1;
  }

  strcpy(nodeFunc2.valeur.text, "putint");
  nodeFunc2.label = id;

  if((fun = findFonc(*tab , "putint")) != NULL){
      char msg[MAX_MSG_SIZE];
      strcpy(msg, "You added the globale function 'putint' twice ' ");
      semError(msg, fun->line);
      return 0;
  }

  /* on ajoute cette fonction à la tab des symboles*/
  if(addNodeInSymb(tab, &nodeFunc2, NULL) == 0){
    semError("Error add of the function putint has failed",0);
    return 0;
  }

  /*on ajoute l'argument de putint*/

  
  nodeArg1.line =0;
  nodeArg1.cat=TEXT;
  nodeArg1.typeExpr = MY_INT;
  nodeArg1.label = parametre;

  nodeArg1.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeArg1.valeur.text == NULL){
    return -1;
  }
  /* on donne un nom à l'argument */
  strcpy(nodeArg1.valeur.text, "x");
  /* on remet le compteur d'adresse des arguments à 8*/
  compteurAdrArg = 8;
  if(copieNodeVersVariableForTabSymb(&tab->tab_fonc[tab->nb_fonc-1][ARGS], &nodeArg1, 2) == 0){
    semError("Error add of the argument of the function putchar has failed",0);
    return 0;
  }
  





  /* Deuxième partie : PUTCHAR ET GETCHAR*/



  /* un noeud pour représenter "la fonction" getchar*/

  nodeFunc3.line =0;
  nodeFunc3.cat=TEXT;
  nodeFunc3.typeExpr = MY_CHAR;
  nodeFunc3.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeFunc3.valeur.text == NULL){
    return -1;
  }

  strcpy(nodeFunc3.valeur.text, "getchar");
  nodeFunc3.label = id;
  if((fun = findFonc(*tab , "getchar")) != NULL){
      char msg[MAX_MSG_SIZE];
      strcpy(msg, "You added the globale function 'getchar' twice ' ");
      semError(msg, fun->line);
      return 0;
  }
  /* on ajoute cette fonction à la tab des symboles*/
  if(addNodeInSymb(tab, &nodeFunc3, NULL) == 0){
    semError("Error add of the function getchar has failed",0);
    return 0;
  }

  tab->tab_fonc[tab->nb_fonc-1][FONCTION].type_rec = MY_CHAR;


  /* un noeud pour représenter "la fonction" putchar*/

  
  nodeFunc4.line =0;
  nodeFunc4.cat=TEXT;
  nodeFunc4.typeExpr = MY_VOID;

  nodeFunc4.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeFunc4.valeur.text == NULL){
    return -1;
  }

  strcpy(nodeFunc4.valeur.text, "putchar");
  nodeFunc4.label = id;

  if((fun = findFonc(*tab , "putchar")) != NULL){
      char msg[MAX_MSG_SIZE];
      strcpy(msg, "You added the globale function 'putchar' twice ' ");
      semError(msg, fun->line);
      return 0;
  }

  /* on ajoute cette fonction à la tab des symboles*/

  if(addNodeInSymb(tab, &nodeFunc4, NULL) == 0){
    semError("Error add of the function putint has failed",0);
    return 0;
  }

  /*on ajoute l'argument de putint*/

  
  nodeArg2.line =0;
  nodeArg2.cat=TEXT;
  nodeArg2.typeExpr = MY_INT;
  nodeArg2.label = parametre;

  nodeArg2.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeArg2.valeur.text == NULL){
    return -1;
  }
   /* on donne un nom à l'argument */
  strcpy(nodeArg2.valeur.text, "x");

  /* on remet le compteur d'adresse des arguments à 8*/
  compteurAdrArg = 8;
  if(copieNodeVersVariableForTabSymb(&tab->tab_fonc[tab->nb_fonc-1][ARGS], &nodeArg2, 2) == 0){
    semError("Error add of the argument of the function putint has failed",0);
    return 0;

  }



  return 1;
}


/* ajoute une fonction pour les variables globales par défaut*/
int addGlobaleParDefaut(TabSymb * tab){
  /* un noeud pour représenter "la fonction"
  globale qui représente l'endroit où sont stockées les variables globales*/
  Node nodeGlobal;
  
  nodeGlobal.line =0;
  nodeGlobal.typeExpr=0;
  nodeGlobal.cat=TEXT;
  nodeGlobal.typeExpr = MY_VOID;
  nodeGlobal.valeur.text = (char *)malloc(sizeof(char)*20);
  if(nodeGlobal.valeur.text == NULL){
    return -1;
  }
  strcpy(nodeGlobal.valeur.text, "globale");
  nodeGlobal.label = id;
  return addNodeInSymb(tab, &nodeGlobal, NULL);
}


/*fonction qui donne les bonnes @ aux arguments dans la tab des symboles*/

void completerAdresseArgs(TabSymb * tab1){
  int adrArg;
  int i;
  Variable *tmp;
   
  /* on regarde dans la partie consacrée aux arguments de la fonction*/
  for(i=0;i<tab1->nb_fonc;i++){
      adrArg = 8;
      adrArg +=8*compteTotalArgFromPos(*tab1, i);
      for(tmp = &tab1->tab_fonc[i][ARGS]; tmp != NULL;  tmp=tmp->suivant){
      /* si une variable est et qu'elle n'est pas équivalente à "none", on lui donne une nouvelle @*/
          if(strcmp(tmp->ident, "\0") != 0 && tmp->type != MY_NONE){
            
            tmp->adresse = adrArg;
            /* on décrémente le compteur d'@ pour la prochaine variable*/
            adrArg-=8;
          }
      }
  }

}

#if 0
/*Fonction qui compte le nombre total de paramètre d'une fonction lors de sa définition
node : un des noeuds associé aux paramètres
compteur : compte le nombre de paramètre
*/
void compteurTotalParametre(Node * node, int * compteur){

  if(node == NULL){
    return;
  }

  /*printf("on tombe sur le noeud %")*/
  /* différents cas à traiter différemment (Début)*/

  if(node->label == typeRetour){

    compteurTotalParametre(node->nextSibling, compteur);
  }

  else if(node->label == id){

    compteurTotalParametre(node->firstChild, compteur);
  }

  /* cas où on tombe sur une déclaration de paramètre : on augmente le nombre de paramètre */
  else if(node->label == type){

    printf(" on tombe sur un paramètre \n\n");
    (*compteur) += 1;
    compteurTotalParametre(node->nextSibling, compteur);

  }

  else if(node->label == Parametres){

    compteurTotalParametre(node->firstChild, compteur);
  }

   /* différents cas à traiter différemment (FIN)*/
  

}
#endif

/*cette fonction ajoute un noeud associé à une variable ou une fonction dans la table
des symboles si il n'y est pas déjà
elle renvoie 0 si on ajoute deux fois la même variable
1 si tout s'est bien passé.
et elle renvoie -1 si il y a un échec d'allocation*/
int addNodeInSymb(TabSymb * tab, Node * node, Node * pere){
  Variable * var1;
  int hash;
  /*on tombe sur une fonction on l'ajoute à la tab*/

  if(node->label == id){

    /* cas où ce nom de fonction est déjà présent : on écrit un msg d'erreur */

    if(findFonc(*tab,node->valeur.text) != NULL){
      char msg[MAX_MSG_SIZE];
      strcpy(msg, "You're trying to add twice the function ' ");
      strcat(msg, node->valeur.text);
      strcat(msg,"' you cannot do it !! " );
      semError(msg, node->line);
      return 0;
    }

    /* on vérifie qu'une variable globale ne porte pas le même nom que la fonction*/

    else if((var1 = findGlobale(*tab, node->valeur.text)) != NULL){
        char msg[MAX_MSG_SIZE];
        char nbToChar[MAX_NUMBER_SIZE];
        strcpy(msg, "You're trying to create a function with the same name as a global variable : ' ");
        strcat(msg, node->valeur.text);
        strcat(msg, " ' !! (declared on Line) ");
        sprintf(nbToChar, "%d", var1->line);
        strcat(msg,  nbToChar);
        semError(msg, node->line);
        return 0;
    }

    tab->nb_fonc++;

    /* A chaque fois qu'on tombe sur une nouvelle fonction
    on remet le compteur d'@ à 8 pour la première variable de la fonction*/

    compteurAdrLocale = 8;

    /* A chaque fois qu'on tombe sur une nouvelle fonction
    on remet le compteur d'@ au max par rapport (début)*/

    /* on compte le nombre total d'argument de la fonction afin de déterminer l'@ de chaque paramètre par rapport à rbp*/
    compteurAdrArg = 8; 

    /* A chaque fois qu'on tombe sur une nouvelle fonction
    on remet le compteur d'@ au max par rapport (début)*/

    /*si on a dépassé le nombre maximum de fonctions
    autorisées on réalloue de la place*/
    if(tab->nb_fonc >= tab->max){
      if(realloueTabSymb(tab) == 0){
        printf("erreur échec de la réallocation \n\n");
        return -1;
      }
    }
    if(node->valeur.text == NULL){
      printf("votre fonction n'a pas de nom \n\n");
      return -1;
    }
    /* on copie les informations du noeud dans un emplacement de la tab*/
    return copieNodeVersVariable(&tab->tab_fonc[tab->nb_fonc-1][FONCTION], node);
  }
  
  /*on tombe sur une variable on l'ajoute à la tab*/
  else if(node->label == variable && node->typeExpr != MY_NONE && node->typeExpr != MY_NULL){
    /* on vérifie que la variable n'est pas déjà dans la tab des symboles*/
    /*if(findVariable(*tab , node->valeur.text, tab->nb_fonc-1) != NULL){
      printf("Erreur : Vous voulez ajouter deux fois la même variable \n\n");
      return 0;
    }*/
    /*int retour;*/
    /* on récupère le hashcode associé à ce nom de variable*/
    hash = hashCode(node->valeur.text);
    /*on copie les infos du noeud dans l'emplacement associé à ce hash*/
    /* cas on ajoute une variable globale*/
    if(tab->nb_fonc == GLOBALE+1) {
      return copieNodeVersVariableForTabSymb(&tab->tab_fonc[tab->nb_fonc-1][hash], node, 1);
    }
    /* cas on ajoute pas une variable globale*/
    else{
      /* la variable était déjà présente en tant qu'argument ou variable */
      if(findVariable(*tab, node->valeur.text, tab->nb_fonc-1) != 0){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, "You're trying to add twice the variable : ' ");
        strcat(msg, node->valeur.text);
        strcat(msg, " ' !!");
        semError(msg, node->line);
        return 0;
      }
      
      return copieNodeVersVariableForTabSymb(&tab->tab_fonc[tab->nb_fonc-1][hash], node, 0);
    }
    /*copieNodeVersVariable(&tab->tab_fonc[tab->nb_fonc-1][hash], node);*/
  }

  /*on tombe sur un paramètre on l'ajoute à la tab*/
  else if(node->label == parametre){
    /* on vérifie que la variable n'est pas déjà dans la tab des symboles*/
    
    if(findVariable(*tab,node->valeur.text,tab->nb_fonc-1) != NULL){
      char msg[MAX_MSG_SIZE];
      printf("erreur double variable \n\n");
      strcpy(msg, "You're trying to add twice the same parameter ' ");
      strcat(msg, node->valeur.text);
      strcat(msg, " ' !!");
      semError(msg, node->line);
      return 0;
    }
    
    /* l'emplacement où sont stockées tous les paramètres est le deuxième emplacement du tableau (1) */
    return copieNodeVersVariableForTabSymb(&tab->tab_fonc[tab->nb_fonc-1][ARGS], node,2); 
  }
  

  /* le noeud ne correspond ni
  à une variable ni à une fonction*/
  return 1;
}

/*
Fonction qui vérifie les affectations à la déclaration spécifiquement car les éléments données peuvent ne pas être valides
elle renvoie :
1: si tout s'est bien passé
0 : cas d'erreur 
Noeud correspond au noeud que l'on analyse et pour lequel on va vérifier qu'il est bien dans 
la tab des symboles "tab".
père correspond au noeud "=" correspond à l'affectation.
Cette fonction part du présupposé qu'on démarre à partir du fils du noeud qui reçoit l'affectation.
Dans le cas contraire, elle ne sera pas fonctionnelle
*/
int checkAffectationPendantRemplir(Node * pere, Node * node, TabSymb tab){
  
  if(node == NULL)
    return 1;
  /* on lance la fonction auxiliaire pour traiter la valeur droite */
  if(node->label == variable && pere != NULL && pere->label == affectation ){
      return checkAffectationPendantRemplirAux(node,node->firstChild,tab);
  }
    
  return 1;
}

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
int checkAffectationPendantRemplirAux(Node * leftValue, Node * node, TabSymb tab){
  
  if(node == NULL)
    return 1;
  
  if(node->label == variable){
   
    /*on vérifie la variable utilisée dans la dernière fonction*/
    if(findVariable(tab, node->valeur.text,tab.nb_fonc-1) == NULL && findGlobale(tab, node->valeur.text) == NULL){
      
      /*si cette variable n'est pas encore présente, on renvoie un code d'erreur*/
      char msg[MAX_MSG_SIZE];
      
      strcpy(msg, "You tried to affect the variable '");
      strcat(msg, node->valeur.text);
      strcat(msg, "' to the variable ' ");
       strcat(msg, leftValue->valeur.text);
      strcat(msg, "' but the right value variable doesn't exist yet !!");
     
      semError(msg, node->line);
      /* cas d'erreur */
      return 0;
      
    }
    
  }
  
  /*on vérifie que tout s'est bien passé chez les enfants et les frères si il y a plusieurs valeurs droites*/
  if(checkAffectationPendantRemplirAux(leftValue, node->nextSibling,tab) == 0)
    return 0;
  if(checkAffectationPendantRemplirAux(leftValue, node->firstChild,tab) == 0)
    return 0;
  /* tout s'est bien passé*/
  
  return 1;
}

/*fonction qui remplie la table des symboles en analysant l'arbre
elle renvoie 0 en cas d'erreur et 1 si tout s'est bien passé
le noeud père correspond au noeud parent de "node" */
int remplirTabSymb(TabSymb * tab, Node * node, Node *pere){
    int retour;
   /* si on tombe sur un noeud null, on arrête le parcours*/
    if(node == NULL){
        return 1;
    }
  /* on traite d'abord les frères avant le noeud lui-même pour résoudre le cas
  particulier des affectations à la déclaration */

  if(node->label == variable){
    
    retour = remplirTabSymb(tab, node->nextSibling, node);
    /* cas où on ajoute 2 fois le même identificateur */
  }

  retour = addNodeInSymb(tab, node, pere);
  /* cas où on ajoute 2 fois le même identificateur */
  if(retour == 0){
    return 0;
  }
  /* échec d'allocaton*/
  if(retour == -1){
    return 3;
  }
  /* cas où on tombe sur une affectation, on vérifie les affectations à la déclaration */
  if(node->label == variable && pere != NULL && pere->label == affectation){
    if(checkAffectationPendantRemplir(pere,node,*tab) == 0)
      return 0;
  }
  /*on continue le parcours pour le reste*/
  retour = remplirTabSymb(tab, node->firstChild, node);
  /* cas où on ajoute 2 fois le même identificateur */
  if(retour == 0){
    return 0;
  }
  /* échec d'allocaton*/
  if(retour == -1){
    return 3;
  }
  if(node->label != variable){
    retour = remplirTabSymb(tab, node->nextSibling, node);
    /* cas où on ajoute 2 fois le même identificateur */
  }
  if(retour == 0){
    return 0;
  }
  /* échec d'allocaiton*/
  if(retour == -1){
    return 3;
  }

  return 1;


    
}


/* fonction qui initalise la table des symboles, notamment en faisant les mallocs nécessaires 
pour celle-ci.*/
int init_tab_symb(TabSymb * tab){
    int i;
    int j;
    /* on ajoute l'espace pour remplir le tableau de tab : on fait :
    une allocation pour toutes les fonctions qu'il peut ajouter.*/
   
        
    tab->tab_fonc = (Variable**) malloc(NB_FONC_INIT * sizeof(Variable*));

    if(tab == NULL){
        printf("Échec de l'allocation du tableau en deux dimensions de la table des symboles \n");
    }
    /*on indique le nombre maximum de fonction acceptable*/
    tab->max = NB_FONC_INIT;
    /* il y a pour l'instant, 0 fonction dans la tab des symboles*/
    tab->nb_fonc = 0;

    /*Une allocation pour le nombre de variables que chaque fonctions peut ajouter*/

    for(i=0;i<NB_FONC_INIT;i++){

        tab->tab_fonc[i] = (Variable*) malloc(sizeof(Variable)*NB_VAR);

    }

    /* par défaut les variables n'ont pas de suivant et elles n'ont pas de noms*/
    
     for(i=0;i<NB_FONC_INIT;i++){
        for(j=0;j<NB_VAR;j++){

            tab->tab_fonc[i][j].suivant = NULL;
            tab->tab_fonc[i][j].type = MY_NONE;
            tab->tab_fonc[i][j].ident[0] = '\0';
            tab->tab_fonc[i][j].type_rec = -1; /* par défaut aucun type n'a été reçu*/
            tab->tab_fonc[i][j].line = 0; /* par défaut aucun ligne n'est associé à l'endroit où on a déclaré la variable */
            tab->tab_fonc[i][j].adresse = 0; /* par défaut, pas d'@ n'est associée à la variable*/
            
            /* la case 0 correspond tjrs réservé à l'identicateur associé à la fonction et ses propriétés*/
            if(j == 0)
                tab->tab_fonc[i][j].cat = MY_FUNCTION;
            /* la case 1 correspond aux paramètres : c'est une liste chainée de paramètre */
            else if(j==1)
                tab->tab_fonc[i][j].cat = MY_PARAMETRE;
            /* les cases qui suivent sont nécessairement pour des variables*/
            else
                tab->tab_fonc[i][j].cat = MY_VARIABLE;
            

        }

    }
    return 1;

}

/* cette fonction crée un hashcode en fonction du nom d'une variable*/
/* elle renvoie une valeur qui correspond au hashcode*/
/* elle renvoie -1 si l'élément donné était une chaine vide*/
int hashCode(char * nom_var){
  int length;
  /* enregiste la somme des caractères en ascii*/
  int somme;
  int i;
  if(nom_var == NULL){
    return -1;
  }
    
  
  length = strlen(nom_var);
  /* échec, chaine vide*/
  if(length <=0)
    return -1;
  somme = 0;
  for(i = 0;i < length;i++){
    /* on additionne les valeurs ascii des lettres de la variable pour créer un hashcode*/
    somme += nom_var[i];
  }

  /* on renvoie un code qui peut être inclus dans les cases du tableau*/
  /* cas où somme risque d'être trop grand, on n'ajoute pas +2 pour le hashcode*/
  if(somme == NB_VAR-1)
    return somme % NB_VAR;
  /* on commence à ajouter les variables à la case 2 donc 2 + mod valeur : les deux premières cases sont prices par le
  nom de la fonction elle-même, et aussi les variables qui sont chainées, les paramètres de la fonction */
  return (VARIABLES+ somme % NB_VAR);
}

/* Add , find, delete */

/*

Trouve le type d'une variable dans la table des symboles en fonction de la fonction dans 
laquelle on se situe

renvoie le type trouvé 
si la variable n'est pas dans la table, on renvoie -1

*/

int findTypeVar(TabSymb tab, char * nom_var, char * nomFonc){
  Variable * tmp;

  /* on vérifie que le nom de la variable existe*/
  if((tmp = findVariable(tab, nom_var, findFoncPos(tab, nomFonc))) == NULL && (tmp = findGlobale(tab, nom_var)) == NULL){
    printf("Erreur : la variable : %s a été appelée alors qu'elle n'existe pas \n", tmp->ident);
    /* la variable a été utilisé lors de l'appel alors qu'elle n'existe pas*/
    return -1;
  }
  /*on renvoie le type de la variabl*/
  return tmp->type;
}

/* 
Cette fonction indique si une variable a déjà été ajouté parmi les variables globales
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie NULL sinon*/
Variable * findGlobale(TabSymb tab_symb , char * nom_var){
  int code;
  Variable * tmp;
  code = hashCode(nom_var);
  /* On regarde d'abord au niveau de la table consacrée aux variables globales*/
  tmp = &tab_symb.tab_fonc[GLOBALE][code];

  /* cas où l'emplacement n'est pas utilisé*/

  if(tmp->type == MY_NONE){
    return NULL;
  }

  /* la variable est là c'est celle en tête de liste*/

    if(strcmp(tmp->ident, nom_var) == 0){
        return tmp;
    }

  /* cas où l'emplacement suivant est utilisé mais on ne sait pas si il contient la variable
  car le premier maillon de la chaine ne contient pas ce nom de variable.
  On avance dans la liste chainée.*/

  while(tmp != NULL){
  /* cette variable est présente parmi les globlaes */
    if(strcmp(tmp->ident, nom_var) == 0){
      return tmp;
    }
    /* on avance dans la recherche car on ne l'a pas trouvée */
    tmp = tmp->suivant;
  }

    return NULL;
}


/* 
Cette fonction indique si cette variable a déjà été ajouté pour cette fonction
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie null sinon
pos = fonction de référence : fonction appelante */
Variable * findVariable(TabSymb tab_symb , char * nom_var, int pos){

  Variable *tmp;

  /* enregistre le hashcode*/
  int code;
    
  code = hashCode(nom_var);


  /* on commence par regarder les arguments de la fonction, c'est à dire, dans la liste chainée des variables qui sont à la première case associée au numéro de la fonction.
  Par définitioon la fonction "globale" (le fichier) n'a pas d'arguments donc il n'est pas nécessaire d'aller ici */
  if(pos != GLOBALE){
    for(tmp =  &tab_symb.tab_fonc[pos][ARGS]; tmp != NULL;  tmp=tmp->suivant){
      /* si une variable est présente à cet emplacement, on tente de comparer son nom avec celui recherché*/
      if(strcmp(tmp->ident, "\0") != 0 && tmp->type != MY_NONE){
        /*printf("l'emplacement en arg de la fonction a été trouvé \n");*/
        if(strcmp(tmp->ident, nom_var) == 0){
            /* la variable a été trouvée, on la renvoie pour l'attester*/
            return tmp;
        }
      }
    }
  }
  
 
  
  /* finalement on regarde au niveau des variables locales*/
  tmp = &tab_symb.tab_fonc[pos][code];

  /* cas où l'emplacement n'est pas utilisé*/
  if(tmp->type == MY_NONE){
    return NULL;
  }
 
  /* la variable est déjà là c'est celle en tête de liste*/
    /*printf(" le contenu de tmp : %s " ,tmp->nom);
    printf(" le contenu de nom_var : %s " ,nom_var);*/
    if(strcmp(tmp->ident, nom_var) == 0){
        return tmp;
    }

  
  /* cas où l'emplacement suivant n'est pas utilisé*/
  while(tmp != NULL){
    /* cette variable est déjà utilisée car on retrouve son nom on arrête tout */
    if(strcmp(tmp->ident, nom_var) == 0){
      return tmp;
    }
    /* on avance dans la recherche*/
    tmp = tmp->suivant;
  }
  
  /* la variable n'a pas été trouvée, on renvoie 0*/
  return NULL;
}

/* 
Cette fonction indique si l'emplacement d'une fonction dans la tab des symboles.
Elle renvoie NULL si la fonction n'a pas été trouvée.
Sinon elle renvoie la fonction en elle même où se trouve la fonction  */

Variable * findFonc(TabSymb tab_symb , char * nomFonc){
  int i;
  /* dans le cas où le user ne donne pas de fonction en argument 
  on part du principe qu'il veut traiter les variables globales*/
  if(nomFonc == NULL){
    nomFonc = "globale";
  }
  
   for(i=0;i<tab_symb.nb_fonc;i++){
     /* On cherche dans tous les emplacements associés aux noms de fonctions, sauf celui qui porte sur la fonction appelée globale 
     car ce n'est pas une vraie fonction seulement le nom générique pour désigner tout le fichier*/
    if(i != GLOBALE){
      /* cas où le nom de la fonction a été trouvé parmi les noms des fonctions */
      if(strcmp(tab_symb.tab_fonc[i][FONCTION].ident, nomFonc) == 0){
          /*printf("la fonction %s a été trouvée ! \n", nomFonc);*/
          return &tab_symb.tab_fonc[i][FONCTION];
      }
    }
  }
  /*printf("la fonction %s n'a pas été trouvée \n\n", nomFonc);*/
  /* la fonction n'a pas été trouvée, on renvoie -1, c'est une erreur*/
  return NULL;
}

/* 
Cette fonction indique l'emplacement d'une fonction dans la tab des symboles.
Elle renvoie -1 si la fonction n'a pas été trouvée.
Sinon elle renvoie la position en elle même où se trouve la fonction  */

int findFoncPos(TabSymb tab_symb , char * nomFonc){
  int i;
   for(i=0;i<tab_symb.nb_fonc;i++){
     /* On cherche dans tous les emplacements associés aux noms de fonctions, sauf celui qui porte sur la fonction appelée globale 
     car ce n'est pas une vraie fonction seulement le nom générique pour désigner tout le fichier*/
    if(i != GLOBALE){
      /* cas où le nom de la fonction a été trouvé parmi les noms des fonctions */
      if(strcmp(tab_symb.tab_fonc[i][FONCTION].ident, nomFonc) == 0){
          /*printf("la fonction %s a été trouvée ! \n", nomFonc);*/
          return i;
      }
    }
  }
  
  /* la fonction n'a pas été trouvée, on renvoie -1, c'est une erreur*/
  return -1;
}


/* affiche le type d'une variable ou le type de retour d'une fonction*/
void show_type(int type){
  if(type == MY_CHAR){
    printf("char");
  }

  else if(type == MY_INT){
    printf("int");
  }

  else if(type == MY_VOID){
    printf("VOID");
  }
}

/* affiche une liste chainée de variable*/
void showListeVar(Variable * var){

    Variable *tmp;

    /* cas où cet emplacement n'a pas été utilisé*/

    if(var->type == MY_NONE){
        return;
    }

    tmp = var;
    
    /* tant qu'on est pas sur le dernier élément on avance*/
    while(tmp != NULL) {
        printf("var : type : ");
        show_type(tmp->type);
        printf("  \n");
        printf("var : nom : %s \n",tmp->ident);
        printf("var : adresse: %d \n",tmp->adresse);
        printf("var : ligne : %d \n", tmp->line);
        tmp = tmp->suivant; 
    }

}








/* affiche la table des symboles*/
void showTabSymb(TabSymb tab_symb){

    int i;
    int j;
    
    printf("le nombre de fonction est : %d \n", tab_symb.nb_fonc-1);
    
    /*on affiche les variables et leurs types pour chaque fonction que l'on possède*/
   for(i = 0; i< tab_symb.nb_fonc;i++){

        /* on affiche pas la fonction "globale" qui n'est pas une vraie fonction*/
        if(i != GLOBALE){
          /* à cause de la fonction globale on doit décrémenter de 1*/
          if(i > GLOBALE)
            printf("nous sommes à la fonction numéro : %d \n" ,i-1);

          else if(i < GLOBALE)
            printf("nous sommes à la fonction numéro : %d \n" ,i);

         

          printf("fonc, type retour attendu: " );
          show_type(tab_symb.tab_fonc[i][FONCTION].type);
          
          printf("  \n");
          printf("fonc, type retour Reçu: " );
          show_type(tab_symb.tab_fonc[i][FONCTION].type_rec);
          printf("  \n");
          printf("fonc, nom : %s \n", tab_symb.tab_fonc[i][FONCTION].ident);
          printf("fonc, adresse mémoire nasm: %d \n", tab_symb.tab_fonc[i][FONCTION].adresse);
          printf("fonc, ligne de déclaration : %d \n", tab_symb.tab_fonc[i][FONCTION].line);

        }
        else{
            printf("les variables globales : \n");
        }
        for(j =1;j<NB_VAR;j++){

          /* on précise que l'on va afficher les arguments tant que j vaut 2*/
          
          if(i == GLOBALE){

           /*on fait rien */

          }

          else{

              if(j == ARGS){

                printf("La liste des arguments de la fonction \n");

              }

            /* on précise que l'on va afficher les variables pour tous les j supérieurs à 2*/

              if(j == VARIABLES){

                printf("La liste des variables de la fonction \n");

              }
          }

            
              showListeVar(&tab_symb.tab_fonc[i][j]);
        }
        

    }
}




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

int checkVariableParametresAppel(Node * node, TabSymb tab, char * currFonc ){
  
  Node * filsG;
  Node * frereD;
  int posFonc;

  if(node == NULL){
    return 1;
  }

  filsG = node->firstChild;
  frereD = node->nextSibling;
  /*on tombe sur une fonction */
  if(node->label == id){

    /* on parcours le reste des noeuds en précisant qu'on est dans la fonction
    valeur.text.
    On renvoie 0 si il y a eu une erreur */
    if(checkVariableParametresAppel(filsG,tab ,node->valeur.text) == 0)
      return 0;
    if(checkVariableParametresAppel(frereD,tab ,node->valeur.text) == 0)
      return 0;
      /* tout s'est bien passé */
    return 1;
  }
  if(node->label == variable){ 

    posFonc = findFoncPos(tab , currFonc);
    /* dans le cas où la fonction n'aurait pas été
    trouvée on part du principe qu'on est dans le cas des variables globales*/
    if(posFonc == -1){
      posFonc = 0;
    }
    
    if(findVariable(tab, node->valeur.text,posFonc) == NULL && findGlobale(tab, node->valeur.text) == NULL){
      /* cas d'erreur la variable n'existe pas pour cette fonction
      ni pour les variables globales*/
      
      char messageErr[MAX_MSG_SIZE];
      strcpy(messageErr, "La variable '");
      strcat(messageErr, node->valeur.text);
      strcat(messageErr, "' n'existe pas");
      /* on envoie un message d'erreur car la variable n'existe pas*/
      semError(messageErr, node->line);
      
      /* on renvoie la valeur d'erreur */
      return 0;
      
    }
  }

  /* on parcours le reste des noeuds en précisant qu'on est dans la fonction
    valeur.text.
    On renvoie 0 si il y a eu une erreur */
    if(checkVariableParametresAppel(filsG,tab ,currFonc) == 0)
      return 0;
    if(checkVariableParametresAppel(frereD,tab ,currFonc) == 0)
      return 0;
  
  return 1;
}

/*
Fonction qui teste les appels de fonction.
Elle renvoie 2 en cas de warning (int remplace char).
renvoie 1 si tout s'est bien passé.
renvoie 0 en cas d'erreur (pas suffisamment d'arguments, fonction inconnue)
on part de la racine et on analyse tout l'arbre
currFonc indique la fonction dans laquelle on est actuellement, la fonction appelante
*/
int checkAppelFonc(Node * node, char * currFonc, TabSymb tab){
  int retour;
  int retour2;
  /* par défaut tout s'est bien passé*/
  retour = 1;
  if(node == NULL)
    return 1;
  /* cas où on entre dnas une nouvelle fonction : on met à jour les infos*/
  if(node->label == id){
    /* en cas d'erreur, on arrête toute l'analyse*/
     if((retour = checkAppelFonc(node->firstChild, node->valeur.text, tab)) == 0)
      return retour;
    else if((retour2 = checkAppelFonc(node->nextSibling, node->valeur.text, tab)) == 0){
      return 0;
    }
    else{
      if(retour2 == 2){
        retour = retour2;
      }
    }

    return retour;

  }
  if(node->label == appelFonction){
    char msg[MAX_MSG_SIZE];
    /* utilisation de la fonction main lors d'un appel : erreur */
    if(strcmp(node->valeur.text, "main") == 0){
      strcpy(msg, " the function ' ");
      strcat(msg, node->valeur.text);
      strcat(msg, " is used for a function call, you can't do it because it's the main ! ");
      /* on écrit un message d'erreur */
      semError(msg, node->line);
      return 0;
    }
    retour2 = checkArgFonc(NULL, node, currFonc, tab, NULL);
    if(retour2 == 0){
      return retour2;
    }
    
  }
  /* on continue le test sur le reste de l'arbre*/
  if((retour2 = checkAppelFonc(node->firstChild, currFonc, tab)) == 0){
   /* en cas d'erreur, on arrête toute l'analyse*/
    return retour2;
  }
  /*on update le nieau d'alerte*/
  else if(retour2 == 2 && retour == 1){
    retour = retour2;
  }
  if((retour2 = checkAppelFonc(node->nextSibling,currFonc, tab)) == 0){
    /* en cas d'erreur, on arrête toute l'analyse*/
    return 0;
  }
  /*on update le nieau d'alerte*/
  else if(retour2 == 2 && retour == 1){
    retour = retour2;
  }

  


  return 1;
}





/* cette fonction envoie un message en cas d'erreur
sémantique. Elle précise la ligne*/
void semError(char * msg, int line){
	fprintf(stderr, "\033[1;31mSEMANTICAL ERROR \033[0m: %s | \033[1mLine : %d\033[1m | \n\n \033[0m",  msg, line);
}


/* cette fonction envoie un message en cas d'erreur
sémantique. Elle ne précise pas de ligne*/
void semError2(char * msg){
	fprintf(stderr, "\033[1;31mSEMANTICAL ERROR \033[0m: %s \n\n \033[0m",  msg);
}
/* cette fonction envoie un warning en cas de problème sémantique
sémantique. Elle précise la ligne*/
void semWarning(char * msg, int line){
	fprintf(stderr, "\033[0;34mWarning\033[0m: %s | \033[1mLine : %d\033[1m | \n\n \033[0m",  msg, line);
}


/*
on initialise le tableau de variables*/
void initTabVar(tabVariable * tabVar){
  /* par défaut une fonction a 0 variable */
  tabVar->nbVar = 0;
  tabVar->nbInt = 0;
  tabVar->nbChar = 0;
  tabVar->nbVoid = 0;
}

/* fonction qui envoie le contenu de la varaible orig vers dst */
void copieVar(Variable *dst, Variable orig){
  dst->adresse = orig.adresse;
  dst->cat = orig.cat;
  dst->line = orig.line;
  strcpy(dst->ident,orig.ident);
  dst->type = orig.type;
  dst->type_rec = orig.type_rec;
}
/*
transforme un noeud associé à une constante en cas particulier de variable.
dst reçoit, orig envoie.
*/
void createVariableFromConstNode(Variable * dst, Node orig){
  dst->adresse = 0;
  dst->cat = MY_CONST;
  dst->line = orig.line;
  if(orig.cat == CAR){ /* cas on on tombe sur un caractère*/
    dst->ident[0] = orig.valeur.car;
    dst->ident[1] = '\0';
  }
  else /* cas où on est sur un entier */
    sprintf(dst->ident, "%d", orig.valeur.entier);
  if(orig.cat == CAR)
    dst->type = MY_CHAR;
  else
    dst->type = MY_INT;
  /*
  if(dst->type == MY_INT)
    
  else if(dst->type == MY_CHAR)
  */
}
/* fonction d'ajout de variables dans la table 
des variables */
void ajoutVariableTabVar(tabVariable * tabVar, Variable var1){

  /* on vérifie qu'on a pas plus d'arguments que le tableau ne peut en contenir*/
  if(tabVar->nbVar >= MAX_ARGS || tabVar->nbVar < 0){
    printf("Erreur nombre maximum d'argument pour une fonction atteint ( 50), impossible d'en vérifier davantage \n\n");
    return;
  }

  /* on copie les info de la variable vers le tableau */

  copieVar(&tabVar->tab[tabVar->nbVar],var1);

  /*on a ajouté une variable dans la tab donc on incrémente */
  tabVar->nbVar+=1;

  if(var1.type == MY_INT){
    tabVar->nbInt+=1;
  }

  else if(var1.type == MY_CHAR){
    tabVar->nbChar+=1;
  }

  else if(var1.type == MY_VOID){
    tabVar->nbVoid+=1;
  }

}

/*
node contient le noeud associé à la fonction qui nous intéresse (fonction appelée).
Deux tableaux: 
tabArgAtt correspond au tableau des arguments attendus
tabArgRec correspond au tableau des arguments reçus 
renvoie 2 en cas de warning (int remplace char).
renvoie 1 si tout s'est bien passé.
renvoie 0 en cas d'erreur (pas suffisamment d'arguments) ou reçoit un void
*/
int compareArgsAttRecu(Node node, tabVariable tabArgAtt, tabVariable tabArgRec){
  char msg[MAX_MSG_SIZE];
  char nbToChar[MAX_NUMBER_SIZE];
  char nbToChar2[MAX_NUMBER_SIZE];
  int i;
  /* correspond à la valeur de retour*/
  int retour;

  /* par défaut tout s'est bien passé */
  retour = 1;

  /* cas d'erreur 1 : pas assez d'arguments */
  if(tabArgAtt.nbVar != tabArgRec.nbVar){
    
    strcpy(msg, " the function ' ");
    strcat(msg, node.valeur.text);
    strcat(msg, " ' expected ' ");
    sprintf(nbToChar, "%d", tabArgAtt.nbVar);
    sprintf(nbToChar2, "%d", tabArgRec.nbVar);
    strcat(msg, nbToChar);
    strcat(msg,  " ' arguments but ' ");
    strcat(msg,  nbToChar2);
    strcat(msg,  " ' were given ! ");
    /* on écrit un message d'erreur */
    semError(msg, node.line);
    /*printf(" le dernier argument donné est : %s \n\n",tabArgRec.tab[tabArgRec.nbVar-1].ident);*/
    return 0;
  }
  else{
    /* deuxième cas arguments de types différents : warning*/

    for(i = 0;i<tabArgAtt.nbVar;i++){
      if(tabArgAtt.tab[i].type == MY_CHAR && tabArgAtt.tab[i].type != tabArgRec.tab[i].type && tabArgRec.tab[i].type == MY_INT){
        /* on tombe sur un warning car on attend un char et on a reçu un int*/
        retour = 2;

        strcpy(msg, " the function ' ");
        strcat(msg, node.valeur.text);
        strcat(msg, " ' expected ");
        strcat(msg, " a 'char' for the argument number '");
        sprintf(nbToChar, "%d", (i+1));
        strcat(msg, nbToChar);
        strcat(msg,  "'  but an 'int' ");
        strcat(msg,  " were given ! ");
        /* on écrit un message d'erreur */
        semWarning(msg, node.line);
        /*printf(" le dernier argument donné est : %s \n\n",tabArgRec.tab[tabArgRec.nbVar-1].ident);
        printf(" le premier argument donné est : %s \n\n",tabArgRec.tab[0].ident);*/

      }

      else if(tabArgAtt.tab[i].type == MY_CHAR && tabArgAtt.tab[i].type != tabArgRec.tab[i].type && tabArgRec.tab[i].type == MY_VOID){
        /* on tombe sur un warning car on attend un char et on a reçu un int*/
        retour = 0;

        strcpy(msg, " the function ' ");
        strcat(msg, node.valeur.text);
        strcat(msg, " ' expected ");
        strcat(msg, " a 'char' for the argument number '");
        sprintf(nbToChar, "%d", (i+1));
        strcat(msg, nbToChar);
        strcat(msg,  "'  but a 'void' ");
        strcat(msg,  " were given ! ");
        /* on écrit un message d'erreur */
        semError(msg, node.line);
        /*printf(" le dernier argument donné est : %s \n\n",tabArgRec.tab[tabArgRec.nbVar-1].ident);
        printf(" le premier argument donné est : %s \n\n",tabArgRec.tab[0].ident);*/

      }

      else if(tabArgAtt.tab[i].type == MY_INT && tabArgAtt.tab[i].type != tabArgRec.tab[i].type && tabArgRec.tab[i].type == MY_VOID){
        /* on tombe sur une erreur car on attend un int et on a reçu un void*/
        retour = 0;

        strcpy(msg, " the function ' ");
        strcat(msg, node.valeur.text);
        strcat(msg, " ' expected ");
        strcat(msg, " a 'int' for the argument number '");
        sprintf(nbToChar, "%d", (i+1));
        strcat(msg, nbToChar);
        strcat(msg,  "'  but a 'void' ");
        strcat(msg,  " were given ! ");
        /* on écrit un message d'erreur */
        semError(msg, node.line);
        /*printf(" le dernier argument donné est : %s \n\n",tabArgRec.tab[tabArgRec.nbVar-1].ident);
        printf(" le premier argument donné est : %s \n\n",tabArgRec.tab[0].ident);*/

      }

      else if(tabArgAtt.tab[i].type == MY_INT && tabArgRec.tab[i].type == MY_NONE){
        /* on tombe sur un warning car on attend un int et on a reçu un none*/
        retour = 0;

        strcpy(msg, " the function ' ");
        strcat(msg, node.valeur.text);
        strcat(msg, " ' expected ");
        strcat(msg, " a 'int' for the argument number '");
        sprintf(nbToChar, "%d", (i+1));
        strcat(msg, nbToChar);
        strcat(msg,  "' but a group of arguments that represents a 'none type' has been given ");
        /* on écrit un message d'erreur */
        semError(msg, node.line);

      }

      else if(tabArgAtt.tab[i].type == MY_CHAR && tabArgRec.tab[i].type == MY_NONE){
        /* on tombe sur un warning car on attend un int et on a reçu un none*/
        retour = 0;

        strcpy(msg, " the function ' ");
        strcat(msg, node.valeur.text);
        strcat(msg, " ' expected ");
        strcat(msg, " a 'char' for the argument number '");
        sprintf(nbToChar, "%d", (i+1));
        strcat(msg, nbToChar);
        strcat(msg,  "' but a group of arguments that represents a 'none type' has been given ");
        /* on écrit un message d'erreur */
        semError(msg, node.line);

      }
    }
  }
  

  return retour;
}
/*
Cette fonction donne les arguments attendus par la fonction de nom "name"
et les ajoutes dans tabVar
*/
void retrieveParametreAttendu(char * name, TabSymb tab_symb, tabVariable * tabVar){
  /* cette variable va permettre de traier chaque argument et de l'ajouter à tabVar en faisant un parcours*/
  Variable * currentArg;
  /* cette variable récupère l'emplacmeent de la fonction dans la tab des symboles */
  int posFonc;
  /* on arrête tout dans le cas où l'argument pour le tableau de variable vaut NULL*/
  if(tabVar == NULL)
    return;
  /* on récupère la position de la fonction */
  posFonc = findFoncPos(tab_symb , name);
  /* on récupère */

  currentArg = &tab_symb.tab_fonc[posFonc][ARGS];
  /* On ajoute chaque argument de la liste chainée des arguments tant qu'il y en a */
  while(currentArg != NULL && currentArg->type != MY_NONE){
    ajoutVariableTabVar(tabVar, *currentArg);
    currentArg = currentArg->suivant;
  }
}


/* fonction qui vérifie le type et le nombre des arguments pour un appel de fonction.
-l'argument est un noeud, au départ ce sera le noeud associé à la fonction
-le name permet de savoir dans quel fonction on est.
-le deuxième argument est la tab des symboles cela permet d'avoir des infos
-le troisième est un tableau de variable qui contient l'ensemble des arguments utilisées
par la fonction.
sur la fonction traitée et ses arguments
renvoie 1 si tout s'est bien passé
renvoie 0 en cas d'erreur */
int checkArgFonc(Node * pere, Node * node, char * name, TabSymb tab1, tabVariable * tabVar){
  /* récupère la valeur de retour */
  int retour;
  int retourTmp;
  /* par défaut tout s'est bien passé */
  retour = 1;
  /* on traite un noeud vide, cas d'arrêt*/
  if(node == NULL){
    return 1;
  }

  /* dans le cas où on agglomère un ensemble de valeur pour former un seul argument, on va simplement incrémenter le compteur avec le type de l'argument*/
  
  if(pere != NULL && pere->label == operateur && node->label != operateur && node->nextSibling == NULL && tabVar != NULL){
      Variable * var1 = NULL;
      /* cas où on a un appel de fonction en argument d'appel de fonction dans une agglomération*/
      if(node->label == appelFonction){
        var1 = findFonc(tab1, node->valeur.text);
        if(var1 != NULL){
          gatherTypeExpr(*var1,tabVar);
        }
        /* fonction introuvable : erreur */
        else{
          return 0;
        }
      }
      /* cas où on a une variable en argument d'appel de fonction dans une agglomération*/
      else if(node->label == variable){
        int posFonc = findFoncPos(tab1, name);
        /* cas d'erreur fonction appelante introuvable */
        if(posFonc == -1){
          return 0;
        }
        var1 = findVariable(tab1, node->valeur.text, posFonc);
        if(var1 != NULL){
          gatherTypeExpr(*var1,tabVar);
        }
        /* variable introuvable :erreur */
        else{
          return 0;
        }
      }

      /* cas où on tombe sur une constante *en argument d'appel de fonction dans une agglomération*/
      else if((node->label == character && tabVar != NULL) || (node->label == num && tabVar != NULL)){
        Variable tmp;
        /* on agglomère la constante aux arguments trouvés*/
        createVariableFromConstNode(&tmp, *node);
        gatherTypeExpr(tmp,tabVar);
        return 1;
      }

      return 1;
  }
      
  
  /* cas où on est sur un appel de fonction : on va enregister ses arguments */
  if(node->label == appelFonction){
     /* tab des arguments récupérés*/
    tabVariable tabArgRecup;
    /* on va ajouter ses paramètres pour comparer ce qui est attendu avec qu'on nous a donné en entrée */
    tabVariable tabVarAtt1;
    /* on teste si cette fonction existe */
    Variable * tmpFonc = (findFonc(tab1, node->valeur.text));
    /* cas où la fonction appelante n'existe pas*/
    if(tmpFonc == NULL){
      char msg[MAX_MSG_SIZE];
      strcpy(msg, " You are using the function ' ");
      strcat(msg, node->valeur.text);
      strcat(msg, " ' but it doesn't exist !! ");
      /* on écrit un message d'erreur */
      semError(msg, node->line);
      return 0;
    }
    
   

    initTabVar(&tabVarAtt1);
    initTabVar(&tabArgRecup);

    /* cas où on est dans une fonction elle-même utilisée comme argument d'une fonction */
    if(tabVar != NULL){
      /* on récupère les infos sur cette fonction*/

      Variable * copyVar = (findFonc(tab1, node->valeur.text));

      /* cas où la fonction appelée existe */
      if(copyVar != NULL){
        /* on ajoute cette "fonction" aux arguments (même si c'est pas une variable) */
        ajoutVariableTabVar(tabVar, *copyVar);
      }
      /* utilisation d'une fonction qui n'existe pas : code d'erreur*/
      else{
        return 0;
      }
    }
    
    /* on va récupérer les types des arguments */

    retourTmp = checkArgFonc(node,node->firstChild, name,tab1, &tabArgRecup);
    /* on met à jour la valeur de retour si  c'est pertinent*/
    if(retourTmp ==0){
      retour = retourTmp;
    }

    /* le frère de ma fonction appelée n'est pas un argument de cette même fonction */
    retourTmp = checkArgFonc(pere, node->nextSibling, name,tab1, tabVar);
    /* on met à jour la valeur de retour si  c'est pertinent*/
    if(retour != 0){
      retour = retourTmp;
    }
    

    /* on récupère le nombre et le type des arguments attendues par la fonction appelée*/

     retrieveParametreAttendu(node->valeur.text, tab1, &tabVarAtt1);


    /* il va falloir comparer les arguments attendues avec ceux que le user a entré */

     return compareArgsAttRecu(*node, tabVarAtt1, tabArgRecup);

  }

  /* rq : il n'est pas nécessaire ici de vérifier que les variables appelées
  existent car cela est vérifiée par une autre fonction.
  Donc leurs noms est nécessairement valide */
  else if(node->label == variable && tabVar != NULL){
      
      /* on récupère la variable que l'on cherchait */
      Variable * copyVar = (findVariable(tab1,node->valeur.text,findFoncPos(tab1,name)));
      /* cette fonction cherche parmi les variables globales la variable*/
      if(copyVar == NULL){
        copyVar = findGlobale(tab1, node->valeur.text);
      }
      if(copyVar != NULL){
        ajoutVariableTabVar(tabVar, *copyVar);
      }
      /* utilisation d'une variable qui n'existe pas :erreur*/
      else{
        return 0;
      }
      
      /* on relance récursivement sur les autres arguments pour update le type en cas d'agglomération*/
      retourTmp = checkArgFonc(node, node->firstChild, name,tab1, tabVar);
      /* on met à jour la valeur de retour si  c'est pertinent*/
      if(retour != 0){
          retour = retourTmp;
      }
      retourTmp =   checkArgFonc(pere, node->nextSibling, name,tab1, tabVar);
      if(retour != 0){
          retour = retourTmp;
      }
      return 1;
    
  }
      

  
  
  
  /* cas où on tombe sur une constante */
  else if((node->label == character && tabVar != NULL) || (node->label == num && tabVar != NULL)){
    
    
    Variable tmp;
    
    /* on ajoute la constante aux arguments trouvés*/
    createVariableFromConstNode(&tmp, *node);
  
    ajoutVariableTabVar(tabVar,tmp);

    
    
    /* on relance récursivement sur les autres arguments s'il y en a*/
    retourTmp = checkArgFonc(node, node->firstChild, name,tab1, tabVar);
    /* on met à jour la valeur de retour si  c'est pertinent*/
    if(retour != 0){
        retour = retourTmp;
    }
    retourTmp = checkArgFonc(pere, node->nextSibling, name,tab1, tabVar);
    /* on met à jour la valeur de retour si  c'est pertinent*/
    if(retour != 0){
        retour = retourTmp;
    }
    
    
    return retour;
    
    
    
  }

  /* on relance l'appel récursivement sur les fils dans les cas restants*/
  retourTmp = checkArgFonc(node, node->firstChild, name,tab1, tabVar);
  /* on met à jour la valeur de retour si  c'est pertinent*/
  if(retour != 0){
        retour = retourTmp;
  }
  retourTmp=checkArgFonc(pere, node->nextSibling, name,tab1, tabVar);
  /* on met à jour la valeur de retour si  c'est pertinent*/
  if(retour != 0){
        retour = retourTmp;
  }
  /* on renvoie le résultat de la valeur de retour*/
  
  
  return retour;
  
}

/*
fonction qui analyse les rvalue et les lvalue d'une fonction et précise si il y a un problème.
renvoie 1 : tout s'est bien passé.
renvoie 2 :cas warning.
renvoie 0 : cas d'erreur (affectation de void)
*/

int CompareRLValue(tabVariable lValue , tabVariable rValue){
  /*printf("lValue a : %d éléments et rvalue : %d éléments \n\n", lValue.nbVar, rValue.nbVar);*/
  int i,j;
  
  /* la valeur de retour par défaut tout s'est bien passé*/
  int retour;
  retour = 1;
  for(i = 0;i<lValue.nbVar;i++){
    for(j=0;j<rValue.nbVar;j++){
      /* cas où on a un char à gauche et un int en rvalue*/
      if(lValue.tab[i].type == MY_CHAR && rValue.tab[j].type == MY_INT){
          if(retour == 1){
            char msg[MAX_MSG_SIZE];
            strcpy(msg, " Lookout ' ");
            strcat(msg, lValue.tab[i].ident);
            strcat(msg, " ' expected ");
            strcat(msg, " a 'char' for the affectation but an int ' ");
            strcat(msg, rValue.tab[j].ident);
            strcat(msg,  " ' were given ! ");
            /* on écrit un message d'erreur */
            semWarning(msg, lValue.tab[i].line);
            retour = 2;

          }
      }
      /* cas où on donne un ensemble de char ce qui créé un int en rvalue même si il n'y a que des char*/
      else if(lValue.tab[i].type == MY_CHAR && rValue.nbVar > 1){
        if(retour == 1){
            char msg[MAX_MSG_SIZE];
            strcpy(msg, " Lookout ' ");
            strcat(msg, lValue.tab[i].ident);
            strcat(msg, " ' expected ");
            strcat(msg, " a 'char' for the affectation but a group of value that represents an int ");
            strcat(msg,  " has been given ! ");
            /* on écrit un message d'erreur */
            semWarning(msg, lValue.tab[i].line);
            retour = 2;

          }
      }
      /* cas où on affecte à une lValue le retour d'une fonction void*/
      else if(rValue.tab[j].type == MY_VOID){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, " You cannot affect to the left value ' ");
        strcat(msg, lValue.tab[i].ident);
        strcat(msg, " ' a  right value of type 'void' from ' ");
        strcat(msg, rValue.tab[j].ident);
        strcat(msg, " ' function !!");
        /* on écrit un message d'erreur */
        semError(msg, lValue.tab[i].line);

        retour = 0;

      }
    }
  }
  return retour;
}


/*
Fonction qui vérifie les affectations
elle renvoie :
2:si il y a un warning
1: si tout s'est bien passé
0 : cas d'erreur 
*/
int checkAffectation(Node * pere, char * currFonc, Node * node, TabSymb tab, tabVariable *lValue , tabVariable * rValue){
  int retour;
  int tmpRetour;
  retour = 1;
  tmpRetour = 1;
  /* cas d'arrêt*/
  if(node == NULL)
    return 1;
  /* cas où commence à traiter les arguments d'une fonction appelée : c'est un cas d'arrêt
  car ce n'est pas cette fonction qui gère ce cas*/
  if(node->label == argument)
    return 1;
  /* cas où on entre dans une nouvelle fonction*/

  if(node->label == id){
    retour = checkAffectation(node, node->valeur.text, node->firstChild , tab, NULL, NULL);
    tmpRetour = checkAffectation(pere, node->valeur.text, node->nextSibling , tab, NULL, NULL);
    /* on met à jour la valeur de retour si nécessaire car l'erreur peut être plus importante que la précédente */
    if(retour != 0 && tmpRetour != 1){
      retour = tmpRetour;
    }

    return retour;
  }

  /* on tombe sur une affectation on va traiter ses fils en tant que valeur gauche et droite */
  if(node->label == affectation){
    /* on initialise les variables qui contiendront les left value et les rights values*/
    tabVariable lValue1;
    tabVariable rValue1;
    initTabVar(&lValue1);
    initTabVar(&rValue1);

    /* on traite tous les éléments liés à l'affectation (les Lvalues et Rvalues)*/
    tmpRetour = checkAffectation(node, currFonc, node->firstChild , tab, &lValue1, &rValue1);
    if(retour != 0 && tmpRetour != 1){
      retour = tmpRetour;
    }
    /* on traite tous les éléments liés à l'affectation (les Lvalues et Rvalues)*/
    tmpRetour = checkAffectation(pere, currFonc, node->nextSibling , tab, NULL, NULL);
    if(retour != 0 && tmpRetour != 1){
      retour = tmpRetour;
    }

    /* après avoir traité récursivement les fils liés à l'affectation en ayant ajouté leurs infos, on compare les 
    lValue avec les rValue */
    /* printf(" le nombre de lvalue vaut : %d \n\n", lValue1.nbVar);*/
    tmpRetour = CompareRLValue(lValue1, rValue1);
    /*on met à jour la valeur de retour*/
    if(retour != 0 && tmpRetour != 1){
      retour = tmpRetour;
    }
    return retour;

  }
  
  /*on tombe sur une valeur gauche on l'ajoute en tant que tel*/
  else if(pere != NULL && pere->label == affectation && node->label == variable){
    Variable * var1 = NULL;
    
    /*printf("on tombe sur une lvalue \n");*/
    var1 = (findVariable(tab,node->valeur.text,findFoncPos(tab,currFonc)));
    
    /* cette fonction cherche parmi les variables globales la variable*/
      if(var1 == NULL){
        var1 = findGlobale(tab, node->valeur.text);
      }
      if(var1 != NULL){
        updateLineVariable(node->line,var1);
        /*printf("on tombe sur une lvalue non null \n");*/
        ajoutVariableTabVar(lValue, *var1);
      }
      /* utilisation d'une variable qui n'existe pas :erreur*/
      else{
        return 0;
      }
  }

  /* cas où on tombe sur un appel de fonciton en rValue*/
  else if(node->label == appelFonction && rValue != NULL){
    /*printf("on tombe sur une rvalue \n");*/
    Variable * f1 = findFonc(tab, node->valeur.text);
    Variable f2;
    /* cas d'erreur cette fonction n'existe pas */
    if(f1 == NULL){
      return 0;
    }
    else{
      /* il faut revenir ici*/
      deepCopyVar(&f2, *f1);
      updateLineVariable(node->line,&f2);
      
      ajoutVariableTabVar(rValue, f2);
      /* on traitera seulement ses frères, pas ses fils car ses fils correspondront à ses arguments.
      Les arguments seront traités ailleurs par une autre fonction */

      /*on met à jour la valeur de retour*/
      tmpRetour = checkAffectation(pere, currFonc, node->nextSibling, tab, lValue , rValue);
      /*on met à jour la valeur de retour*/
      if(retour != 0 && tmpRetour != 1){
        retour = tmpRetour;
      }
    }

    return retour;
  }
  
  
  
  /* cas où on tombe sur des right-values */

  /* rq : il n'est pas nécessaire ici de vérifier que les variables appelées
  existent car cela est vérifiée par une autre fonction.
  Donc leurs noms est nécessairement valide */
  
  
  else if(node->label == variable && rValue != NULL){
    /*printf("on tombe sur une rvalue \n");*/
      /* on récupère la variable que l'on cherchait */
      Variable * copyVar = (findVariable(tab,node->valeur.text,findFoncPos(tab,currFonc)));
      /* cette fonction cherche parmi les variables globales la variable*/
      if(copyVar == NULL){
        copyVar = findGlobale(tab, node->valeur.text);
      }
      if(copyVar != NULL){
        ajoutVariableTabVar(rValue, *copyVar);
        updateLineVariable(node->line, &(rValue->tab[rValue->nbVar]));
      }
      /* utilisation d'une variable qui n'existe pas :erreur*/
      else{
        return 0;
      }
      return 1;

  }
  
  
  /* cas où on tombe sur une constante en r-Value */
  else if((node->label == character && rValue != NULL) || (node->label == num && rValue != NULL)){
    /*printf("on tombe sur une rvalue \n");*/
    
    Variable tmp;

    /* on ajoute la constante aux arguments trouvés*/

    createVariableFromConstNode(&tmp, *node);
    
    ajoutVariableTabVar(rValue,tmp);
    updateLineVariable(node->line, &(rValue->tab[rValue->nbVar]));
  }
  
  


  /* On relance l'appel récursif */


  tmpRetour = checkAffectation(pere, currFonc, node->nextSibling, tab, lValue, rValue);
  /*on met à jour la valeur de retour*/
  if(retour != 0 && tmpRetour != 1){
      retour = tmpRetour;
  }

  tmpRetour = checkAffectation(node, currFonc, node->firstChild, tab, lValue, rValue);
  /*on met à jour la valeur de retour*/
  if(retour != 0 && tmpRetour != 1){
      retour = tmpRetour;
  }
  
  return retour;
}

/* 
cette fonction met à jour la ligne
où a été trouvée une variable */
void updateLineVariable(int line, Variable * var1){
  var1->line = line;
}

/* this function check if a main is present
it returns 0 if not
it returns 1 if yes
 */
int checkIfMainIsHere(TabSymb tab_symb){
  Variable * var;
  if((var = findFonc(tab_symb , "main"))== NULL){
    semError2(" There's no main in your C FILE :( \n\n");
    return 0;
  }
  if(var->type != MY_INT){
    semError2(" Your main need 'int' for return type \n\n");
    return 0;
  }
  return 1;
}

/*
fait une copie profonde d'une variable vers une autre 
*/
void deepCopyVar(Variable *dest, Variable source){
    if(dest == NULL){
      return;
    }
    dest->line = source.line;
    strcpy(dest->ident,source.ident);
    dest->type = source.type;
    dest->adresse = source.adresse;
    dest->suivant = NULL;
    dest->type_rec = source.type_rec;
}

/*
node correspond au noeud traité, si c'est une valeur de retour on l'ajoute à returnValues
la fonciton renvoie 0 en cas de problème
renvoie 1 si tout s'est bien passé 
*/
int retrieveReturnValues(TabSymb tab, Node * node, char * currFonc, tabVariable * returnValues){
  if(returnValues == NULL)
    return 1;
  if(node == NULL)
    return 1;
  /* cas où on tombe sur un appel de fonction parmi les retour */
  if(node->label == appelFonction){
    Variable * tmp = findFonc(tab, node->valeur.text);
    if(tmp != NULL){
      ajoutVariableTabVar(returnValues, *tmp);
     /* printf("ajout d'un appel de fonction en valeur de retour \n\n");*/
    }
    else{
      return 0;
    }
    
  }
  /* cas où on tombe sur une variable parmi les valeurs de retour */
  if(node->label == variable){
    int fonc1pos;
    Variable * var1;
    fonc1pos = findFoncPos(tab,currFonc);
    /*printf("ajout d'une variable en valeur de retour \n\n");*/
    var1 = findVariable(tab, node->valeur.text, fonc1pos );
    if(var1 == NULL){
      var1 = findGlobale(tab, node->valeur.text);
    }
    if(var1 != NULL){
      ajoutVariableTabVar(returnValues, *var1);
      
    }
    else{
      return 0;
    }
  }

  /* cas où on tombe sur une constante */

  else if(node->label == character  || node->label == num){
    Variable tmp;
    /* on ajoute la constante aux arguments trouvés*/
    createVariableFromConstNode(&tmp, *node);
    ajoutVariableTabVar(returnValues,tmp);
    /*printf("ajout d'une constante en valeur de retour \n\n");*/
  }

  return 1;
}

/*
renvoie le type de retour associé à un ensemble de valeur donné en retour
*/
int fromTabToReturnVal(tabVariable returnValues){
  
  /* par défaut on renvoie du void*/
  int retourType;

  retourType = MY_VOID;
  
  if(returnValues.nbVoid > 0 ){
    /* on fait des opérations avec un void et d'autres types : cela créé une erreur */
    if(returnValues.nbChar > 0 || returnValues.nbInt >0 || returnValues.nbVoid >1  ){
      return MY_NONE;
    }
  }

  if(returnValues.nbChar >= 1){
    if(returnValues.nbInt > 0 || returnValues.nbChar >= 2){
      return MY_INT;
    }
    else{
      return MY_CHAR;
    }
  }

  if(returnValues.nbInt >= 1){
    return MY_INT;
  }
  

  /* cas par défaut : void*/
  return retourType;
}
/*
la currfonc correspond à la fonction dans laquelle on est actuellement
node correspond au noeud auquel on est
pere correspond au pere du noeud
tab_symb est la tab des symboles à remplir avec les infos
tab1 correspond à l'ensemble des valeurs données comme valeurs de retour
*/
void addReturnValue(TabSymb * tab_symb, char * currFonc, Node * node, Node * pere, tabVariable  *tab1){
  if(node == NULL)
    return;
/*on tombe sur une fonction on l'ajoute comme currFonc*/
  if(node->label == id){
    /*printf("on traite la fonction %s \n\n", node->valeur.text);*/
    addReturnValue(tab_symb, node->valeur.text, node->firstChild, node, tab1);
    addReturnValue(tab_symb, node->valeur.text, node->nextSibling, pere, tab1);
    return;
  }

/* on tombe sur des valeurs de retour on les ajoutes*/
  if(pere != NULL && pere->label == Return){
    
    /* on initiliase un tableau qui contiendra les valeurs de retour */
    Variable * fonc;
    tabVariable returnValues;
    
    
    
    initTabVar(&returnValues);

    /* on récupère la fonction pour laquelle on va ajouter un type de retour*/
    fonc = findFonc(*tab_symb,currFonc);
    
    /* on récupère les autres valeurs de retour */
    if(node->label != appelFonction){

      addReturnValue(tab_symb, currFonc, node->firstChild, node, &returnValues);

    }
    addReturnValue(tab_symb, currFonc, node->nextSibling,pere, &returnValues);

    /* on récupère les infos sur le noeud qui est aussi une partie de la valeur de retour */

    retrieveReturnValues(*tab_symb, node, currFonc, &returnValues);
    
    if(fonc != NULL){
      /*printf("on traite la fonction %s \n\n", fonc->ident);
       printf("elle a  %d int en retour \n\n", returnValues.nbInt);
        printf("elle a  %d char en retour \n\n", returnValues.nbChar);*/
      /* on donne le type de retour reçu à la fonction */
      fonc->type_rec = fromTabToReturnVal(returnValues);
    }
    
    /* on arrête le traitement*/
    return;
  }
  

  /* on récupère les valeurs restantes si ce sont des valeurs de retour */

  retrieveReturnValues(*tab_symb, node, currFonc, tab1);
  
  /* on ne regarde pas les fils des fonctions car ce sont ses arguments*/
  if(node->label != appelFonction){
    addReturnValue(tab_symb, currFonc, node->firstChild, node, tab1);
  }
  addReturnValue(tab_symb, currFonc, node->nextSibling, pere,  tab1);
  
  
}

/*
Cette fonction compare le type reçu par les fonctions avec celui attendu par celle-ci.
Si il sont incompatibles elle renvoie 0.
Si il sont compatibles elle renvoie 1.
Si cela soulève un warning elle renvoie 2.

*/
int compareTypeRecToExp(TabSymb tabSymb){
  int i;
  int retour;
  /* par défaut tout s'est bien passé */
  retour = 1;
  for(i = 0;i<tabSymb.nb_fonc;i++){
    /* reçu et exp correspondent au type attendu et au type reçu*/
    int recu;
    int exp;
    Variable tmp = tabSymb.tab_fonc[i][FONCTION];
    exp = tabSymb.tab_fonc[i][FONCTION].type;
    recu = tabSymb.tab_fonc[i][FONCTION].type_rec;
    if(exp != recu){
      
      /* les cas problématiques */

      /* un char qui reçoit un entier : Warning */
      if(exp == MY_CHAR && recu == MY_INT){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, " Lookout ' ");
        strcat(msg, tmp.ident);
        strcat(msg, " ' expected ");
        strcat(msg, " a 'char' for return value but you gave an int ");
        /* on écrit un message d'avertissement */
        semWarning(msg, tmp.line);
        /* on met à jour le retour pour indiquer un warning si on a pas déjà une erreur*/
        if(retour !=0 )
          retour = 2;
      }

      /* mélange de types incompatibles*/
      else if(recu == MY_NONE){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, "  ' ");
        strcat(msg, tmp.ident);
        strcat(msg, " 'has received a return a value with inconsistent values ");
        /* on écrit un message d'avertissement */
        semError(msg, tmp.line);
        retour = 0;
      }

      /* un void avec n'importe quoi qui n'est pas void == Erreur*/
      else if(exp == MY_VOID){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, "  ' ");
        strcat(msg, tmp.ident);
        strcat(msg, " ' didn't expected any return value ");
        semError(msg, tmp.line);
        retour = 0;
      }

      /* un void avec n'importe quoi qui n'est pas void == Erreur*/
      else if(exp == MY_INT && recu == MY_VOID){
        char msg[MAX_MSG_SIZE];
        strcpy(msg, "  ' ");
        strcat(msg, tmp.ident);
        strcat(msg, " ' expected an int but didn't received any return value ");
        semError(msg, tmp.line);
        retour = 0;
      }

    }
  }

  return retour;
}

/*
cette fonction rassemble le type de la dernière variable de tabvar
avec le type de la nouvelle variable associée en tant que paramètre
pour créer un nouveau type.
Exemple :

int + int = MY_INT
int + char = MY_INT
_ +void = MY_NONE
*/
void gatherTypeExpr(Variable var1, tabVariable * tabVar){
  /* on met à jour le type de la dernière variable*/
  if(var1.type == MY_VOID){
    tabVar->tab[tabVar->nbVar-1].type = MY_NONE ;
  }

  else {
    tabVar->tab[tabVar->nbVar-1].type = MY_INT;
  }
}



/* 
Cette fonction indique si cette variable a déjà été ajouté pour cette fonction
renvoie un pointeur sur la variable si la variable est déjà dedans
et renvoie NULL sinon
nomFonc = fonction de la fonciton de référence : fonction appelante */
Variable * findVariable2(TabSymb tab1 , char * nomVar, char * nomFonc){

  /* récupère la position de la fonction dans la tab des symboles*/
  int pos;
  Variable * var1;
  pos = findFoncPos(tab1, nomFonc);
  if(pos != -1){
    var1 = findVariable(tab1,nomVar,pos);
    /* si on a pas trouvé la variable parmi les locales, on cherche dans les globales*/
    if(var1 == NULL){
      var1 = findGlobale(tab1 , nomVar);
    }
    return var1;
  }
  /* cas d'erreur */
  return NULL;

}

/* fonction qui indique si une variable est locale
1 == oui
0 == non
*/
int isLocalVariable(TabSymb tab1 , char * nomVar, char * nomFonc){
  /* récupère la position de la fonction dans la tab des symboles*/
  int pos;
  Variable * var1;
  pos = findFoncPos(tab1, nomFonc);
  if(pos != -1){
    var1 = findVariable(tab1,nomVar,pos);
    if(var1 != NULL){
      return 1;
    }
  }
  /* la variable n'est pas locale ou la fonction n'existe pas*/
  return 0;

}

/* fonction qui indique si une variable est locale
1 == oui
0 == non
*/
int isGlobalVariable(TabSymb tab1 , char * nomVar){
  Variable * var1;
  var1 = findGlobale(tab1,nomVar);
  /* la variable a été trouvée parmi les globales*/
  if(var1 != NULL){
      return 1;
  }
  /* la variable n'est pas locale ou la fonction n'existe pas*/
  return 0;

}

/* donne l'@ d'une variable locale ou globale
en cherchant dans les variables locales puis globales
currFonc correspond à la fonction dans laquelle on est actuellement
tab1 est la tab des symboles
retourne -1 si la variable est introuvable
 */
int findAdresseVariable(char * nameVariable, char * currFonc , TabSymb tab1){
  Variable * var1 = findVariable2(tab1, nameVariable, currFonc);
  if(var1 != NULL)
    return var1->adresse;
  /* cas d'erreur */
  return -1;
}

/* fonction auxiliaire de la fonction checkComparaison*/
int checkComparaisonAux(Node * node, Node * pere, TabSymb tab1){
  /* une variable pour récupérer une fonction */
  Variable * var1;
  /* cas d'arrêt*/
  if(node == NULL)
    return 1;
  /* cas où on tombe sur un élément de la comparaison : on test la validité */
  /* */
  if(node->label != operateur && pere != NULL && (pere->label == operateur || pere->label == comparateur || (pere->label == conditionnel && strcmp(pere->valeur.text, "if") == 0))){

    /* on récupère les infos de la fonction*/
    if(node->label ==  appelFonction){

      var1 = findFonc(tab1,node->valeur.text);


      /* si la fonction renvoie du void : erreur */
  
        if(var1 != NULL && var1->type == MY_VOID){
          
          char msg[MAX_MSG_SIZE];
          
          strcpy(msg, "You're trying to use the fonction ' ");
          strcat(msg, node->valeur.text);
          strcat(msg, " ' ");
          strcat(msg, "for a comparison, but it's not possible because it returns 'void' !! ");
          
          semError(msg, node->line);
          

        
          return 0;
          

        }
    }
    if(pere->label != conditionnel){
      /*on regarde sur les frères de la fonction ou de l'élément de façon plus générale*/
      return checkComparaisonAux(node->nextSibling, pere, tab1);
    }

    return 1;
  }

  /* cas particulier avec n opérateur (x+y+....+z), on va aussi chercher dans le frère*/
    if(node->label == operateur && (pere == NULL || pere->label != conditionnel)){
      if(checkComparaisonAux(node->nextSibling, pere, tab1) == 0){
        return 0;
      }
    }

  /* on relance la recherche récursivement sur les fils à partir de l'opérateur de comparaison*/

  return checkComparaisonAux(node->firstChild,node, tab1);

}
/*
fonction qui vérifie que les comparaisons sont toutes valides.
Cela revient à vérifier s'il n'y a pas d'élément de type "void fonction"
(fonction qui renvoie void) parmi les éléments de la comparaison
renvoie 0 en cas d'erreur sémantique
renvoie 1 si tout s'est bien passé
*/
int checkComparaison(Node * node, TabSymb tab1){
  int retour;
  /* cas d'arrêt */
  if(node == NULL)
    return 1;
  /* on lance la vraie fonction qui gère les comparaisons*/

  if(node->label == comparateur || conditionnel){
    retour = checkComparaisonAux(node, NULL,tab1);
    if(retour == 0){
      return 0;
    }

  }

  /* on relance la fonction récursivement */
  if(checkComparaison(node->firstChild, tab1) == 0)
    return 0;

  if(checkComparaison(node->nextSibling, tab1) == 0)
    return 0;

  return 1;

  
}

/* 
Cette fonction indique si cette variable est un argument de la fonction choisie. 
Elle renvoie 1 si c'est un argument et 0 si ce n'est pas un argument */
int thisVarIsArg(TabSymb tab_symb , char * nom_var, int pos){

  Variable *tmp;

  /* enregistre le hashcode*/
  /*à supprimer int code;
    
  code = hashCode(nom_var);*/


  /* on commence par regarder les arguments de la fonction, c'est à dire, dans la liste chainée des variables qui sont à la première case associée au numéro de la fonction.
  Par définitioon la fonction "globale" (le fichier) n'a pas d'arguments donc il n'est pas nécessaire d'aller ici */
  if(pos != GLOBALE){
    for(tmp =  &tab_symb.tab_fonc[pos][ARGS]; tmp != NULL;  tmp=tmp->suivant){
      /* si une variable est présente à cet emplacement, on tente de comparer son nom avec celui recherché*/
      if(strcmp(tmp->ident, "\0") != 0 && tmp->type != MY_NONE){
        /*printf("l'emplacement en arg de la fonction a été trouvé \n");*/
        if(strcmp(tmp->ident, nom_var) == 0){
            /* la variable a été trouvée parmi les argument*/
            return 1;
        }
      }
    }
  }

  /* la variable n'est pas un argument */
  
  return 0;
}
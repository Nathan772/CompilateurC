/* fichier qui gère la génération de l'assembleur à partir de l'arbre*/

#include "generationAssembleur.h"

/*


Il manque :  
    1) pb du traitement 2x du même argument :
      -le pb est lié à la fonction "handle appel" donc
      il faut plutôt regarder de ce côté. 
      (la fonction compute2 évite normalement la possibilité de premiers problèmes)
    -La gestion des conditionnels (if-else).
    -La gestion des comparaisons.
    -La gestion des appels de fonctions en right-value (avec et sans arguments + avec arguments composés) :
    -Dès qu'il y a un appel de fonction il faut empiler les précédentes valeurs des arguments
    avant d'effectuer cet appel et les remettre dans les registres après (avec pop)
    -La gestion des appels de fonction seul (avec et sans arguments + avec arguments composé)

*/

/* fonction qui gère les calculs
lorsqu'on tombe sur une opération
dans l'arbre 
node correspond au noeud sur lequel on est 
currFonc correspond à la fonction dans laquelle on est actuellement*/
void compute(Node *node, TabSymb tab1, char * currFonc, FILE *fichier){
  Node *tmp;
  int i;
  /* si on tombe sur un noeud null, on arrête le parcours */

  if(node == NULL){
      return;
  }
   
  /*printf(" un noeud a le label operateur %d \n\n" ,node->label);
  printf("on entre dans compute \n \n");*/
    /*on tombe sur une soustraction : on réalise l'opération de soustraction
    après avoir ajouté les éléments dans la pile */

  if(node->label == operateur){
      /* on fait l'appel récursif sur le sibling pour récupérer l'opérande gauche s'il y en a*/
      compute(node->nextSibling, tab1 , currFonc, fichier);
      /* on fait l'appel récursif sur les enfants pour ajouter les valeurs des opérandes sur la pile*/
      compute(node->firstChild, tab1 , currFonc, fichier); 
      /*compute(node->nextSibling, tab1, currFonc, fichier); à supprimer ??*/

       fprintf(fichier, " ; calculs (addition, soustraction, division ou multiplication) \n");
        /*on tombe sur une soustraction*/

        if(node->valeur.car == '-'){
          fprintf(fichier, "pop rcx ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "sub rax,rcx \n");
          fprintf(fichier, "push rax ; on empile le résultat du calcul \n");
        }

        /*on tombe sur une addition*/
        else if(node->valeur.car == '+'){
          fprintf(fichier, "pop rcx ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "add rax,rcx \n");
          fprintf(fichier, "push rax ; on empile le résultat du calcul  \n");
        }

        /*on tombe sur une multiplication*/

        else if(node->valeur.car == '*'){
          fprintf(fichier, "pop rcx ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "imul rax,rcx \n");
          fprintf(fichier, "  push rax ; on empile le résultat du calcul \n");
        }

        /*on tombe sur une division */
        
        else if(node->valeur.car == '/'){

          fprintf(fichier, "mov rdx, 0 ; on met rdx à 0 pour que ses valeurs résiduelles ne posent pas problèmes \n");
          fprintf(fichier, "pop r8 ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "idiv r8 ; on fait la division \n");
          fprintf(fichier, "push rax ; on empile le résultat du calcul \n");

        }

      return;
      
  }

  else if(node->label == appelFonction){

    /* on calcul et empile les arguments de la fonction*/

    if(node->firstChild != NULL){
      
      /* on va regarder les arguments */
      tmp = node->firstChild->firstChild;
      
      while(tmp != NULL){

        compute2(node, tmp, tab1, currFonc, fichier);
        
        /* on parcourt l'argument suivant */

        tmp = tmp->nextSibling;
      }
    }

    /* cas du bourrage*/

    if(compteTotalArg(tab1, node->valeur.text) % 2 != 0){
      /* on bourre */
      fprintf(fichier, "push 0 ; on bourre car on a un nombre impair d'argument \n");
    }

    fprintf(fichier, "call %s ; on appel la fonction après qu'elle ait reçu ses arguments \n", node->valeur.text);
    
    

    for(i = 0; i<compteTotalArg(tab1, node->valeur.text);i++){
      
      /* cas où on doit dépiler une fois de plus car il y avait eu du bourrage*/
      if(compteTotalArg(tab1, node->valeur.text) % 2 != 0){
        if(i == 0){
          fprintf(fichier, "pop r8 ; on dépile les arguments qui avaient été empilés pour l'appel \n");
        }
      }
      /* on dépile les arguments qui étaient empilés pour l'appel de fonction*/

      fprintf(fichier, "pop r8 ; on dépile les arguments qui avaient été empilés pour l'appel \n");
    }
    /* on empile le résultat de l'appel de fonction pour pouvoir s'en servir lors des calculs */
    fprintf(fichier, "push rax ; on empile le résultat de l'appel de fonction pour pouvoir s'en servir lors des calculs \n");
    
    return;
  }

  /* on tombe sur un nombre on l'empile */
  else if(node->label == num){

      fprintf(fichier, "push %d ; on tombe sur un nombre en calcul intermédiaire, on l empile \n", node->valeur.entier);

  }

  /*on tombe sur un caractère, on l'empile */
  else if(node->label == character){
    fprintf(fichier, "push '%c' ; on tombe sur une lettre en calcul intermédiaire, on l empile \n", node->valeur.car);
  }

  /* on tombe sur une variable, on l'empile */

  else if(node->label == variable){

      /* variable associée à la variable que l'on cherche dans la tab des symboles a stocker */
      
      Variable * var1;

      /* variable qui stocke la position de la fonction dans laquelle on est par rapport à la tab des symboles*/
      int pos;

      /* indique que la variable est un argument */
      int isArg;

      /* on trouve la position de la fonction dans la tab des symboles*/
      pos = findFoncPos(tab1,currFonc);

      /* on récupère la variable pour avoir son adresse*/
      var1 = findVariable(tab1, node->valeur.text ,pos);

      /*on récupère l'info si oui ou non la variable est un argument*/
      isArg = thisVarIsArg(tab1 ,  node->valeur.text, pos);



      /* cas où c'est une variable globale que l'on cherche*/
      if(var1 == NULL){

        var1 = findGlobale(tab1, node->valeur.text);

        /* cas d'erreur qui n'est pas censé arriver*/

        if(var1 == NULL){
          printf("Erreur variable %s , ligne %d introuvable \n\n", node->valeur.text, node->line);
          return;
        }

        if(var1->type == MY_INT){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov eax, [global_Int+%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire \n");
        }

        else if(var1->type == MY_CHAR){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov al, [global_Char+%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire \n");
        }

      }
      /* cas où on est sur une variable locale*/
      else if(!isArg){

        if(var1->type == MY_INT){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov rax, [rbp-%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire associée à la variable locale \n");
        }

        else if(var1->type == MY_CHAR){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov rax, [rbp-%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire associée à la variable locale \n");
        }

        
      }

      /* cas où on est sur un argument*/

      else if(isArg){

        /* on compte le nombre d'arguments */
        int nbArg;

        /* on compte le nombre d'arguments de la fonction */

        nbArg = compteTotalArg(tab1, currFonc);

         if(var1->type == MY_INT){
          
          /* cas où il n'y a pas de bourrage */

          if(nbArg % 2 == 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument \n", var1->adresse);

          }

          else if(nbArg % 2 != 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument \n", var1->adresse+8);

          }

          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire associée à l'argument\n");
        }

        else if(var1->type == MY_CHAR){

          
          /* cas où il n'y a pas de bourrage */

          if(nbArg % 2 == 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument \n", var1->adresse);

          }

          /* Il y a du bourrage car on a un nombre impair d'arguments donc il faut faire l'@ de l'élément +8 */

          if(nbArg % 2 != 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument\n", var1->adresse+8);

          }

          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax \n");

        }
      }

  }

  /*on continue le parcours pour le reste*/

  compute(node->firstChild, tab1 , currFonc, fichier); 
  compute(node->nextSibling, tab1, currFonc, fichier);
  
}

/* fonction qui gère les calculs
lorsqu'on tombe sur une opération
dans l'arbre mais dans le cas spécifique des appels de fonctions
node correspond au noeud sur lequel on est 
pere correspond au pere de node
currFonc correspond à la fonction dans laquelle on est actuellement*/
void compute2(Node * pere, Node *node, TabSymb tab1, char * currFonc, FILE *fichier){

  /* si on tombe sur un noeud null, on arrête le parcours */

  if(node == NULL){
      return;
  }
   
  /*on tombe sur une soustraction : on réalise l'opération de soustraction
    après avoir ajouté les éléments dans la pile */

  if(node->label == operateur){
      /* on évite de traiter les prochains arguments comme si ils étaient associés au 
      même argument*/
      if(pere->label != argument && pere->label != arguments){
        /* on fait l'appel récursif sur le sibling pour récupérer l'opérande gauche s'il y en a*/
        compute2(pere, node->nextSibling, tab1 , currFonc, fichier);
      }
      /* on fait l'appel récursif sur les enfants pour ajouter les valeurs des opérandes sur la pile*/
      compute2(node, node->firstChild, tab1 , currFonc, fichier); 
      /*compute(node->nextSibling, tab1, currFonc, fichier); à supprimer ??*/

       fprintf(fichier, " ; calculs (addition, soustraction, division ou multiplication) \n");
        /*on tombe sur une soustraction*/

        if(node->valeur.car == '-'){
          fprintf(fichier, "pop rcx ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "sub rax,rcx \n");
          fprintf(fichier, "push rax ; on empile le résultat du calcul \n");
        }

        /*on tombe sur une addition*/
        else if(node->valeur.car == '+'){
          fprintf(fichier, "pop rcx ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "add rax,rcx \n");
          fprintf(fichier, "push rax ; on empile le résultat du calcul  \n");
        }

        /*on tombe sur une multiplication*/

        else if(node->valeur.car == '*'){
          fprintf(fichier, "pop rcx ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "imul rax,rcx \n");
          fprintf(fichier, "  push rax ; on empile le résultat du calcul \n");
        }

        /*on tombe sur une division */
        
        else if(node->valeur.car == '/'){

          fprintf(fichier, "mov rdx, 0 ; on met rdx à 0 pour que ses valeurs résiduelles ne posent pas problèmes \n");
          fprintf(fichier, "pop r8 ; on récupère les valeurs qui ont été empilées lors de la traversée des opérandes gauches et droites \n");
          fprintf(fichier, "pop rax ; même chose \n");
          fprintf(fichier, "idiv r8 ; on fait la division \n");
          fprintf(fichier, "push rax ; on empile le résultat du calcul \n");

        }

      return;
      
  }

  /* on tombe sur un nombre on l'empile */
  if(node->label == num){

      fprintf(fichier, "push %d ; on tombe sur un nombre en calcul intermédiaire, on l empile \n", node->valeur.entier);

  }

  /*on tombe sur un caractère, on l'empile */
  else if(node->label == character){
    fprintf(fichier, "push '%c' ; on tombe sur une lettre en calcul intermédiaire (ou ), on l empile \n", node->valeur.car);
  }

  /* on tombe sur une variable, on l'empile */

  else if(node->label == variable){

      /* variable associée à la variable que l'on cherche dans la tab des symboles a stocker */
      
      Variable * var1;

      /* variable qui stocke la position de la fonction dans laquelle on est par rapport à la tab des symboles*/
      int pos;

      /* indique que la variable est un argument */
      int isArg;

      /* on trouve la position de la fonction dans la tab des symboles*/
      pos = findFoncPos(tab1,currFonc);

      /* on récupère la variable pour avoir son adresse*/
      var1 = findVariable(tab1, node->valeur.text ,pos);

      /*on récupère l'info si oui ou non la variable est un argument*/
      isArg = thisVarIsArg(tab1 ,  node->valeur.text, pos);



      /* cas où c'est une variable globale que l'on cherche*/
      if(var1 == NULL){

        var1 = findGlobale(tab1, node->valeur.text);

        /* cas d'erreur qui n'est pas censé arriver*/

        if(var1 == NULL){
          printf("Erreur variable %s , ligne %d introuvable \n\n", node->valeur.text, node->line);
          return;
        }

        if(var1->type == MY_INT){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov eax, [global_Int+%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire \n");
        }

        else if(var1->type == MY_CHAR){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov al, [global_Char+%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire \n");
        }

      }
      /* cas où on est sur une variable locale*/
      else if(!isArg){

        if(var1->type == MY_INT){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov rax, [rbp-%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire associée à la variable locale \n");
        }

        else if(var1->type == MY_CHAR){
          /* on cherche à l'endroit associé à l'adresse de l'élément*/
          fprintf(fichier, "mov rax, [rbp-%d] \n", var1->adresse);
          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire associée à la variable locale \n");
        }

        
      }

      /* cas où on est sur un argument*/

      else if(isArg){

        /* on compte le nombre d'arguments */
        int nbArg;

        /* on compte le nombre d'arguments de la fonction */

        nbArg = compteTotalArg(tab1, currFonc);

         if(var1->type == MY_INT){
          
          /* cas où il n'y a pas de bourrage */

          if(nbArg % 2 == 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument \n", var1->adresse);

          }

          else if(nbArg % 2 != 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument \n", var1->adresse+8);

          }

          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax ; on empile la valeur intermédiaire associée à l'argument\n");
        }

        else if(var1->type == MY_CHAR){

          
          /* cas où il n'y a pas de bourrage */

          if(nbArg % 2 == 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument \n", var1->adresse);

          }

          /* Il y a du bourrage car on a un nombre impair d'arguments donc il faut faire l'@ de l'élément +8 */

          if(nbArg % 2 != 0){

            /* on cherche à l'endroit associé à l'adresse de l'élément*/

            fprintf(fichier, "mov rax, [rbp+%d] ; on empile la valeur intermédiaire associée à l'argument\n", var1->adresse+8);

          }

          /*on ajoute cet élément sur la pile*/
          fprintf(fichier, "push rax \n");

        }
      }

  }

  /* on continue le parcours pour le reste ssi c'est une autre opérande */

  if(pere->label == operateur){
    compute2(pere, node->nextSibling, tab1, currFonc, fichier);
  }
  
}



/*
void newFile(FILE *fichier){
    fprintf(fichier, "global \n section .text\n");
}*/

/* écrit l'entête de base pour un fichier nasm*/
void enteteBasique(FILE * fichier){
  /* il faudra ajouter putint à cette section*/
  fprintf(fichier, "\nsection .text\nglobal _start\nextern getchar\nextern getint \nextern putchar \nextern putint \nextern show_registers\n");
  
}

void beginMainFile(FILE * fichier){
  fprintf(fichier, "\n _start: \n");
}

void endFile(FILE * fichier){
    fprintf(fichier, "mov rax, 60 ; code nécessaire pour indiquer la fin du programme \nmov rdi, 0 ; valeur de retour par défaut : 0 \nsyscall\n");
}

/* fonction de fin de main 
valRet correspond à la valeur de retour qu'on a choisi 
*/

void endFile2(FILE * fichier, int valRet){
    fprintf(fichier, "mov rax, 60 ; code nécessaire pour indiquer la fin du programme \nmov rdi, %d ; valeur de retour par défaut : 0 \nsyscall\n", valRet);
}

/* fonction qui génère le fichier assembleur à partir de
la racine d'un arbre*/
void generateAssembly(FILE * fichier, TabSymb tab,Node * node1){
  ajoutGlobal(fichier, tab);
  return;
}

/* fonction qui prend les variables globales de la tab des symboles et les ajoute au fichier assembleur*/
void ajoutGlobal(FILE * fichier, TabSymb tab) {
  int i;
  Variable * next;
  /* compte le nombre d'entier global*/
  int nbInt = 0;
  int nbChar = 0;
  fprintf(fichier, "section .data \n");
  /* choix provisoire d'implémentation : 
  une variable globale = une étiquette*/
  /* on ajoute les variables globales entières */
  /*fprintf(fichier, "globalInt : dd ");*/

  for(i=0;i<NB_VAR;i++){


    /* POUR chaque variable global de type INT on indique avec un 0 un nouvel emplacement associé*/
    if(tab.tab_fonc[GLOBALE][i].type == MY_INT){
      if(nbInt == 0){

        fprintf(fichier, "global_Int : dd 0");
        nbInt++;

      }
      else{
        fprintf(fichier, ", 0");
      }
      
    }

    next = tab.tab_fonc[GLOBALE][i].suivant;

      while(next != NULL){
        
        
        if(next->type == MY_INT){

          if(nbInt == 0){

            fprintf(fichier, "global_Int : dd 0");
            nbInt++;

          }
          else{
            fprintf(fichier, ", 0");
          }
        }

        next = next->suivant;

      }
  }

  fprintf(fichier, "\n");

  /* on ajoute les variables globales caractères */
  /*fprintf(fichier, "globalChar : db ");*/
  /*fprintf(fichier, "globalInt : dd ");*/
  /*fprintf(fichier, " ");*/

  for(i=0;i<NB_VAR;i++){

      if(tab.tab_fonc[GLOBALE][i].type == MY_CHAR){
        /*on tombe sur le premier char on créé une étiquette associé aux chars global*/
        if(nbChar == 0){
          fprintf(fichier, "global_Char : db 'a' ");
          nbChar++;
          
        }
        /* pour le reste des chars on ajoute simplement des lettres*/
        else{
          fprintf(fichier, ", 'a' ");
          nbChar++;
        }
        
      }
      next = tab.tab_fonc[GLOBALE][i].suivant;

      while(next != NULL){

          
          if(next->type == MY_CHAR){

            if(nbChar == 0){
              fprintf(fichier, "global_Char : db 'a' ");
              nbChar++;
            }
            else{
              fprintf(fichier, ", 'a'");
            }
            nbChar++;
          }

          next = next->suivant;

      }
  }

  

  fprintf(fichier, "\n");

}

/* fonction qui gère les affectations */
void handleAffectation(FILE * fichier, Node * pere,  Node * node, char * currFonc, TabSymb tab1){

  Variable * variable1;

  /* cas d'arrêt */
  if(node == NULL)
    return;
  /* on vérifie qu'on est bien sur une affectation ou sur le premier fils d'une affectation*/
  if(node->label != affectation && pere->label != affectation){ 
    return;
  }
  /* on relance sur le premier fils si on est tombé sur le symbole d'affectation */

  if(node->label == affectation){

    handleAffectation(fichier,  node, node->firstChild, currFonc, tab1);
    /* on retire la right-value qui avait été empilée car on ne pouvait pas le faire avant la fin du traitement */
    fprintf(fichier,"pop rax ; on retire la valeur droite qui avait empilée car elle n'est plus utile \n");

    return;
  }

  /* on tombe sur la valeur gauche */
  if(node->label == variable && pere->label == affectation){

    /* on lance compute sur le fils pour récupérer le résultat du calcul (la valeur droite)*/
    compute(node->firstChild,  tab1, currFonc, fichier);

    /* on relance sur le frère gauche car c'est la deuxième valeur gauche*/
    handleAffectation(fichier, pere, node->nextSibling, currFonc, tab1);

    /* on récupère la variable dans la tab des symboles*/

    variable1 = findVariable2(tab1, node->valeur.text,currFonc);
    /* cas d'erreur qui n'est pas censé arriver : la variable n'existe pas*/
    if(variable1== NULL){
      printf("erreur cette variable locale n'existe pas ");
      return;
    }

    /* cas où on est tombé sur une variable locale */

    if(isLocalVariable(tab1, node->valeur.text, currFonc)){

      /* on récupère le résultat du dernier calcul qui a normalement été empilé (la right value)
      on ne fait pas un push car dans le cas où il y aurait plusieurs valeur gauche, l'une d'elle risquerait
      de ne pas recevoir le résultat de l'affectation*/
      /* cas où c'est une variable et pas un argument*/
      if(!thisVarIsArg(tab1, node->valeur.text, findFoncPos(tab1, currFonc))){
        fprintf(fichier,"mov rax, [rsp] ; on récupère le résultat du dernier calcul (la right value), on ne fait pas de pop car dans le cas où il y aurait plusieurs left-value l'une d'elle en aurait encore besoin \n");
        /* on met à jour la valeur associée à la variable grâce à son @*/
        fprintf(fichier,"mov [rbp-%d], rax ; on donne à la left value la r-value qui vient d'être mise dans rax \n", variable1->adresse);
      }
      /* cas où c'est un argument*/
      else{
        int totalArg;
        /* on vérifie si un bourrage est nécessaire */
        totalArg = compteTotalArg(tab1, currFonc);
        fprintf(fichier,"mov rax, [rsp] ; on récupère le résultat du dernier calcul (la right value), on ne fait pas de pop car dans le cas où il y aurait plusieurs left-value l'une d'elle en aurait encore besoin \n");
        /* on met à jour la valeur associée à la variable grâce à son @*/
        /* pas de bourrage */
        if(totalArg % 2 == 0)
          fprintf(fichier,"mov [rbp+%d], rax ; on donne à la left value la r-value qui vient d'être mise dans rax \n", variable1->adresse);
        /* un bourrage est nécessaire on incrémente l'@ de 16  */
        else{
          fprintf(fichier,"mov [rbp+%d], rax ; on donne à la left value la r-value qui vient d'être mise dans rax \n", variable1->adresse+8);
        }
      }
    }

    /* cas où on est tombé sur une variable globale */
    else{
      /* on met rax à 0 pour éviter les problèmes de valeurs résiduelles */
      fprintf(fichier,"mov rax, 0 ; on met rax à 0 pour éviter les problèmes de valeurs résiduelle \n");
      /* on récupère le résultat du dernier calcul qui a normalement été empilé (la right value)*/
      fprintf(fichier,"mov rax, [rsp] ; on récupère le résultat du dernier calcul (la right value), on ne fait pas de pop car dans le cas où il y aurait plusieurs left-value l'une d'elle en aurait encore besoin \n");

      /* cas d'un caractère*/
      if(variable1->type == MY_CHAR){
        /* on met à jour la valeur associée à la variable grâce à son @*/
        fprintf(fichier,"mov [global_Char+%d], al ; on met à jour la valeur associée à la variable globale \n", variable1->adresse);
      }
      /* cas d'un entier*/
      else{
         /* on met à jour la valeur associée à la variable grâce à son @*/
        fprintf(fichier,"mov [global_Int+%d], eax ; on met à jour la valeur associée à la variable globale \n", variable1->adresse);

      }

    }
  }


}

/* fonction qui gère les déclaration de variable locales mais qui ne gérera pas les affectations
les affectations à la déclaration seront gérées par d'autres variables */
void handleDeclLocale(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1){
  /* cas d'arrêt*/
  if(node == NULL)
    return;

  /* cas d'arrêt*/
  if(node->label != type && node->label != DeclVarsFonc && pere->label != type && node->label != affectation 
  && pere->label != affectation){
    return;
  }
  /* on est tombé sur la déclaration d'une variable locale*/
  if(node->label == variable && (pere->label == type || pere->label == affectation)){
  /* on alloue 8 octets sur la pile pour la variable locale */
    fprintf(fichier, "push 0 \n");
  }

  /* on relance récursivement sur les fils et les frères*/
  handleDeclLocale(fichier, pere, node->nextSibling, currFonc, tab1);
  handleDeclLocale(fichier, node, node->firstChild, currFonc,tab1);


}

/* fonction qui gère les appels de fonctions 
node correspond au noeud sur lequel on est actuellement*/
#if 0
int handleAppel(FILE * fichier, Node * pere, Node * node,  char * currFonc, TabSymb tab1){
  if(node == NULL)
    return 1;
  if(node->label == appelFonction){
    /* avant de lancer l'appel on empile/enregistres dans des registres les différents arguments nécessaires avec des appels récursifs */
    
    if(node->firstChild->firstChild->label != argument || strcmp(node->firstChild->firstChild->valeur.text, "void") != 0){
      handleAppel(fichier,node->firstChild,node->firstChild->firstChild,currFonc,tab1);
    }
    
    /* on lance l'appel de fonction après avoir récupéré les arguments*/
    fprintf(fichier, "call %s \n", node->valeur.text);
  }

  return 1;
}
#endif


/* fonction qui gère le début des structures avec un "if" */

void handleIf(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1){
  #if 0
  if(node->label == If){

    /*on fait des appels récursifs */

    handleIf(fichier, node,node->firstChild,currFonc, tab1);

    /*puis on effectue la comparaison avec les éléments empilés*/


  }

  /* cas où on tombe sur un opérande de comparaison */

  if(node->label == operateur){

    /* opérande (gauche ou droite) d'une comparaison: on calcule et on empile */

    compute(node, tab1, currFonc, fichier);

  }

  else if(node->label == character){

    /* opérande (gauche ou droite) d'une comparaison: on calcule et on empile */

    compute(node, tab1, currFonc, fichier);

  }
  
  else if(node->label == num){

    /* opérande (gauche ou droite) d'une comparaison: on calcule et on empile */

    compute(node, tab1, currFonc, fichier);

  }

  else if(node->label == variable){

    /* opérande (gauche ou droite) d'une comparaison: on calcule et on empile */

    compute(node, tab1, currFonc, fichier);

  }

  /* on fait un calcul que l'on empile car on tombe sur une comparaison */

  else if(node->label == comparateur){
  
    /* on va empiler les résultats des calculs des deux côtés des opérateurs de comparaison */
    /*
    compute(node->ne);
    */




  }
  #endif
  

}

/* fonction qui gère les returns dans une fonction */

void handleReturn(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1){
  /* cas d'arrêt */
  if(node == NULL)
    return;

  if(node->label == Return){

    /* on calcule la valeur de retour et on l'empile noeud*/

    compute(node->firstChild, tab1, currFonc, fichier);
    fprintf(fichier, "pop rax ; on récupère la valeur de retour qui avait été empilée par 'compute' et on la met dans le registre associé aux valeurs de retour \n");

    /* cas où on est pas dans la fonction main */

    if(strcmp(currFonc, "main") != 0){
      fprintf(fichier, "pop rbp ; on redonne à rbp son ancienne valeur de début de pile \n");
      fprintf(fichier, "ret ; on termine la fonction \n");
    }
    

    /* gérer séparément le cas où on est dans la fonction main */
    else{
      fprintf(fichier, "mov rdi, rax ; on met dans rdi la valeur de retour qui était dans rax \n");
      fprintf(fichier, "mov rax, 60 ; code nécessaire pour indiquer la fin du programme; \nsyscall\n");
    }

  }
}

/* 
fonction qui gère les appels de fonctions 
compteurArgs : variable qui compte le nombre d'arguments qu'on a déjà ajouté.
*/
void handleAppelFonc(FILE * fichier, Node * pere, Node * node, char * currFonc, TabSymb tab1, int * compteurArgs){
  int i;
  int totalPop;
  /* cas d'arrêt */

  if(node == NULL){

    return;

  }

  /* on tombe sur l'appel de fonction : on relance la fonction sur les fils pour récupérer les arguments */
  
  if(node->label == appelFonction){
    /* ce cas gère seulement les appels de fonctions isolés mais non associés à des 
    affectations
    */
    if(pere->label == suiteInstr ){

        /* on traite les arguments */
        fprintf(fichier, "; test : on gère les arguments avant d'appeler la fonction \n\n");
        handleAppelFonc(fichier, node, node->firstChild,currFonc,tab1, compteurArgs);

        /* une fois qu'on a traité tous les arguments on vérifie si du bourrage est nécessaire et on fait l'appel*/

        if((*compteurArgs) % 2 != 0){

          /* on a un nombre impair d'arguments on fait du bourrage*/

          fprintf(fichier, "push 0 ; valeur de bourrage pour retourner sur un multiple de 16 sur la pile \n");
        }

        /* on lance l'appel de la fonction*/
        
        fprintf(fichier, "call %s ; appel de la fonction qui a reçu ses arguments \n", node->valeur.text);
      
        /* on dépile les arguments qu'on avait empilé pour l'appel (Début)*/

        totalPop = 0;

        /* on compte le nombre total de pop à faire pour dépiler tous les arguments qui avaient été utilisés lors de l'appel*/

        totalPop = compteTotalArg(tab1, node->valeur.text);
        if(totalPop % 2 == 0){
          for(i=0; i<totalPop;i++){
            fprintf(fichier, "pop r8 ; on dépile les arguments après l'appel \n");
          }
        }
        else{
          for(i=0; i<totalPop+1;i++){
            fprintf(fichier, "pop  r8  ; on dépile les arguments après l'appel \n");
          }
        }

        /* on dépile les arguments qu'on avait empilé pour l'appel (FIN)*/
    }
  }

  /* cas où on tombe sur le noeud "argument"*/
  else if(node->label == argument ){
      /* on traite les arguments récursivement */
      handleAppelFonc(fichier, node, node->firstChild,currFonc,tab1, compteurArgs);
  }

  /* cas où on tombe sur un argument*/
  else{

      if(pere->label == argument){
          /* on incrémente le nombre d'arguments qu'on a trouvé pour ensuite faire du bourrage de pile
          si nécessaire */
          (*compteurArgs)++;
          /* on traite l'argument en empilant sa valeur*/
          compute2(pere, node, tab1, currFonc, fichier);
          /* on traite les autres arguments s'il y en a éventuellement en empilant aussi leurs résultats*/
          handleAppelFonc(fichier, pere, node->nextSibling, currFonc, tab1, compteurArgs);
      }

  }
}

/* fichier qui gère de façon globale la génération du fichier NASM 
fichier qui va recevoir le code nasm
node le noeud sur lequel on est actuellement
currFonc : indique la fonction dans laquelle on est 
tab1 la tab des symboles
pere est le pere du node
*/
int creationNasm(FILE * fichier,Node * pere, Node * node, char * currFonc, TabSymb tab1){
  if(node == NULL) 
    return 1;

  /* on tombe sur une déclaration de fonction qui n'est pas le main, on lui associe une étiquette dans le fichier assembleur*/

  if(node->label == id && strcmp(node->valeur.text, "main") != 0){

    fprintf(fichier, "\n %s: ; etiquette de la fonction \n", node->valeur.text);
    fprintf(fichier, "push rbp ;on empile l'@ de rbp pour conserver l'ancien bloc d'activation \n");
    fprintf(fichier, "mov rbp, rsp ;on met à jour rbp pour le nouveau bloc d'activation \n");
  }
  
  /* cas où on entre dans le main*/
  else if(node->label == id && strcmp(node->valeur.text, "main") == 0){
    beginMainFile(fichier);
    fprintf(fichier, "mov rbp, rsp ;on met à jour rbp pour le nouveau bloc d'activation \n");
  }

  /* si on est tombé sur une fonction on relance l'appel en précisant qu'on entre dans une nouvelle
  fonction pour adapter les noms des variables locales*/
  if(node->label == id){
    creationNasm(fichier, node, node->firstChild, node->valeur.text, tab1);
    creationNasm(fichier, pere, node->nextSibling, node->valeur.text, tab1);
    return 1;
  }


  /* cas où on tombe sur une affectation : on utilise le gestionnaire d'affectation */
  else if(node->label == affectation){
    handleAffectation(fichier, pere,  node, currFonc, tab1);
  }
  
  /* on tombe sur la déclaration des variables locales d'une fonction*/
  else if(node->label == DeclVarsFonc){
    handleDeclLocale(fichier, pere, node, currFonc, tab1);
  }

  /* cas où on tombe sur un appel de fonction : on utilise le gestionnaire d'appel de fonction*/
  else if(node->label == appelFonction){
    /* on initialise un compteur d'arguments pour savoir le nombre d'arguments qu'on a croisé */
    int compteurArgs = 0;
    fprintf(fichier, "; juste avant l'appel de fonction \n");
    handleAppelFonc(fichier, pere, node, currFonc, tab1, &compteurArgs);
    fprintf(fichier, "; juste après l' appel de fonction \n");
  }

  #if 0
  /* on tombe sur un if, on gère le "if" */
  else if(node->label == If){
    handleIf(fichier, node, node, currFonc, tab1);
  }
  #endif

  else if(node->label == Return){
    handleReturn(fichier, node, node, currFonc, tab1);
  }
  #if 0
  /* on tombe sur else, on gère le "else" */
  else if(node->label == Else){
    handleElse(fichier, node, node, currFonc, tab1);
  }
  #endif
  

  /* cas où on ne tombe pas sur le noeud d'une fonction : on lance un appel récursif classique*/

  creationNasm(fichier, node, node->firstChild, currFonc, tab1);
  creationNasm(fichier, pere, node->nextSibling, currFonc, tab1);
  
  
  return 1;
}
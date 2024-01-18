%{
/* tpc2022-2023 */


#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "../src/tree.h"
#include "../src/analyseTree.h"
#include "../src/anonymous.h"
#include "../src/generationAssembleur.h"

/* il y a 4 options possibles
--tree
-t  
-h
--help*/
#define OPTION 4 
#define OPTION_LEN 50


TabSymb tabSymb1;
void yyerror(char * s);
int yylex();
int yyparse();
int tflag = 0;
int symbFlag = 0;
/* variable qui va récupérer le nom du fichier choisi par le user*/
char * fileName;
extern int lineno;
extern int carno;
/* noeud qui sert à faire des parcours d'arbre*/
Node * nodeTmp;

%}

/* les éléments déclarés ici sont ceux qui sont tous ceux renvoyés
dans le fichier ".lex" que ce soit sous forme d'entier, de char, d'int, etc...
Selon si la valeur de yylval qui leur est associée est un int, un char, un char *, etc... il faudra préciser plus bas
par "%type <qqc>" le type qui leur est associé.

-Si yylval est associé à un entier alors ce sera:
"%type<entier>" TYPE, pour dire que "TYPE" renvoie un entier (car entier est associé à "int").

-Si yylval est associé à une chaine de caractère alors ce sera:
"%type<text> IF", pour dire que "IF" renvoie un type associé à "text", c'est à dire "char *".


Pour savoir quel mot-clé écrire entre les chevrons il faut regarder l'union définie plus bas.
On retrouve les mots-clés "node", "text", "entier", "car" (choisis par nous-mêmes).

En principe les états non-terminaux auront un noeud associé à leur lexème, tandis
que les terminaux auront plutôt un type comme char, int, char *.

*/

%token TYPE
%token VOID
%token IDENT
%token IF
%token ELSE
%token WHILE
%token RETURN
%token OR
%token AND
%token EQ
%token ORDER
%token ADDSUB
%token DIVSTAR
%token NUM
%token CHARACTER


%union {
    Node * node; /*un pointeur sur un noeud*/
	char * text;	/*  chaine de caractère pour identifier  */
	int entier;
	char car; /*reconnait un caractère */
}

/* pour tous les éléments associés à la partie "<text>",
dans le fichier tpcc.y,
il faut donc faire un malloc pour récupérer le text associé et 
l'enregistrer dans la variable yylval. 
en faisant :
1) yylval.text = (char *) malloc(sizeof((char)*(strlen(yytext)+1)));
2) puis : strcpy(yytext.text, yytext));
même chose avec les éléments dans "car", il faut faire
yylval.car = yytext[0];
même chose avec NUM :
yylval.entier = NUM;
même chose avec ADDSUB,DIVSTAR...
*/
%type <text>  IDENT  ORDER EQ TYPE VOID IF ELSE WHILE
%type <entier> NUM 
%type <car> CHARACTER ADDSUB DIVSTAR 
%type <node> Prog DeclVars LValue  DeclFoncts EnTeteFonct Corps Parametres SuiteInstr DeclarateursFonc DeclVarsFonc
%type <node> Declarateurs DeclFonct ListTypVar Instr Arguments 
%type <node> Exp TB M E FB T F ListExp


%%

Prog:  DeclVars DeclFoncts 
        {  
            Node * node1;
            Node * node3;

            /*
            -je créé le noeud dont l'étiquette est "PROG"
            -ce n'est ni une fonction, ni une variable donc implicite
            a comme type "TYPE_OTHER"
            -il n'a pas de type de retour ni de de type (ce n'est ni
            un int, ni un char) donc "NONE_TYPE"
            */
           
            //newFile();
            node1 = makeNode(PROG, TYPE_OTHER, MY_NONE,NONE);
            /* le noeud "prog" aura pour fils les résultats de DeclVars ($1)
            et de DeclFoncts ($2)*/
            node1 = makeNode(PROG, TYPE_OTHER, MY_NONE,NONE);
            node3 = makeNode(DeclFoncts,TYPE_OTHER,MY_NONE,NONE);
            //node2 = makeNode(DeclVars,TYPE_OTHER,MY_NONE,NONE);

            
            /* declfonct prendra pour fils le résultat des déclarations de fonctions.
            Et declvars prendra pour fils le résultat des déclarations des variables globales */
            addChild(node3,$2);
            // addChild(node2,$1);
            /* prog prendra pour 2 fils les résultats de declvars et de delcfoncts */
            addChild(node1,$1);
            addChild(node1,node3);

            /* ici on fait l'analyse sémantique (début) */
                int retour;
                completerNode(node1, NULL);
                init_tab_symb(&tabSymb1);
                addGlobaleParDefaut(&tabSymb1);
                /*on ajoute les variables et les fonctions de base*/
                retour = remplirTabSymb(&tabSymb1, node1, NULL);

                if(retour == 0){
                    /* erreur sémantique */
                    return 2;
                }

                /* on ajoute les fonctions de la libraire standard */
                retour = addLibStandard(&tabSymb1);
                completerAdresseArgs(&tabSymb1);
                /* cas où le user a ajouté deux fois le même identificateur */
                if(retour == 0){
                    /* erreur sémantique */
                    return 2;
                }
                /* plus d'allocation possibles */
                else if(retour == 3){
                    return 3;
                }
                
                if(checkVariableParametresAppel(node1, tabSymb1, "globale") == 1){

                    retour = checkAppelFonc(node1, "globale", tabSymb1);

                }
                else{
                    retour = 0;
                }
                

                
                /* s'il n'y a pas eu d'erreur aux étapes précédentes on vérifie les affectations */

                if(retour != 0){

                    retour = checkAffectation(NULL, NULL, node1, tabSymb1, NULL , NULL);
                    /* cas d'erreur sémantique*/
                    if(retour == 0){
                        return 2;
                    }
                }

                else{

                    /* cas d'erreur sémantique */

                    return 2;
                }

                if(checkComparaison(node1, tabSymb1) == 0){
                    /* cas d'erreur sémantique : comparaison avec du void*/
                    return 2;
                }
                
                /* on vérifie si il y a bien un main */
                if(checkIfMainIsHere(tabSymb1) == 0){
                /* pas de main : erreur */
                    return 2;
                }
                /* on recupère les valeurs de retour des fonctions */

                addReturnValue(&tabSymb1, NULL, node1, NULL, NULL);
                
                /* on compare les types reçus avec les types attendus en valeur de retour */
                
                retour = compareTypeRecToExp(tabSymb1);

                /*cas d'erreur sémantique valeur de retour incompatibles */
                if(retour == 0){
                    return 2;
                }

            /* ici on fait l'analyse sémantique (FIN) */

            if(symbFlag == 1){
                /*on affiche la tab des symboles */
                showTabSymb(tabSymb1);
            }
            /* le flag associé à l'affichage de l'arbre est activé*/
            if(tflag == 1){
                /* on affiche l'arbre et la tab des symboles ssi il n'y a pas eu d'erreur sémantique ni syntaxique */
                if(retour != 0){
                    printf("\n\n ------------------------- -------------------------Arbre absrait------------------------- ------------------------- \n\n\n");
                    printTree(node1);
                    printf("\n\n\n");
                    
                }
            }
                
            /* partie assembleur, création du fichier assembleur */
            if(retour != 0){
                FILE * fichier;
                /* contient le nom du fichier */
                /*char filePath[500];
                strcpy(filePath, "bin/");
                strcat(filePath, fileName);*/
                if((fichier = fopen("bin/anonymous.asm", "w")) == NULL){
                    printf("Echec de lecture de fichier \n\n");
                    exit(1);
                }
                /*newFile(fichier);*/
                ajoutGlobal(fichier, tabSymb1);
                /*compute(node1, fichier);*/
                /* ajout des imports et des éléments de base de */
                enteteBasique(fichier);
                /* gestion global du fichier nasm*/
                /* on commence directment par la partie DeclFoncts*/

                /* cas avec les variables globales */
                if(node1->firstChild->label == DeclVars){
                    creationNasm(fichier, node1,node1->firstChild->nextSibling, NULL,tabSymb1);
                }
                /*cas sans variables globales */
                else if(node1->firstChild->label == DeclFoncts){
                    creationNasm(fichier, node1,node1->firstChild, NULL,tabSymb1);
                }
                /*endFile(fichier);*/
                    
                fclose(fichier);
            }
            
                
            /* pas d'erreur */
            return 0;
            
            
        } 
        ;
    

DeclVars:
       DeclVars TYPE Declarateurs ';' 
    {
        /* on récupère DeclVars pour préciser qu'on est dans
        une déclaration de variable*/
        Node * node1;
        Node * node2;
       
        node1 = makeNode(DeclVars,TYPE_OTHER,MY_NONE,NONE);

        /* on met la valeur de DeclVars dans $$ si elle n'est pas nulle*/
        if($1 != NULL){
            /* le résultat de la valeur la plus à gauche qui sera en fait une valeur non-nulle
            contenant Declvars père de type,est donné à $$.
            Le DeclVars de la valeur la plus à gauche sera le résultat renvoyé par $$*/
            $$ = $1;
             
            /* node2 contiendra le type de l'élément le plus à droite*/
            node2 = makeNode(type,TYPE_OTHER,MY_NONE,TEXT); 
            /* on récupère le nom exact du type (int ou char)*/
            if(copieChaineNoeud(node2, $2) == 0){
            /* erreur échec du malloc*/
                return 2;
            }
            /* declvar prend pour dernier fils le type des derniers (tout à droite) éléments  qui lui sont associés */
            addChild($1,node2);
            /*  "type" prend pour fils le résultat de Declarateurs
            c'est à dire, la succession d'identifcateurs qui auront été déclarés et qui
            sont de type "type"*/
            addChild(node2, $3); 
         }
         /* sinon on prend la valeur de "type declarateurs" */
         else{
            $$ = node1; /* on affecte node1 à $$ car c'est 
            le contenu de $$ qui sera ensuite donné comme
            résultat de "Declvars" à la variable "node1" de la partie PROG*/

            node2 = makeNode(type,TYPE_OTHER,MY_NONE,TEXT); 
            /* on récupère le nom exact du type (int ou char)*/
            if(copieChaineNoeud(node2, $2) == 0){
            /* erreur échec du malloc*/
                return 2;
            }
            /* declvar prend pour fils le type des éléments qui lui sont associés*/
            addChild(node1, node2);
            /*  "type" prend pour fils le résultat de Declarateurs ($3)
            c'est à dire, la succession d'identifcateurs qui auront été déclarés et qui
            sont de type "type"*/
            addChild(node2, $3); 

         }


    }
    | 
    /*lorsque on ne fait rien avec le noeud, il faut remplir $$ avec "NULL"*/
    {$$ = NULL;} 

    
    ;
    
    
Declarateurs:
       Declarateurs ',' IDENT
       {
           /* un exemple
           int x,y,z,a;
           */
           Node * node2;
          
           /*node1 = makeNode(variable, TYPE_VAR, MY_NONE,TEXT); à supprimer*/
            

           /* on ajoute la succession 
           d'éléments "Declarateurs" représentés
           par $1 (ce seront tous des identificateurs), en frère 
           de l'identificateur en tête de file représenté par "node1"*/

           /*addSibling(node1, $1); à supprimer */

           /* la succession d'ident à gauche de la virgule (représentés par "Declarateurs") 
           seront les frères du dernier élément déclaré (l'IDENT à droite de la virgule)*/
           $$ = $1;

           /*on créé un noeud pour l'identificateur qui est tout à droite pour le rattacher à l'arbre*/
            node2 = makeNode(variable, TYPE_VAR, MY_NONE,TEXT);
            /* échec du malloc*/
            if(copieChaineNoeud(node2,$3) == 0){
                return 2;
            }
            addSibling($$,node2);

           
       }
    
    |  IDENT 
    
    {
        /*on créé un noeud pour stocker l'identificateur.
        à partir d'ici on ne peut pas connaitre son type donc on met provisoirement
        "MY_NONE"*/

        Node * node1 = makeNode(variable, TYPE_VAR,MY_NONE,TEXT);
        /* on enregistre le nom de la variable*/

        if(copieChaineNoeud(node1, $1) == 0){
            /* échec du malloc*/
            return 2;
        }

        /*on met le contenu de "node1" dans "$$" pour le raccrocher au reste
        de l'arbre car le contenu de $$ est considéré comme 
        le résultat "IDENT" */

        $$=node1;
    }
    
    ;
DeclFoncts:
       DeclFoncts DeclFonct 
    {
        Node * node1;
        
        /*node1 = makeNode(DeclFoncts,TYPE_OTHER,MY_NONE,NONE);*/
        
        /* les différentes fonctions qui seront déclarées seront les fils
        du noeud node1*/
        node1 = $1;
        $$ = node1;
        
        /* la dernière fonction déclarée (=$2), sera le frère des autres 
        fonctions (=$1) */
        addSibling(node1, $2);
        //addSibling($1, $2);
    }
    |  DeclFonct 
    {
        Node * node1;
        /*node1 = makeNode(DeclFoncts,TYPE_OTHER,MY_NONE,NONE);*/
        /* DeclFonct, renverra le résultat de $1*/
        /*addChild(node1, $1);*/
        node1 = $1;
        $$ = node1;

    }
    ;
DeclFonct:
       EnTeteFonct Corps
       {
           /* récupère le résultat "d'entête fonction"*/
           $$ = $1;
           /* le "Corps" de la fonction est considéré comme son fils*/
          addChild($1,$2);

       }
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'
       { 
        /*
        if(strcmp("main", $2) == 0) {
            

        }*/
       } 
       {
        Node * node1;
        Node * node2;
        Node * node3;
        

        /* on créé un noeud auquel on associe le type de retour de la 
        fonction qu'on a trouvé
        (soit int soit char)*/

        node1 = makeNode(typeRetour,TYPE_OTHER,MY_NONE,TEXT);
        if(copieChaineNoeud(node1, $1) == 0){
            /* échec du malloc*/
            return 2;
        }

        /* on donne à ce "noeud2" comme fils (noeud2) le type de retour 
        associé à la fonction */

        node2 = makeNode(id,TYPE_FUN, typeStrToInt($1),TEXT);
        if(copieChaineNoeud(node2, $2) == 0){
            /* échec du malloc*/
            return 2;
        }
        /*l'identificateur de la fonction sera le père des noeuds liés à l'entête*/
        $$ = node2;
        addChild(node2,node1);
        /*node3 devient le père des paramètres de la fonction*/

        node3 = makeNode(Parametres,TYPE_OTHER,MY_NONE,NONE);
        addChild(node3, $4);

        /*on donne a ce "noeud2" comme deuxième fils "node3" qui 
        est le père qui a pour fils les paramètres de la fonction*/

        addChild(node2, node3);
        }

    |  VOID IDENT '(' Parametres ')' 

    { 
        Node * node1;
        Node * node2;
        Node * node3;

        /*on récupère l'identifcateur de la fonction 
        qui sera le noeud père des autres noeuds
        formés à partir de l'en-tête*/

        
        node1 = makeNode(id,TYPE_FUN,MY_VOID,TEXT);
        $$ = node1;
        if(copieChaineNoeud(node1,$2) == 0){
            /* échec du malloc*/
            return 2;
        }

        /*on copie ensuite le type de retour (ici void) pour l'associé à
        node2 pour placé un noeud avec le type de retour dans l'arbre.
        */

        node2 = makeNode(typeRetour,TYPE_OTHER,MY_NONE,TEXT);
        if(copieChaineNoeud(node2, "void") == 0){
            /* échec du malloc*/
            return 2;
        }
        /*le noeud avec le type de retour devient le fils de node1*/
        addChild(node1,node2);
        /* les paramètres seront aussi les fils de node3 c'est à dire
        de l'identificateur qui identifie la fonction mais le label "paramètre"
        sera le fils de node1
        */
        node3 = makeNode(Parametres,TYPE_OTHER,MY_NONE,NONE);
        addChild(node1, node3);
        addChild(node3, $4);

    
    
    }
    ;
Parametres:
       VOID {
            /* cas où il y a aucun paramètre : on créé
            un noeud qui contiendra le text "void"*/

            Node * node1;
            //Node * node2;
            
            /* si le paramètre est void, on enregistre le fait que ce soit void*/
            node1 = makeNode(Void,TYPE_PARAM,MY_VOID,TEXT);
            /*node2 = makeNode(Parametres,TYPE_OTHER,MY_NONE,NONE);
            addChild(node2, node1);*/
            $$=node1;
            if(copieChaineNoeud(node1, "void") == 0){
                /* échec du malloc*/
                return 2;
            }
           }
    |  ListTypVar 
    {
        /* si on tombe sur une liste de variable, on récupère le résultat
        de cette liste de variable*/
        $$=$1;
    }
    ;

ListTypVar: /* cas de la forme
int x, char y, int xy;
c'est à dire un cas avec une déclaration de plusieurs variables
de différents types, en même temps.
*/
       ListTypVar ',' TYPE IDENT 
       
    {
        Node * node2;
        Node * node3;

        /*l'ensemble des noeuds de la longue liste sera
        la première variable croisée mais le noeud node1 sera considéré
        comme le dernier de la longue liste*/

        $$=$1;

        
        /* on copie le type du paramètre le plus à droite dans un noeud */

        node2 = makeNode(type,TYPE_OTHER,MY_NONE,TEXT);

        addSibling($1,node2);

        /* on copie le type du paramètre dans la chaine*/
        if(copieChaineNoeud(node2,$3) == 0){
            /* échec du malloc*/
            return 2;
        }

        /*on créé un noeud pour la variable la plus à droite*/

        node3 = makeNode(parametre,TYPE_VAR,MY_NONE,TEXT);
        
        if(copieChaineNoeud(node3, $4) == 0){
            /* échec du malloc*/
            return 2;
        }

        /*on écrit sous forme d'un entier le type de la variable associé à node3*/
        /*copieIntType(node3, $3);*/

        /* le dernier paramètre a pour père son propre type*/
        addChild(node2, node3);

    }
    |  TYPE IDENT 
    {
         Node * node1;
         Node * node2;

        /*la longue liste ne contient en fait qu'un seul éléments.
        On ajoute cet élément à "node1" et on fait pointer $$ sur node1 */

        
        node1 = makeNode(parametre,TYPE_PARAM,MY_NONE,TEXT);
        /* on recopie le nom de l'identificateur dans le noeud "node1"*/

        if(copieChaineNoeud(node1, $2) == 0){
            /* échec du malloc*/
            return 2;
        }

        /* on copie le type du paramètre dans une chaine */
        node2 = makeNode(type,TYPE_OTHER,MY_NONE,TEXT);
        $$ = node2;
        /* on copie le type du paramètre dans le noeud*/
        if(copieChaineNoeud(node2,$1) == 0){
            /* échec du malloc*/
            return 2;
        }

        /*le type a pour fils le paramètre qui lui est associé*/     
        addChild(node2, node1);


    }
    ;
Corps: '{' DeclVarsFonc SuiteInstr '}' 
    {
        Node * node1;
        Node * node2;
       
        /* la déclaration de variable et la suite d'instructions
        seront frères (donc node1 et node2 seront frères)*/

        node1 = $2;//makeNode(DeclVarsFonc,TYPE_OTHER,MY_NONE,NONE);
        
        $$ = node1;

        //on créé un noeud pour les suiteINSTR
        node2 = makeNode(suiteInstr,TYPE_OTHER,MY_NONE,NONE);
        //addSibling(node1,node2);
        /*les résultats de DeclVarsFonc et de suiteInstr seront respectivement
        les fils de node1 et de node2 (faux)*/
        //addChild(node1, $2);
        addChild(node2, $3);
        if(node1 != NULL)
            addSibling(node1, node2);
        /* cas où il y a directement une instruction.
        Généralement ce sera un cas où il y aura directement un return.
        Mais ça peut aussi être directement un traitement sur les arguments */
        else{
            $$ = node2;
        }
    
    }
    ;

DeclVarsFonc:
       DeclVarsFonc TYPE DeclarateursFonc ';' 
    {
        


        Node * node1;
        node1 = makeNode(DeclVarsFonc,TYPE_OTHER,MY_NONE,NONE);

        /* cas où on est pas sur la valeur la plus à gauche */

        if($1 != NULL ){
            /* on récupère DeclVars pour préciser qu'on est dans
            une déclaration de variable*/
            
            Node * node2;

            $$ = $1; /* on affecte $1 à $$ car c'est 
            le contenu de DeclVars (l'élément le plus à gauche) qui sera ensuite donné comme
            résultat de "Declvars" à la variable "node1" de la partie PROG*/

            node2 = makeNode(type,TYPE_OTHER,MY_NONE,TEXT); 
            /* on récupère le nom exact du type (int ou char)*/
            if(copieChaineNoeud(node2, $2) == 0){
            /* erreur échec du malloc*/
                return 2;
            }
            /* declvar prend pour fils le type des éléments qui lui sont associés*/
            addChild($1, node2);
            /*  "type" prend pour fils le résultat de Declarateurs
            c'est à dire, la succession d'identifcateurs qui auront été déclarés et qui
            sont de type "type"*/
            addChild(node2, $3); 
        }

        /* cas où on est sur la valeur la plus à gauche */
        else{
            /* on récupère DeclVars pour préciser qu'on est dans
            une déclaration de variables */
            
            Node * node2;

             $$ = node1;  /* on affecte node1 à $$ car c'est 
            le contenu de $$ qui sera ensuite donné comme
            résultat de "Declvars" à la variable "node1" de la partie PROG*/

            node2 = makeNode(type,TYPE_OTHER,MY_NONE,TEXT); 
            //$$ = node2;
            /* on récupère le nom exact du type (int ou char)*/
            if(copieChaineNoeud(node2, $2) == 0){
            /* erreur échec du malloc*/
                return 2;
            }
            /* declvar prend pour fils le type des éléments qui lui sont associés*/
            addChild(node1, node2);
            /*  "type" prend pour fils le résultat de Declarateurs
            c'est à dire, la succession d'identifcateurs qui auront été déclarés et qui
            sont de type "type"*/
            addChild(node2, $3); 

        }


    }
    | 
    /*lorsque on ne fait rien avec le noeud, il faut remplir $$ avec "NULL"*/
    {$$ = NULL;} 

    
    ;
    
/*un déclarateur dans le cas où on est dans une fonction*/   
DeclarateursFonc:
       DeclarateursFonc ',' IDENT
       {
           /* un exemple
           int x,y,z,a;
           */
           Node * node2;
          
           /*node1 = makeNode(variable, TYPE_VAR, MY_NONE,TEXT); à supprimer*/
            

           /* on ajoute la succession 
           d'éléments "Declarateurs" représentés
           par $1 (ce seront tous des identificateurs), en frère 
           de l'identificateur en tête de file représenté par "node1"*/

           /*addSibling(node1, $1); à supprimer */

           /* la succession d'ident à gauche de la virgule (représentés par "Declarateurs") 
           seront les frères du dernier élément déclaré (l'IDENT à droite de la virgule)*/
           $$ = $1;

           /*on créé un noeud pour l'identificateur qui est tout à droite pour le rattacher à l'arbre*/
            node2 = makeNode(variable, TYPE_VAR, MY_NONE,TEXT);
            /* échec du malloc*/
            if(copieChaineNoeud(node2,$3) == 0){
                return 2;
            }
            addSibling($$,node2);

           
       }
    
    |  IDENT 
    
    {
        /*on créé un noeud pour stocker l'identificateur.
        à partir d'ici on ne peut pas connaitre son type donc on met provisoirement
        "MY_NONE"*/

        Node * node1 = makeNode(variable, TYPE_VAR,MY_NONE,TEXT);
        /* on enregistre le nom de la variable*/

        if(copieChaineNoeud(node1, $1) == 0){
            /* échec du malloc*/
            return 2;
        }

        /*on met le contenu de "node1" dans "$$" pour le raccrocher au reste
        de l'arbre car le contenu de $$ est considéré comme 
        le résultat "IDENT" */

        $$=node1;
    }

    /* une suite de déclaration d'identificateurs auxquels on associe une expression
        dans le cas où on se trouve dans une fonction
    */

    |  DeclarateursFonc ',' IDENT '=' Exp 

    {
        Node * node1;
        Node * node2;

        node1 = makeNode(affectation,TYPE_OTHER,MY_NONE,NONE);

        /*on créé un noeud associé à l'identificateur qu'on a trouvé*/

        node2 = makeNode(variable, TYPE_VAR, MY_NONE,TEXT);
        if(copieChaineNoeud(node2,$3) == 0){
            /* échec du malloc*/
            return 2;
        }

        /* le symbole d'affectation est le père de la valeur gauche (IDENT)*/

        addChild(node1, node2);

        /*la valeur gauche est père de l'expression (EXP : LA VALEUR droite)*/

        addChild(node2,$5);

        
        

        /* 
        si le déclarateurs à gauche est aussi une affectation alors les 
        deux affectations sont frères (les deux noeuds sont frères).*/
        if($1->label == affectation){
            addSibling($1,node1);
            /* $$ prend pour retour le premier noeud qui est une affectation*/ 
            $$ = $1;
        }

        /*si le déclarateur à gauche est une variable à laquelle a
        aucune affectation n'a été fait alors son résultat est fils 
        de l'affectation (fils de node1)*/
        else {
           
            addChild(node1, $1);
            /* $$ prend pour retour node 1 qui est l'affectation*/
            $$ = node1;
        }



        
    }

    /* un seul identifcateur auxquels on associe une expression*/

    | IDENT '=' Exp
    {
        Node * node1;
        Node * node2;

        node1 = makeNode(affectation, TYPE_OTHER, MY_NONE,NONE);
        $$ = node1;
        /* Le type de la variable est inconnu à partir d'ici donc on lui écrira "MY_NONE" provisoirement*/
        node2 = makeNode(variable, TYPE_VAR, MY_NONE,TEXT);

        if(copieChaineNoeud(node2,$1) ==0){
            /* échec du malloc*/
            return 2;
        }

        /* l'affectation a pour fils l'identifcateur qui correspond à la valeur gauche*/
        addChild(node1,node2);
        /*la valeur droite est le fils de la valeur gauche*/
        addChild(node2,$3);

    }
    
    ;





SuiteInstr:
       SuiteInstr Instr 
       
       {
           Node * node1;
           
           /* on créé un noeud pour indiquer qu'on est tombé sur une
           suite d'instructions (à supprimer)*/
           //node1 = makeNode(suiteInstr,TYPE_OTHER,MY_NONE,NONE);
            node1 = $1;
            //$$=node1;
           /* les instructions seront frères les unes des autres (à supprimer)*/
           //addChild(node1, $1);
           /*la dernière instruction sera le dernier frère (à supprimer)*/
            //addChild(node1, $2);

            //les instructions sont frères
            if(node1 != NULL)
                addSibling($1, $2);
            else{
                node1 = $2;
                $$ = node1;
            }

           
        }

    | {
        /* il n'y a pas d'instruction donc $$ prend la valeur null*/
        $$= NULL;
        }
    ;
Instr:
       LValue '=' Exp ';' {

           Node * node1;
           node1 = makeNode(affectation,TYPE_OTHER,MY_NONE,NONE);
            $$ = node1;
           /*le symbole d'affectation aura pour premier fils l'élément
           à gauche de l'affectation (la LValue)*/

           addChild(node1, $1);

           /* la left-value aura pour frère les rights values représentées par Exp*/

           addChild($1,$3);
           

       }

    |  IF '(' Exp ')' Instr 
        {
            Node * node1;
            
            /*on créé un noeud pour "if" qui sera le père de 
            l'expression et des instructions*/
            node1 = makeNode(conditionnel,TYPE_OTHER,MY_NONE,TEXT);
            $$ = node1;
            if(copieChaineNoeud(node1, "if") == 0){
                // échec du malloc
                return 2;
            }
           
            addChild(node1,$3);
          
            /* l'expression dans les parenthèses du "if"
            et les instructions dans les accolades sont frères*/
            addSibling($3,$5);
       }
    |  IF '(' Exp ')' Instr ELSE Instr
        {
            Node * node1;
            Node * node2;
           

            /*on créé un noeud pour "if" qui sera le père de 
            l'expression et des instructions*/

            node1 = makeNode(conditionnel,TYPE_OTHER,MY_NONE,TEXT);
            $$ = node1;
            if(copieChaineNoeud(node1, "if") == 0){
                /* échec du malloc*/
                return 2;
            }
            addChild(node1,$3);

            /* l'expression dans les parenthèses du "if"
            et les instructions dans les accolades sont frères*/

            addSibling($3,$5);
            /*on créé un noeud pour "else" qui sera le père de 
            des instructions numéro 2 */
            node2 = makeNode(conditionnel,TYPE_OTHER,MY_NONE,TEXT);
            if(copieChaineNoeud(node2, "else") == 0){
                /* échec du malloc*/
                return 2;
            }

            /* le résultat de INSTR sera le fils de "else" */

            addChild(node2,$7);

            /* "if" et "else" sont frères*/

            addSibling(node1, node2);



       }
    |  WHILE '(' Exp ')' Instr
        {
         Node * node1;
        

            /*on créé un noeud pour "while" qui sera le père de 
            l'expression et des instructions*/

            node1 = makeNode(conditionnel,TYPE_OTHER,MY_NONE,TEXT);
             $$ = node1;
            if(copieChaineNoeud(node1, "while") == 0){
                /* échec du malloc*/
                 return 2;
            }

            addChild(node1,$3);

            /* l'expression dans les parenthèses du "while"
            et les instructions dans les accolades sont frères*/

            addSibling($3,$5);
       }
    |  IDENT '(' Arguments  ')' ';'
       {
           Node* node1;
            Node * node2;

          

           node1 = makeNode(appelFonction,TYPE_FUN,MY_NONE,TEXT);
             $$ = node1;
           /* Ici il faudra utiliser la table des symboles pour en profiter pour ajouter
            le type au noeud associé à la fonction lors de l'appel. 
            en regardant son nom pour retrouver son type */

            if(copieChaineNoeud(node1, $1) == 0){
                /* échec du malloc*/
                return 2;
            }

            /* On créé un noeud pour savoir qu'on aura des arguments */

            node2 = makeNode(argument,TYPE_OTHER,MY_NONE,NONE);
            
            addChild(node1, node2);

            /* le résultat de "Arguments" sera le fils du noeud 2 ("argument")*/

            addChild(node2, $3);



           
       }
    |  RETURN Exp ';'
        {
            Node * node1;
            

            node1 = makeNode(Return,TYPE_OTHER,MY_NONE,NONE);
            $$ = node1;
            /* Le résultat de l'expression sera le fils du noeud "return"*/
            addChild(node1,$2);
           
       }
    |  RETURN ';'
        {
            Node * node1;
            
            node1 = makeNode(Return,TYPE_OTHER,MY_NONE,NONE);
            $$ = node1;
       }
    |  '{' SuiteInstr '}'
        {
            
            /*node1 = makeNode(suiteInstr,TYPE_OTHER,MY_NONE,NONE);
             $$ = node1;*/
            /* le résultat de la suite d'instruction sera le fils du noeud
            "SuiteInstr"*/
            $$ = $2;
       }
    |  ';' {
           $$ = NULL;
       }
    ;
Exp :  Exp OR TB 
    {
        Node * node1;

        

        node1 = makeNode(opeLogique,TYPE_OTHER,MY_NONE,TEXT);
        $$ = node1;

        /*on copie le 'ou' sous forme de chaine de caractère */

        if(copieChaineNoeud(node1, "OR") == 0){
            /* échec du malloc*/
            return 2;
        }

        /* Le "Or" sera le père des éléments qui sont associés à l'opérateur
         "or" */
        addChild(node1,$1);

        /*les éléments associés à "Or" seront des frères*/
        addSibling($1,$3);

    }
    |  TB 
    {
        /* cas où l'expression n'a pas de "OR" : 
        on renvoie le résultat de TB*/

         $$ = $1;
    }
    ;
TB  :  TB AND FB 
    {
        Node * node1;
        

        /* "and" sera le père des éléments qu'il compare.*/

        node1 = makeNode(opeLogique,TYPE_OTHER,MY_NONE,TEXT);
        $$ = node1;
        /*on copie le 'and' sous forme de chaine de caractère */

        if(copieChaineNoeud(node1, "AND") == 0){
            /* échec du malloc*/
            return 2;
        }

        /*Les éléments à gauche et à droit du "and" seront ses fils*/
        addChild(node1,$1);
        addSibling($1,$3);
    }
    |  FB 
    {
        /* cas où l'expression n'a pas de "AND" : 
        on renvoie le résultat de FB*/

         $$ = $1;
    }
    ;
FB  :  FB EQ M
    {
        Node * node1;
        
        /*on enregistre le comparateur d'égalité dans un node1 */

        node1= makeNode(comparateur,TYPE_OTHER,MY_NONE,TEXT);
        $$ = node1;
        /* on copie sous forme de chaine caractère sa valeur*/

        if(copieChaineNoeud(node1, $2) == 0){
            /* échec du malloc*/
            return 2;
        }

        /* les éléments comparés sont les fils de 
        l'opérateur de comparaison*/

        addChild(node1, $1);

        /*ils sont donc frères*/

        addChild(node1, $3);


    }
    |  M
    {
        /* cas où l'expression n'a pas de "EQ" (égalité): 
        on renvoie le résultat de M*/

         $$ = $1;

    }
    ;
M   :  M ORDER E
    {
        Node * node1;
        
        /*on enregistre le comparateur dans un node1 */

        node1= makeNode(comparateur,TYPE_OTHER,MY_NONE,TEXT);
        $$ = node1;
        /* on copie sous forme de chaine caractère sa valeur*/

        if(copieChaineNoeud(node1, $2) == 0){
            /* échec du malloc*/
            return 2;
        }

        /* les éléments comparés sont les fils de 
        l'opérateur de comparaison*/

        addChild(node1, $1);

        /*ils sont donc frères*/
        
        addChild(node1, $3);


    }
    |  E
    {
        /* cas où l'expression n'a pas de comparateur (égalité): 
        on renvoie le résultat de E*/

         $$ = $1;

    }
    ;
E   :  E ADDSUB T 
    {
        Node * node1;
        

        /* on créé un noeud pour l'opérateur (+/-) */

        node1 = makeNode(operateur,TYPE_OTHER,MY_NONE,CAR);
        $$ = node1;
        /* on copie le symbole trouvé ("+" ou "-") dans le champ "car" */

        node1->valeur.car = $2;

        /*les éléments associés à l'opérateurs sont ses fils*/

        addChild(node1, $1);
        addChild(node1, $3);
        
    }
    |  T 
    {
        /* on créé un noeud à partir du résultat de T*/
        $$ = $1;
        
    }
    ;    
T   :  T DIVSTAR F  
    {
        
        Node * node1;
        

        /* on créé un noeud pour l'opérateur (* / % / /) */

        node1 = makeNode(operateur,TYPE_OTHER,MY_NONE,CAR);
        $$ = node1;
        /* on copie le symbole trouvé ("*" ou "%" ou "/") dans le champ "car" */

        node1->valeur.car = $2;

        /*les éléments associés à l'opérateurs sont ses fils*/

        addChild(node1, $1);
        addChild(node1, $3);
    
    }
    |  F  
    {
        /* $$ récupère le résultat de F*/
        $$ = $1;
    }
    ;
F   :  ADDSUB F
    {
        Node * node1;

        /*On créé une noeud associé à l'opérateur */

        node1 = makeNode(operateur,TYPE_OTHER,MY_NONE,CAR);
        $$ = node1;
        /* On ajoute la valeur de l'opérateur au noeud en le stockant sous forme de
        chaine de caractère */

        node1->valeur.car = $1;


        /* on donne le résultat de "F" en fils de l'opérateur stocké dans 
        "node1" */

        addChild(node1,$2);
    }
    |  '!' F

    {
        Node * node1;
        

        /* on copie le "non" (!) dans un noeud */

        node1 = makeNode(opeLogique,TYPE_OTHER,MY_NONE,CAR);
        $$ = node1;
        node1->valeur.car ='!';

        /* le résultat de F sera le fils de l'opérateur logique*/

        addChild(node1,$2);
    }

    |  '(' Exp ')'
    {
        /*on renvoie l'expression (Exp) comme résultat à $$*/

        $$ = $2;
    }
    |  NUM 
    {
        Node * node1;
        
        /*on créé un noeud associé au nombre*/
        node1 = makeNode(num,TYPE_CONST,MY_INT,ENTIER);
        $$ = node1;
        node1->valeur.entier = $1;
        
    }
    |  CHARACTER
    {
        Node * node1;
        

        /*on créé un noeud associé au caractère*/

        node1 = makeNode(character,TYPE_CONST,MY_CHAR,CAR);
        $$ = node1;
        node1->valeur.car = $1;
    }
    |  LValue
    {
        /* on récupère le résultat de LValue*/
        $$ = $1;
        
        
    }
    |  IDENT '(' Arguments  ')' 
    {
        Node * node1;
        Node * node2;
        

        /*on créé un noeud associé à l'appel de la fonction*/

        node1 = makeNode(appelFonction,TYPE_FUN,MY_NONE,TEXT);
        $$ = node1;
        /* on copie le nom de l'identificateur dans 
        le champ "text" de "node1" */

        if(copieChaineNoeud(node1, $1) == 0){
            /* échec du malloc*/
            return 2;
        }

        /* on créé un noeud associé au fait qu'on passe sur les arguments */

        node2 = makeNode(arguments,TYPE_OTHER,MY_NONE,NONE);

        /*le noeud "argument" est le fils de du noeud associé à la fonction*/

        addChild(node1,node2);

        /*les arguments sont les fils de du noeud "arguments" */

        addChild(node2,$3);     
    }
    ;
LValue
    : IDENT 
    {
        /* on créé un noeud pour stocker la variable*/
        
        Node * node1;
        
        node1 = makeNode(variable, TYPE_VAR,MY_NONE,TEXT);
        $$=node1;
        /* on enregistre le nom de la variable*/

        if(copieChaineNoeud(node1, $1) == 0){
            /* échec du malloc*/
            return 2;
        }

    }
    ;

Arguments:
       ListExp 
       
       {
        /* le résultat est le retour de ListExp*/
           $$ = $1;
       }
    | 
    {
        Node * node1;
        node1 = makeNode(argument,TYPE_OTHER,MY_NONE,TEXT);
        if(copieChaineNoeud(node1, "void") == 0){
            /* échec du malloc*/
            return 2;
        }
        $$ = node1;
        
    }
    ;




ListExp:
       ListExp ',' Exp 
       {
           addSibling($1,$3);
           $$ = $1;

       }
    |  Exp 
    {
        $$ = $1;
    }
    ;





%%

void yyerror(char * msg){
	fprintf(stderr, "\033[1;31mERROR \033[0m: %s | \033[1mLine : %d\033[1m | \033[1mCharacter : %d\033[1m\n",  msg, lineno, carno-1);
}

/*
2 arguments pour un argument qui compte le nombre d'argument et un autres
qui contient les chaines de caractères associées à ces argume ts
2 arguments pour les deux drapeaux possibles :
hflag prend la valeur "1" si il s'avère que le "help flag" est activé.
tflag prend la valeur "1" si il s'avère que l'on souhaite génère tree.
*/

void choixOptions(int argc, char ** argv, int * hflag, int * tflag, int * errflag ){
    int i;
    /* il n'y a pas d'arguments, la fonction s'arrête*/
    if(argc == 1)
        return;
    /* il y a plus de 5 arguments, ce n'est pas possible*/
    else if(argc > 5){
        *errflag = 1;
        return;
    }

    for(i=1;i<argc;i++){

        /* cas où l'option --tree ou -t est activée*/

        if(strcmp(argv[i],"--tree") == 0 || strcmp(argv[i],"-t") == 0  )
            *tflag = 1;

        /* cas ou l'option d'aide "help" est activée */

        else if(strcmp(argv[i],"--help") == 0 || strcmp(argv[i],"-h") == 0  )
            *hflag = 1;
        else{
                /* cas où l'utilisateur a choisi une option qui n'existe, on active le drapeau pour les erreurs*/
                *errflag = 1;
        }
    }
    

}


void choixOptions2(int argc, char ** argv, int * hflag, int * tflag, int * symbFlag, int * errflag ){

    /* int option_index = 0;
    ici ce n'est pas nécessaire*/

    int opt1;

    static struct option long_options[] = {
        {"help", no_argument, 0 ,'h'}, /* le raccourci pour l'option --help est -h*/
         {"tree", no_argument, 0 ,'t'}, /* le raccourcit pour l'option --tree est -t */
         {"symtabs", no_argument, 0 ,'s'}, /* le raccourcit pour l'option --symtabs est -s */

         /*indique la fin des arguments*/
         {0,0,0,0}
    };

    while((opt1 = getopt_long(argc,argv,"hts",long_options, NULL)) != -1){
      /* on vérifie chaque option utilisée*/
         if(opt1 == 'h'){
        /* on a activé l'option help*/
             *hflag = 1;
        }  
        else if(opt1 == 't'){
        /* on a activé l'option tree */
             *tflag = 1;
        }

        else if(opt1 == 's'){
        /* on a activé l'option affichage tab des symboles */
             *symbFlag= 1;
        }
        /* on est tombé sur une option qui n'existe pas */
        else{
           *errflag = 1;
        }     
    }
    

}



void manuelUser(void){
    printf("Bonjour et bienvenue dans le manuel utilisateur ! \n");
    printf(" Les différentes options accessibles sont --tree (ou -t)  pour afficher l'arbre abstrait et --help (ou -h) pour afficher l'aide. \n\n");
    printf("Il faut ainsi écrire ''./bin/tpcas --tree < chemin/vers/lefichier/a/traiter.txt''  pour traiter le fichier traiter.txt qui se trouve dans le dossier ''chemin/vers/lefichier/a/'' \n\n\n\n");
    printf("Si vous écrivez ''./bin/tpcas --help'' alors le manuel utilisateur sera affiché. \n\n");

}

int main(int argc, char * argv[]){
    int hflag, errflag;
    int retour;

    /* on utilise cette fonction pour savoir les options choisies par le user*/

    choixOptions2(argc,argv,&hflag, &tflag, &symbFlag,&errflag);

    if(hflag == 1 && errflag != 1){

        manuelUser();
    }

    if(errflag == 1){
        printf(" Vous avez saisi une mauvaise commande. \n\n");
        printf(" Je vais donc vous rappeler comment fonctionne ce projet : n\n");
        manuelUser();
        /* valeur de retour pour les erreurs de type ligne de commande*/
        return 3;
    }

    /*on lance l'analyse avec la création de l'arbre si le tree flag est activé */

    else{
        /* on lance l'analyse syntaxique*/
        
        if((retour = yyparse()) == 1){ 
            
            return 1; // il y une erreur syntaxique ou lexicale dans le fichier
        }
        /* cas d'erreur sémantique */

        else if(retour == 2){
            return 2;
        }

        /* problème d'allocation*/

        /*else if(yyparse() == 2){
            return 2;
        }*/
    }


    

   

    return 0;	/*Aucune erreur dans le fichier*/
}








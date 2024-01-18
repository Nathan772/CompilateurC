#!/bin/bash

#on créé une fonction qui va analyser chaque fichiers test dans un dossier
#l'argument sera le chemin vers le dossier
function analyseFichiersTest(){
    local dossier=$1
    # pour chaque fichier test "file" "dans le dossier $1 (argument 1)
    #for file in "./$dossier/";
    #for file in ./test/good/*
    #for file in ./$dossier/*
    #readarray -d '' entries < <(printf '%s\0' ./$dossier/* | sort -V) 

    #on créé un tableau "entries" qui contient chaque path vers chaque fichier du dossier "$dossier"
    readarray -d '' entries < <(printf '%s\0' ./$dossier/* )
    #local entriesSorted = ($(for path in ${entries[@]}; do echo $path; done | sort))
    #on trie le contenu de "entries" dans un nouveau tableau
    #le nouveau tableau avec les éléments triés
    #attention, il ne faut pas mettre d'espace entre le "=" et la parenthèse, sinon le tableau ne sera pas reconnu.
    #On met l'option "-V" pour trier dans l'ordre numérique.
    entriesSorted=( 
        
        $(for path in "${entries[@]}";
        do  
            echo "$path";
        done | sort -V) 
    )


    for file in  "${entriesSorted[@]}";
    do

        printf "Fichier traité : $file\n\n" >> ./test/resultat.txt # on écrit le nom du fichier traité dans le fichier de résultat.
        #./bin/tpcas < $file  #on lance le programme sur chaque fichier et sans cacher la sortie obtenue après ce traitement
        cat  $file | ./bin/tpcc 2> /dev/null #on lance le programme sur chaque fichier et on cache la sortie obtenue après ce traitement
        
        local res=$? # on récupère la valeur de retour associé au fichier traité.
        if [ "$res" -eq 0 ] # si tout s'est bien passé. "eq" permet de symboliser le "=" en bash
         #attention les espaces entre les opérateurs de l'expression condtionnelle et les crochets sont indispensables.
        then
            printf "Test : fichier valide \n\n" >> ./test/resultat.txt
        elif [ "$res" -eq 1 ] #erreur 1 : erreur syntaxique.
        then
                printf "Test : erreur sytaxique\n\n" >> ./test/resultat.txt

        elif [ "$res" -eq 2 ] #erreur 2 : erreur sémantique.
        then
                printf "Test : erreur sémantique \n\n" >> ./test/resultat.txt
        
        else # erreur 3 : erreur de commande ou de malloc.
            printf "Test : autres type d'erreur (malloc, mauvaise entrée...) \n\n" >> ./test/resultat.txt
        fi


    done
}

#on créé le fichier résultat

function creationResultat(){
    #on supprime le fichier resultat si il existe déjà.
    if test -f "./test/resultat.txt"; then
        rm ./test/resultat.txt
    fi

    #on créé un fichier pour les résultats
    touch ./test/resultat.txt       
}
 
creationResultat ;
#on écrit la présentation du fichier
printf "Résultat de l'analyse syntaxique de fichiers \n\n\n" >> ./test/resultat.txt

printf "Fichiers essais du dossier 'good' :  \n\n" >> ./test/resultat.txt

#on analyse les fichiers "good"
analyseFichiersTest "test/good";

printf "\n\n\n ---------------- \n\n\n" >> ./test/resultat.txt

printf "Fichiers essais du dossier 'syn-err' :  \n\n" >> ./test/resultat.txt

#on analyse les fichiers "syn-err"

analyseFichiersTest "test/syn-err";

printf "\n\n\n ---------------- \n\n\n" >> ./test/resultat.txt

printf "Fichiers essais du dossier 'sem-err' :  \n\n" >> ./test/resultat.txt

#on analyse les fichiers "sem-err"

analyseFichiersTest "test/sem-err";

printf "\n\n\n ---------------- \n\n\n" >> ./test/resultat.txt

printf "Fichiers essais du dossier 'warn' :  \n\n" >> ./test/resultat.txt

#on analyse les fichiers "warn"

analyseFichiersTest "test/warn";


extern show_registers 

global getint

getint:
    sub rsp, 8 ; on réserve de l'espace sur la pile pour pouvoir
    ; écrire la valeur de l'utilisateur
    mov rax, 0 ; lire l'entrée de l'utilisateur
    mov rdi, 0 ; indique que l'on va lire l'entrée standard
    mov rsi, rsp ; on récupère l'adresse associé à 
    ;l'endroit où l'on va écrire
    mov rdx, 8 ; la taille de la donnée à lire est de 8 octet, taille
    ;maximum pour un nombre 
    syscall ; on appelle la fonction pour écrire
    ;on converti ensuite la chaine de caractère en un entier
    mov rax, 0 ; rax va contenir le résultat
    mov rcx, 0;variable qui va indiquer de combien on avance
    ; pour traiter le caractère (digit) suivant

    test_caractere: ; cette première partie test si
    ;le premier caractère donné est une lettre plutôt qu'un nombre
    ; si c'est le cas, on tombe sur une erreur
        cmp byte [rsi+rcx], 31h ; on compare la valeur
        jl err ; la valeur est trop petite pour être un nombre (31h ==
        ; 1 en ascii)
        cmp byte[rsi+rcx],39h; 39h == 9 en ascii
        jg err ; ,la valeur est trop grande pour être un nombre
        ;sub byte [rsi], 48 ; 

    boucle_conversion:
        cmp byte [rsi+rcx], 30h ; on compare la valeur
        jl end ; la valeur est trop petite pour être un nombre (31h ==
        ; 0 en ascii)
        cmp byte [rsi+rcx],39h; 39h == 9 en ascii
        jg end ; ,la valeur est trop grande pour être un nombre
        sub byte [rsi+rcx], 48 ; caclul qui permet de convertir un 
        ; caractère en nombre
        imul rax, 10 ;on multiplie le contenu de rax par 10 
        ; pour faire le décalage d'un rang pour les précédents
        ; entiers qui avaient été ajouté au résultat final
        mov dil, byte [rsi+rcx] ;on copie
        ; le byte à copier
        add rax,rdi
        add rcx,1 ; on avance de un caractère pour la suite
        jmp boucle_conversion

    end:
        pop rcx ; on remet la pile à l'@ de retour du main
        ret 
    err:
        mov rax, 5 ; code d'erreur
        pop rcx ; on remet la pile à l'@ de retour du main
        ret 

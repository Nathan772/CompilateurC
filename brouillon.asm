my_putint:
    push rbp ; on enregistre l'emplacement du précédent bloc d'activation
    mov rbp, rsp ; on enregistre le début de la fonction dans rbp
    ;mov r8,rsp ; on, enregistre l'adresse de la tête de pile dans r8
    ; push 0 ; on initialise un emplacement sur la pile pour imax
    push rdi ; on met le nombre que l'on veut afficher sur la pile à l'emplacement "rbp-24" pour le conserver en mémoire
    mov r9, 0 ; on initialise une variable pour number
    mov r8, [imax] ; on initialise i à imax

    while: ; on récupère dans cette boucle les différentes 
    ;parties du nombre que l'on converti en caractères
        
        mov rax, 10 ; on va diviser notre entier par 10
        idiv rdi ; division de rdi (number) par rax (10)
        mov rbx, rdi ; on met le reste dans rbx
        mov rdi, rax ; on copie le quotient dans rdi
        add rbx, 48 ; on ajoute 48 au reste
        mov byte [digits+r8], bl ; on copie le résultat dans ; digits+i (i == r8)
        sub r8, 1; on décrémente i de 1
        cmp rdi, 0 ; on vérifie que number est supérieur à 0
        jg while
        

        mov r9, [imax] ; on prépare la valeur de imax dans r9 pour la comparaison qui sera nécessaire à la fin de la boucle "ecriture"
        mov r8,0  ; on met i à 0 pour la partie écriture

    ecriture:
        ;partie écriture
        add r8, 1 ; on incrémente de 1 pour passer au digit suivant
        mov rax,1 ; correspond au fait que l'on souhaite écrire
        mov rdi, [digits+r8] ; on donne à rdi le digit à écrire
        push rdi ; on met l'argument sur la pile pour syscall
        mov rdi, 1 ; on indique que l'on va écrire sur la sortie standard (1)
        mov rdx, 1 ; taille de l'élémennt que l'on souhaite ; afficher ici un caractère donc un octet
        ;mov rsi, digits ; on met dans rsi l'adresse de l'élément que l'on souhaite
        ; afficher, ce dernier était contenu dans rdi (premier argument)
        syscall
        cmp r8,r9
        jl ecriture
        
    end:
        pop rdi ; on enlève l'argument qui avait été donné à syscall
        pop rdi ; on redonne à l'emplacement initial la valeur donnée en argument
        pop rbp ; on remet le bloc d'activation à sa place
        ret

---

à remettre

while: ; on récupère dans cette boucle les différentes 
        ;parties du nombre que l'on converti en caractères
            
            mov rax, 10 ; on va diviser notre entier par 10
            idiv rdi ; division de rdi (number) par rax (10)
            mov rbx, rdi ; on met le reste dans rbx
            mov rdi, rax ; on copie le quotient dans rdi
            add rbx, 48 ; on ajoute 48 au reste
            mov byte [digits+r8], bl ; on copie le résultat dans ; digits+i (i == r8)
            sub r8, 1; on décrémente i de 1
            cmp rdi, 0 ; on vérifie que number est supérieur à 0
            jg while

----

meme chose :

 while: ; on récupère dans cette boucle les différentes 
        ;parties du nombre que l'on converti en caractères
            
            mov rax, 10 ; on va diviser notre entier par 10
            idiv rdi ; division de rdi (number) par rax (10)
            mov rbx, rdi ; on met le reste dans rbx
            mov rdi, rax ; on copie le quotient dans rdi
            add rbx, 48 ; on ajoute 48 au reste
            mov byte [digits+r8], bl ; on copie le résultat dans ; digits+i (i == r8)
            sub r8, 1; on décrémente i de 1
            cmp rdi, 0 ; on vérifie que number est supérieur à 0
            jg while



ecriture:
            ;partie écriture
            add r8, 1 ; on incrémente de 1 pour passer au digit suivant
            mov rax,1 ; correspond au fait que l'on souhaite écrire
            mov dil, [digits+r8] ; on donne à rdi (dil) le digit à écrire
            ;add rdi, 0x30 ; on converti le chiffre en caractère
            push rdi ; on met l'argument sur la pile pour syscall
            mov rdi, 1 ; on indique que l'on va écrire sur la sortie standard (1)
            mov rdx, 1 ; taille de l'élémennt que l'on souhaite ; afficher ici un caractère donc un octet
            mov rsi, rsp ; on met dans rsi l'adresse de l'élément que l'on souhaite écrire
            syscall
            pop rdi ; on enlève le précédent nombre qu'on avait empilé pour replacer la pile corrctement
            cmp r8,r9
            jl ecriture




mov r12,0
mov r12, [rbp-16] ; on met la valeur de x dans r12
call show_registers
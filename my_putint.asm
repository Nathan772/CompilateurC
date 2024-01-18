
section .data
digits: db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0; on aura au plus 20 chiffres dans notre nombre ils seront stockés sous la forme de "char"
; au maximum notre nombre sera de taille ~2^64
; ce qui implique que la puissance de 10 supérieur la plus 
; proche est 10^20 donc 10^20 implique qu'il faut ajouter 20 pour imax (i)
imax2: dq 20
imax: dq 20 
diviseur: dq 10


section .text 
global putint
; global _start
; extern show_registers 

    putint:
   
        push rbp ; on enregistre l'emplacement du précédent bloc d'activation
        mov rbp, rsp ; on enregistre le début de la fonction dans rbp
        mov rdi, [rbp+24] ; on récupère l'argument 
        ;mov r8,rsp ; on, enregistre l'adresse de la tête de pile dans r8
        push rdi ; on met le nombre que l'on veut afficher sur la pile à l'emplacement "rbp-16" pour le conserver en mémoire
        push rbx ; on empile la valeur de rbx à [rbp-24] pour ne pas la perdre car on va se servir de ce registre
        mov rax, rdi ; on initialise une variable pour number (comme le diviseur
        ; est tjrs rax, on mov [number] (rdi) dans rax) 
        mov r8, [imax] ; on initialise i à imax

        while: 

        ; on récupère dans cette boucle les différentes 
        ;parties du nombre que l'on converti en caractères
            mov rdx, 0 ;il faut mettre rdx à 0 avant de vouloir faire une division sinon ses valeurs résiduelles
            ; vont poser problème et créer des valeurs de divisions incohérentes
            mov rbx, [diviseur] ; on va diviser notre entier par 10
            idiv rbx ; division de rax (number) par rbx (10)
            mov rsi, 0
            mov rsi, rdx ; on met le reste dans rsi (le reste est tjrs dans rdx)
            add rsi, 48 ; on ajoute 48 au reste pour transformer le chiffre en caractère
            mov byte [digits+r8], sil ; on copie le résultat dans ; digits+i (i == r8)
            sub r8, 1; on décrémente i de 1
            mov rbx,[diviseur]
            cmp rax, 0 ; on vérifie que number est supérieur à 0
            jg while

        ; sub r8,1
        mov r9, [imax] ; on prépare la valeur de imax dans r9 pour la comparaison qui sera nécessaire à la fin de la boucle "ecriture"
        

        ecriture:
            ;partie écriture
            add r8, 1 ; on incrémente de 1 pour passer au digit suivant
            mov rax,1 ; correspond au fait que l'on souhaite écrire
            mov rdi, 0 ; on met à 0 pour éviter des problèmes de valeurs résiduelles
            mov dil, byte [digits+r8] ; on donne à rdi (dil) le digit à écrire
            push rdi ; on met l'argument sur la pile pour syscal
            mov rdi, 1 ; on indique que l'on va écrire sur la sortie standard (1)
            mov rdx, 1 ; taille de l'élémennt que l'on souhaite ; afficher ici un caractère donc un octet
            mov rsi, rsp ; on met dans rsi l'adresse de l'élément que l'on souhaite écrire
            syscall
            pop rdi ; on enlève le précédent nombre qu'on avait empilé pour replacer la pile corrctement
            cmp r8,[imax]
            jl ecriture


        end:
            pop rbx ; on redonne à rbx la valeur qu'il avait au début
            pop rdi ; on redonne à rdi la valeur de l'argument
            pop rbp ; on redonne à rbp l'@ de l'ancien bloc d'activation
            ret

;_start:
    ;mov rdi, 123; on ajoute un nombre en argument, ici '3256'
    ;call my_putint
    ; sortie de programme
    ;mov rax,60
    ;mov rdi,0

    ;syscall
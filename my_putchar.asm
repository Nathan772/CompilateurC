 
; extern show_registers 

global putchar 

;global _start

putchar:
        mov rax,1 ; correspond au fait que l'on souhaite écrire
        push qword [rsp+16] ; on met l'argument sur la pile pour syscall
        mov rdi, 1 ; on indique que l'on va écrire sur la sortie standard (1)
        mov rdx, 1 ; taille de l'élément que l'on souhaite
        ; afficher ici un caractère donc un octet
        mov rsi, rsp ; on met dans rsi l'adresse de l'élément que l'on souhaite
        ; afficher, ce dernier était contenu dans rdi (premier argument)
        syscall
        pop rdi ; on réaligne la pile à l'adresse du main de la fonction principale
        ret

;_start:
;   mov rdi, 'A'; on ajoute un caractère aux arguments, ici 'A'
;    call my_putchar

    ; sortie de programme
 ;   mov rax,60
 ;   mov rdi,0

  ;  syscall
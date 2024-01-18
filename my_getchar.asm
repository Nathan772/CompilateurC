;extern show_registers 

global getchar

; global _start

getchar:
    sub rsp, 8 ; on réserve de l'espace sur la pile pour pouvoir
    ; écrire la valeur de l'utilisateur
    mov rax, 0 ; lire l'entrée de l'utilisateur
    mov rdi, 0 ; indique que l'on va lire l'entrée standard
    mov rsi, rsp ; on récupère l'adresse associé à 
    ;l'endroit où l'on va écrire le résultat
    mov rdx, 1 ; la taille de la donnée à lire est de 1 octets, taille
    ;maximum pour un nombre 
    syscall ; on appel la fonction pour écrire
    pop rax ; on réaligne la pile et on enregistre la valeur qui
    ; était dans la pile
    ; dans rax qui, d'après les conventions, contient la valeur
    ;de retour
    ret


;_start:
    ; call my_getchar
    ;mov rbx, rax
    ;call show_registers
    ; sortie de programme
    ;mov rax,60
    ;mov rdi,0

   ; syscall
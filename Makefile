# Makefile TP Flex/Bison

# $@ : the current target
# $^ : the current prerequisites
# $< : the first current prerequisite
#remarque hyper important : il n'est pas possible d'écrire en dur un path dans un makefile
#il est obligatoire d'utiliser des raccourcis avec $(lechemin) e
CC=gcc
CFLAGS=-Wall -g
LDFLAGS = -Wall
DDFLAGS = -Wall -ansi -pedantic
EXEC=tpcc
EXEC2= anonymous
TPC = tpc-2022-2023
OBJDIR = ./obj
BINDIR = ./bin
SRCDIR = ./src
NASM = nasm -f elf64
NASMFLAGS = -nostartfiles -no-pie

#bin contient l'exécutable final.
#src contient les fichiers sources c'est à dire créé par des humains
#obj contient les fichiers intermédiaires générés par la machine

#créé l'exécutable (td3-ex1) et les fichiers intermédiaires tel que lex.yy.o, td3-ex1.tab.c 

#all crée les fichiers exécutables : ici on créé seulement  "$(BINDIR)/$(EXEC)"
all: $(BINDIR)/$(EXEC)   $(OBJDIR)/my_getint.o $(OBJDIR)/my_getchar.o $(OBJDIR)/my_putchar.o $(OBJDIR)/my_putint.o #$(BINDIR)/$(EXEC2)


$(BINDIR)/$(EXEC2): $(OBJDIR)/$(EXEC2).o $(OBJDIR)/utils.o $(OBJDIR)/my_getint.o $(OBJDIR)/my_getchar.o $(OBJDIR)/my_putchar.o $(OBJDIR)/my_putint.o
	$(CC) -o $@ $^ $(NASMFLAGS) 

$(BINDIR)/$(EXEC): $(OBJDIR)/lex.yy.c $(OBJDIR)/$(TPC).tab.o  $(OBJDIR)/tree.o $(OBJDIR)/analyseTree.o $(OBJDIR)/generationAssembleur.o
	$(CC) -o $(BINDIR)/$(EXEC) $^ $(CFLAGS)

# $(BINDIR)/my_putint: $(OBJDIR)/my_putint.o $(OBJDIR)/utils.o
#	$(CC) -o $(BINDIR)/my_putint $^ $(NASMFLAGS)

#$(BINDIR)/analyseTree : $(OBJ_DIR)/analyseTree.o


#obj contient les fichiers intermédiaires.
#on créé le fichier lex.yy.c grâce au fichier "td3-ex1.lex".

#La partie "-o $(OBJDIR)/lex.yy.c" permet de nommer le résultat de la compilation du fichier "./$(SRCDIR)/$(EXEC).lex" en " $(OBJDIR)/lex.yy.c" " 
# avant d'être créé, le fichier lex.yy.c dépend de "./$(OBJDIR)/$(EXEC).tab.h" et "./$(SRCDIR)/$(EXEC).lex".
# ce qui donne "$(OBJDIR)/lex.yy.c."

$(OBJDIR)/lex.yy.c: ./$(SRCDIR)/$(EXEC).lex ./$(OBJDIR)/$(TPC).tab.h
	flex -o $(OBJDIR)/lex.yy.c ./$(SRCDIR)/$(EXEC).lex


$(OBJDIR)/%.o: /$(OBJDIR)/$(TPC).tab.c $(OBJ_DIR)/generationAssembleur.o ./$(OBJDIR)/$(TPC).tab.h $(OBJDIR)/tree.o $(OBJDIR)/analyseTree.o
	$(CC) -o $@ -c $< $(CFLAGS)
	
# créé un lien entre le .o et le .c grâce au point .h \

$(OBJ_DIR)/analyseTree.o : $(SRCDIR)/analyseTree.h 

$(OBJ_DIR)/generationAssembleur.o : $(SRCDIR)/generationAssembleur.h


$(OBJDIR)/my_putchar.o : $(SRCDIR)/my_putchar.asm
		$(NASM) -o $@ $<

$(OBJDIR)/anonymous.o : $(BINDIR)/anonymous.asm
		$(NASM) -o $@ $<

$(OBJDIR)/my_putint.o : $(SRCDIR)/my_putint.asm
		$(NASM) -o $@ $<

$(OBJDIR)/my_getint.o : $(SRCDIR)/my_getint.asm
		$(NASM) -o $@ $<

$(OBJDIR)/my_getchar.o : $(SRCDIR)/my_getchar.asm
		$(NASM) -o $@ $<

$(OBJDIR)/utils.o: $(SRCDIR)/utils.asm
	$(NASM) -o $@ $< 

#"$@" signifie le premier élément à gauche des ":" 
# le"$<" signifie le premier élément après les ":"
$(OBJDIR)/analyseTree.o : $(SRCDIR)/analyseTree.c
	$(CC) -o $@ -c $< $(DDFLAGS)

$(OBJDIR)/generationAssembleur.o : $(SRCDIR)/generationAssembleur.c
	$(CC) -o $@ -c $< $(DDFLAGS)

# créé un lien entre le .o et le .c grâce au point .h \

$(OBJDIR)/tree.o: $(SRCDIR)/tree.c $(SRCDIR)/tree.h 
	$(CC) -o $@ -c $< $(CFLAGS)


$(OBJDIR)/$(TPC).tab.c ./$(OBJDIR)/$(TPC).tab.h : ./$(SRCDIR)/$(TPC).y ./$(SRCDIR)/tree.h 
	bison -d -o $(OBJDIR)/$(TPC).tab.c $(SRCDIR)/$(TPC).y

# $(OBJDIR)/anonymous.o : $(BINDIR)/anonymous.asm 
#	$(NASM) -o $@ $<


	
clean_directories:
	make clean
	rm -f $(OBJDIR)/*  $(BINDIR)/*

clean:
	rm -f  $(OBJDIR)/* $(BINDIR)/*

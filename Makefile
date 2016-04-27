all:
	gcc -o Projet_OS -Wall ProjetOS.c -lpthread
run:
	./Projet_OS max fichier1.txt
clean:
	rm ./Projet_OS

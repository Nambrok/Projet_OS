#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_SIZE_BUF 256

struct byThread{
	float * chiffre;
	int taille;
	int deb;
	int fin;
	char * mode;
	//~ pthread_mutex_t * mutex;
};

//~ struct byThread * creationByThread(float * chiffre, int deb, int fin, char * mode, pthread_mutex_t * mutex){
struct byThread * creationByThread(float * chiffre, int deb, int fin, int taille, char * mode){
	//Initialise la structure de données byThread qui sert au processus chefs d'équipes à passer les données que doivent traiter les threads aux différents threads qu'il engendre.
	assert(deb>=0);
	assert(fin>=0);
	struct byThread * thInfo = malloc(sizeof(struct byThread));
	thInfo->deb = deb;
	thInfo->fin = fin;
	thInfo->chiffre = chiffre;
	thInfo->mode = mode;
	thInfo->taille = taille; //taille de chiffre[]
	return thInfo;
}

void chefEquipeMain(char * nomFichier, char* mode);
void* mainThread(void* a);

int main(int argc, char ** argv){
	if(argc>=3){
		//~ fprintf(stdout, "こんにちは世界!\n");
		char nomFichiers[MAX_SIZE_BUF][MAX_SIZE_BUF]; int i, j, nombreFichiers = 0;//nomFichiers contient les noms des fichiers d'entrée qu'il faudra faire passer aux processus chefs d'équipe.
		//nombreFichiers est la taille max de la première dimension de nomFichiers, c'à-d le nombre de nom de fichier et le nombre de fichier qu'on a.
		char mode[MAX_SIZE_BUF];//mode est le mode, c'à-d "max", "min", "avg" etc...
		
		i = 0, j= 2;
		while(i<MAX_SIZE_BUF && j<argc){
			strcpy(nomFichiers[i], argv[j]);
			nombreFichiers++;
			i++; j++;
		}
		strcpy(mode, argv[1]); //On récupère le mode d'utilisation entrée dans mode pour utiliser plus tard.
				
		pid_t pid; char fichier[MAX_SIZE_BUF];
		strcpy(fichier, "fichier1.txt");
		for(i = 0; i<nombreFichiers; i++){
			pid = fork();
			//TODO: Il faudrait choisir ici quels fichiers sera passer au processus fils(ou processus chefs d'équipes.
			if(pid){
				//pere
				wait(NULL);				
			}
			else{
				//fils
				chefEquipeMain(fichier, mode);
				exit(0);
			}
		}
		
		//PERE
		//Le père doit procréer autant qu'il y a de fichiers d'entrées.		
		//FILS
		//doit assigner les valeurs de sont fichiers aux différents threads et les créer.
		
		//~ pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
		//~ float chiffre[N];
		//~ chiffre[0] = 0.0;
		//~ for(i = 1; i<N; i++){
			//~ chiffre[i] = chiffre[i-1]+1.7;
		//~ }
		//~ struct byThread * thInfo = creationByThread(chiffre, 2, 3, mode, &mutex); //Façon d'initialiser un struct byThread. C'est la structure qu'il faudra envoyer aux threads. Elles seront initialisé par les processus chefs d'équipe.
		
		//Affichage des noms de fichier en entrée (affichage test).
		for(i = 0; i<nombreFichiers; i++){
			fprintf(stdout, "%s ", nomFichiers[i]);
		}
		fprintf(stdout, "\n");
		
		
	}
	else{
		fprintf(stderr, "Manque d'arguments, il faut au moins 2 arguments. [option] [nom du fichier]\nExemples d'options : \n- min\n- max\n- avg\n- sum\n- odd\n");
	}
	return 0;
}

//Il faudrait créer des fonctions "main" pour les threads et les processus chefs d'équipe
void chefEquipeMain(char * nomFichier, char* mode){
	char buf[MAX_SIZE_BUF]; char entreeTraiter[MAX_SIZE_BUF]; int i;
	float valeurs[100000]; int nombreValeurs = 0; int j, k;
	int FS = open(nomFichier, O_RDONLY);
	assert(FS!=-1);
	if(FS == -1){
		fprintf(stderr, "Erreur dans l'ouverture du fichier %s\n", nomFichier);
	}
	else{
		do{
			read(FS, buf, MAX_SIZE_BUF); j = 0; k = 0;
			for(i = 0; i<MAX_SIZE_BUF; i++){
				if(buf[i] != '\n'){
					entreeTraiter[k] = buf[i];
					k++;
				}
				else{
					valeurs[nombreValeurs] = atof(entreeTraiter);
					k = 0; nombreValeurs++;
				}
			}//On extraie une partie des valeurs du ficher nomFichier dans le tableau valeurs. valeurs[0] est le nombre de valeurs dans le fichier.
		}while(nombreValeurs<valeurs[0]);
		
		
		fprintf(stdout, "nombreValeurs : %d\n", nombreValeurs);
		for(i = 1; i<nombreValeurs; i++){
			fprintf(stdout, "%d : %f\n", i-1, valeurs[i]);
		}
		
		
		struct byThread * thInfo = creationByThread(valeurs, 0, nombreValeurs, nombreValeurs, mode);
		int nombreThreadTotal = nombreValeurs / 100; int nombreThreadCreer = 0;
		pthread_t tid; 
		for(nombreThreadCreer = 0; nombreThreadCreer<nombreThreadTotal; nombreThreadCreer++){
			pthread_create( &tid, NULL, &mainThread, &(thInfo));
			pthread_join(tid, NULL);
		}
		
		
	}
}

void* mainThread(void* a){
	struct byThread* id = a; int i;
	assert(id->taille>0);//C'est assert se déclenche de temps en temps sans que je sache d'ou ça puisse venir.
	assert(id->chiffre != NULL);
	printf("DEBUT\n");
	for(i = id->deb; i < id->fin; i++){
		printf("%f ", id->chiffre[i]);
	}
	printf("\nFIN\n");
	pthread_exit(EXIT_SUCCESS);
	
}

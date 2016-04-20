#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define N 256

struct byThread{
	float * chiffre;
	int taille;
	int deb;
	int fin;
	pthread_mutex_t * mutex;
};

struct byThread * creationByThread(float * chiffre, int deb, int fin, pthread_mutex_t * mutex){
	struct byThread * thInfo = malloc(sizeof(struct byThread));
	thInfo->deb = deb;
	thInfo->fin = fin;
	thInfo->chiffre = chiffre;
	thInfo->mutex = mutex;
	return thInfo;
}

int main(int argc, char ** argv){
	if(argc>=3){
		//~ fprintf(stdout, "こんにちは世界!\n");
		char nomFichiers[N][N]; int i, j, nombreFichiers = 0;
		char mode[N];
		
		i = 0, j= 2;
		while(i<N && j<argc){
			strcpy(nomFichiers[i], argv[j]);
			nombreFichiers++;
			i++; j++;
		}
		strcpy(mode, argv[1]); //On récupère le mode d'utilisation entrée dans mode pour utiliser plus tard.
		
		
		/*pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
		float chiffre[N];
		chiffre[0] = 0.0;
		for(i = 1; i<N; i++){
			chiffre[i] = chiffre[i-1]+1.7;
		}
		struct byThread * thInfo = creationByThread(chiffre, 2, 3, &mutex);*///Façon d'initialiser un struct byThread. C'est la structure qu'il faudra envoyer aux threads. Elles seront initialisé par les processus chefs d'équipe.
		
		//Affichage des noms de fichier en entrée (affichage test).
		for(i = 0; i<nombreFichiers; i++){
			fprintf(stdout, "%s ", nomFichiers[i]);
		}
		
		
	}
	else{
		fprintf(stderr, "Manque d'arguments, il faut au moins 2 arguments. [option] [nom du fichier]\nExemples d'options : \n- min\n- max\n- avg\n- sum\n- odd\n");
	}
	return 0;
}

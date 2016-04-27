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
typedef struct byThread BYTHREAD;
//~ struct byThread * creationByThread(float * chiffre, int deb, int fin, char * mode, pthread_mutex_t * mutex){
BYTHREAD* creationByThread(float * chiffre, int deb, int fin, int taille, char * mode){
	//Initialise la structure de données byThread qui sert au processus chefs d'équipes à passer les données que doivent traiter les threads aux différents threads qu'il engendre.
	assert(deb>=0);
	assert(fin>=0);
	BYTHREAD * thInfo = malloc(sizeof(BYTHREAD));
	thInfo->deb = deb;
	thInfo->fin = fin;
	thInfo->chiffre = chiffre;
	thInfo->mode = mode;
	thInfo->taille = taille; //taille de chiffre[]
	return thInfo;
}

void afficherByThread(BYTHREAD thInfo){
	//~ int i;
	//~ for(i = 0; i<thInfo.taille; i++){
		//~ printf("%f ", thInfo.chiffre[i]);
	//~ }
	//~ printf("\n");
	printf("deb : %d, fin : %d, mode : %s, taille de chiffre : %d\n", thInfo.deb, thInfo.fin, thInfo.mode, thInfo.taille);
}

float chefEquipeMain(char * nomFichier, char* mode);
void* mainThread(void* a);

float max(float in[], int tailleTab);
float min(float in[], int tailleTab);
float avg(float in[], int tailleTab);
float sum(float in[], int tailleTab);
float odd(float in[], int tailleTab);
int oddMiseEnCommun(float in[], int tailleTab);
int comparerChaines(const char* chaine1, const char* chaine2);

int main(int argc, char ** argv){
	if(argc>=3){
		char nomFichiers[MAX_SIZE_BUF][MAX_SIZE_BUF]; int i, j, nombreFichiers = 0;//nomFichiers contient les noms des fichiers d'entrée qu'il faudra faire passer aux processus chefs d'équipe.
		//nombreFichiers est la taille max de la première dimension de nomFichiers, c'à-d le nombre de nom de fichier et le nombre de fichier qu'on a.
		char mode[MAX_SIZE_BUF];//mode est le mode, c'à-d "max", "min", "avg" etc...
		pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
		
		i = 0, j= 2;
		while(i<MAX_SIZE_BUF && j<argc){
			strcpy(nomFichiers[i], argv[j]);
			nombreFichiers++;
			i++; j++;
		}
		strcpy(mode, argv[1]); //On récupère le mode d'utilisation entrée dans mode pour utiliser plus tard.
				
		pid_t pid; char fichier[MAX_SIZE_BUF]; int fd[2];
		char buffer[MAX_SIZE_BUF]; float res[MAX_SIZE_BUF]; j= 0;
		for(i = 0; i<nombreFichiers; i++){
			strcpy(fichier, nomFichiers[i]);
			pipe(fd);
			pid = fork();
			if(pid){
				//pere
				close(fd[1]);
				waitpid(pid, NULL, 0);
				
				pthread_mutex_lock(&mutex);
				
				read(fd[0], buffer, MAX_SIZE_BUF);
				res[j] = atof(buffer); j++;
				
				pthread_mutex_unlock(&mutex);
				close(fd[0]);
			}
			else{
				//fils
				close(fd[0]);
				char str[MAX_SIZE_BUF];
				sprintf(str, "%f", chefEquipeMain(fichier, mode)); //Permet de traduire le float en chaine de caractères pour l'envoyer à travers le pipe.
				write(fd[1], str, MAX_SIZE_BUF);
				close(fd[1]);
				exit(EXIT_SUCCESS);
			}
		}

		float miseEnCommunRes;
		if(comparerChaines(mode, "max")){
			miseEnCommunRes = max(res, j);
		}
		else if(comparerChaines(mode, "min")){
			miseEnCommunRes = min(res, j);
		}
		else if(comparerChaines(mode, "avg")){
			miseEnCommunRes = avg(res, j);
		}
		else if(comparerChaines(mode, "sum")){
			miseEnCommunRes = sum(res, j);
		}
		else if(comparerChaines(mode, "odd")){
			miseEnCommunRes = oddMiseEnCommun(res, j);
		}
		else{
			fprintf(stderr, "Erreur dans le mode entrée\n");
			exit(EXIT_FAILURE);
		}
		printf("Le %s dans les fichiers données en entrées est %f.\n", mode, miseEnCommunRes);
		
		fprintf(stdout, "Fichiers d'entrées : ");
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

float chefEquipeMain(char * nomFichier, char* mode){
	char buf[MAX_SIZE_BUF]; char entreeTraiter[MAX_SIZE_BUF]; int i;
	float valeurs[100000]; int nombreValeurs = 0; int k;
	int FS = open(nomFichier, O_RDONLY);
	if(FS == -1){
		fprintf(stderr, "Erreur dans l'ouverture du fichier %s\n", nomFichier);
		exit(EXIT_FAILURE);
	}
	else{
		do{
			read(FS, buf, MAX_SIZE_BUF); k = 0;
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
		close(FS);
		
		int nombreThreadTotal = (nombreValeurs / 100)+1; int nombreThreadCreer = 0;
		float * res;
		pthread_t tid;
		float tabRes[MAX_SIZE_BUF] = {0.0};
		BYTHREAD * thInfo[MAX_SIZE_BUF]; int deb = 1, fin = 101; int taille = 100;//J'initialise la structure de données à envoyées pour chaque thread.
		
		if(nombreValeurs < 100){
			thInfo[0] = creationByThread(valeurs, deb, nombreValeurs, nombreValeurs-1, mode);
		}
		else{
			for(i = 0; i<nombreThreadTotal; i++){
				thInfo[i] = creationByThread(valeurs, deb, fin, taille, mode);
				
				deb+= 100; fin+= 100;
				if(fin > nombreValeurs){
					fin =  nombreValeurs;
				}
			}
		}
		
		for(nombreThreadCreer = 0; nombreThreadCreer<nombreThreadTotal; nombreThreadCreer++){
			pthread_create(&tid, NULL, &mainThread, thInfo[nombreThreadCreer]);
			pthread_join(tid, (void*)&res);
			tabRes[nombreThreadCreer] = *res;
			//~ printf("Threads %d fermées, résultat est %f\n", nombreThreadCreer, *res);
		}
		
		float miseEnCommunRes;
		if(comparerChaines(mode, "max")){
			miseEnCommunRes = max(tabRes, nombreThreadTotal);
		}
		else if(comparerChaines(mode, "min")){
			miseEnCommunRes = min(tabRes, nombreThreadTotal);
		}
		else if(comparerChaines(mode, "avg")){
			miseEnCommunRes = avg(tabRes, nombreThreadTotal);
		}
		else if(comparerChaines(mode, "sum")){
			miseEnCommunRes = sum(tabRes, nombreThreadTotal);
		}
		else if(comparerChaines(mode, "odd")){
			miseEnCommunRes = oddMiseEnCommun(tabRes, nombreThreadTotal);
		}
		else{
			fprintf(stderr, "Erreur dans le mode entrée\n");
			exit(EXIT_FAILURE);
		}
		
		for(i =0; i<nombreThreadTotal; i++){
			free(thInfo[i]);
		}
		return miseEnCommunRes;
	}
	return -1;
}

void* mainThread(void* a){
	struct byThread* id = (struct byThread*)a; int i, j = 0;
	assert(id->taille>0);
	float out[MAX_SIZE_BUF];
	for(i = id->deb; i<id->fin; i++){
		out[j] = id->chiffre[i];
		j++;
	}
	//~ for(i = 0; i<id->taille; i++){
		//~ printf("%f\n", out[i]);
	//~ }
	float * res = malloc(sizeof(float));
	if(comparerChaines(id->mode, "max")){
		*res = max(out, id->taille);
	}
	else if(comparerChaines(id->mode, "min")){
		*res = min(out, id->taille);
	}
	else if(comparerChaines(id->mode, "avg")){
		*res = avg(out, id->taille);
	}
	else if(comparerChaines(id->mode, "sum")){
		*res = sum(out, id->taille);
	}
	else if(comparerChaines(id->mode, "odd")){
		*res = odd(out, id->taille);
	}
	else{
		fprintf(stderr, "Erreur dans le mode entrée\n");
		exit(EXIT_FAILURE);
	}
	return (void*)(res);
}

float max(float in[], int tailleTab){
	assert(tailleTab>0);
	float max_actuel = in[0]; int i;
	for(i = 0; i<tailleTab; i++){
		if(in[i] > max_actuel){
			max_actuel = in[i];
		}
	}
	return max_actuel;
}

float min(float in[], int tailleTab){
	assert(tailleTab>0);
	float min_actuel = in[0]; int i;
	for(i = 0; i<tailleTab; i++){
		if(in[i] < min_actuel){
			min_actuel = in[i];
		}
	}
	return min_actuel;
}

float avg(float in[], int tailleTab){
	assert(tailleTab>0);
	float sum = 0; int i;
	for(i = 0; i<tailleTab; i++){
		sum += in[i];
	}
	return sum / tailleTab;
}

float sum(float in[], int tailleTab){
	assert(tailleTab>0);
	float sum = 0; int i;
	for(i = 0; i<tailleTab; i++){
		sum += in[i];
	}
	return sum;
}

float odd(float in[], int tailleTab){
	assert(tailleTab>0);
	float nombreValeursImpair = 0; int i;
	for(i = 0; i<tailleTab; i++){
		if((int)in[i]%2){
			nombreValeursImpair += 1;
		}
	}
	return nombreValeursImpair;
}

int oddMiseEnCommun(float in[], int tailleTab){
	assert(tailleTab>0);
	int nombreValeursImpair = 0; int i;
	for(i = 0; i<tailleTab; i++){
		nombreValeursImpair += in[i];
	}
	return nombreValeursImpair;
}

int comparerChaines(const char* chaine1, const char* chaine2){ 
	int i;
    if(strlen(chaine1) != strlen(chaine2)){
        return 0;
	}
	
    for(i=0;i<strlen(chaine1);i++){
        if( chaine1[i] != chaine2[i]){
            return 0;
        }
	}
    return 1;
}

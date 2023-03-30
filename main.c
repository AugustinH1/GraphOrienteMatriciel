#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <limits.h>  // pour INT_MAX


#include "./include/traces.h" 
#include "./include/check.h" 




//#define DEBUG  // si active test ouverture fichier : fopen ok
#include "./include/check.h"

char * outputPath = "./img";




// représentation sous forme matrice d'adjacence
typedef struct {
	unsigned int nbSommets;
	int *visited; // 0 = non visité, 1 = visité, 2 = en cours de visite
	int **mat;
}T_graphMD;

typedef struct {
	int *dist;
	int *pred;
}T_dp;


T_graphMD * newGraphMD(int n); // créer graphe forme matrice
void dumpGraphMD(FILE * fp, T_graphMD * g); // affichage de la structure du graphe
void createDotPOT(T_graphMD * g,  const char *basename); // permet de produire les images, utilise dumpGrpahMD

T_dp bellmanFord(T_graphMD * g,  int s); // calcul plus court chemin (alog page 47 du cours 
void affiche_ppcBellman(T_graphMD * g, int s); // fonction permet d'afficher le résultat sous forme de tableau

void afficherGraphSorieStandart(T_graphMD * g);
void printVisited(T_graphMD * g);



int main(void) {

	//Création du graphe
	T_graphMD * g = newGraphMD(8);
	
	g->mat[0][1] = 12;
	g->mat[0][5] = 15;
	g->mat[0][6] = 20;
	g->mat[1][2] = 21;
	g->mat[2][7] = 19;
	g->mat[3][2] = 7;
	g->mat[3][7] = 7;
	g->mat[4][3] = 13;
	g->mat[4][7] = 14;
	g->mat[5][2] = 17;
	g->mat[5][4] = 28;
	g->mat[5][6] = 4;
	g->mat[6][4] = 18;
	g->mat[6][7] = 45;
		


	

					
	// Afficher ce graphe sur la sortie standard : 
	// Format : 
	// sommet -> (poids) -> sommet
	afficherGraphSorieStandart(g);

	// Production du graphe sous forme d'image png createDotPNG
	createDotPOT(g ,"graphMD");

	affiche_ppcBellman(g, 1);
		
}
/*-----------------------------------------------------------------*/
T_graphMD * newGraphMD(int n) {
	
	T_graphMD * g;
	int i, j;

	g = (T_graphMD *) malloc( sizeof(T_graphMD) );
	g->nbSommets = n;
	g->mat = (int **)malloc(n * sizeof(int *));
	g->visited = (int *)malloc(n * sizeof(int *));

	for (i = 0; i < n; i++){
		g->mat[i] = (int *)malloc(n * sizeof(int));
	}
		

	for (i = 0; i < n; i++){
		g->visited[i] = 0;
		for (j = 0; j < n; j++){
			g->mat[i][j] = INT_MAX;
		}	
	}

	return g;
}


void dumpGraphMD(FILE * fp, T_graphMD * g) {
	// Affiche la structure du graphe dans un FILE *
	// Tester avec stdout

	int i, j;
	for (i = 0; i < g->nbSommets; i++) {

		switch(g->visited[i])
		{
			case 1:
				fprintf(fp, "%d [fillcolor=red];\n", i);
				break;
			case 2:
				fprintf(fp, "%d [fillcolor=green];\n", i);
				break;
			
		}

		for (j = 0; j < g->nbSommets; j++) {
			if (g->mat[i][j] != INT_MAX) {
				fprintf(fp, "%d -> %d [label = \"%d\"];\n", i, j, g->mat[i][j]);
			}
		}
	}
	
}

void createDotPOT(T_graphMD * g,  const char *basename) {
	static char oldBasename[FILENAME_MAX + 1] = "";
	static unsigned int noVersion = 0;

	char DOSSIER_DOT[FILENAME_MAX + 1]; 
	char DOSSIER_PNG[FILENAME_MAX + 1]; 

	char fnameDot [FILENAME_MAX + 1];
	char fnamePng [FILENAME_MAX + 1];
	char	cmdLine [2 * FILENAME_MAX + 20];
	FILE *fp;
	struct stat sb;
	

	// Au premier appel, création (si nécessaire) des répertoires
	// où seront rangés les fichiers .dot et .png générés par cette fonction	

	// il faut créer le répertoire outputPath s'il n'existe pas 
	if (stat(outputPath, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    } else {
        printf("Création du répertoire %s\n", outputPath);
		mkdir(outputPath, 0777);
    }

	// il faut créer les répertoires outputPath/png et /dot 
	sprintf(DOSSIER_DOT, "%s/dot/",outputPath);
	sprintf(DOSSIER_PNG, "%s/png/",outputPath);

	if (oldBasename[0] == '\0') {
		mkdir(DOSSIER_DOT,	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		mkdir(DOSSIER_PNG,	S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	}

	 // S'il y a changement de nom de base alors recommencer à zéro
	 // la numérotation des fichiers 

	if (strcmp(oldBasename, basename) != 0) {
		noVersion = 0;
		strcpy(oldBasename, basename); 
	}

	sprintf(fnameDot, "%s%s_v%02u.dot", DOSSIER_DOT, basename, noVersion);
	sprintf(fnamePng, "%s%s_v%02u.png", DOSSIER_PNG, basename, noVersion);

	CHECK_IF(fp = fopen(fnameDot, "w"), NULL, "erreur fopen dans saveDotBST"); 
	
	noVersion ++;
	
	// Ecrire ici l'entête pour le fichier Graphiz comme indiqué dans la page 17 du cours
	fprintf(fp, "digraph graphe {\n");
	fprintf(fp, "rankdir = LR;\n");
	fprintf(fp, "node [fontname=\"Arial\", shape = circle, color=lightblue, style=filled];\n");
	fprintf(fp, "edge [color=red];\n");

	


	// et appel de la fonction dumpGraphMD
	dumpGraphMD(fp, g);
	
	
	fprintf(fp, "}\n");	
		
	fclose(fp);

	sprintf(cmdLine, "dot -Tpng  %s -o %s", fnameDot, fnamePng);
	system(cmdLine);

	printf("Creation de '%s' et '%s' ... effectuee\n", fnameDot, fnamePng);
}



void afficherGraphSorieStandart(T_graphMD * g) {
	int i, j;
	for (i = 0; i < g->nbSommets; i++) {
		for (j = 0; j < g->nbSommets; j++) {
			if (g->mat[i][j] != INT_MAX) {
				printf("%d -> (%d) -> %d \n", i, j, g->mat[i][j]);
			}
		}
	}
}



T_dp bellmanFord(T_graphMD * g,  int s){
	// s est le numéro du sommet de départ
	
	T_dp dp;
	int i, j, k;

	dp.dist = (int *)malloc(g->nbSommets * sizeof(int));
	dp.pred = (int *)malloc(g->nbSommets * sizeof(int));

	// Initialisation
	for (i = 0; i < g->nbSommets; i++) {
		dp.dist[i] = INT_MAX;
		dp.pred[i] = 0;
	}

	dp.dist[s] = 0;
	

	// Boucle principale
	for (k = 0; k < g->nbSommets; k++) {
		if(k != 0){
			g->visited[k] = 1;
			g->visited[k-1] = 2;
		}

		

		for (i = 0; i < g->nbSommets; i++) {
			

			for (j = 0; j < g->nbSommets; j++) {

				if (g->mat[i][j] != INT_MAX) {
					if (dp.dist[i] != INT_MAX &&  dp.dist[i] + g->mat[i][j] < dp.dist[j]) {
						dp.dist[j] = dp.dist[i] + g->mat[i][j];
						dp.pred[j] = i;
					}
				}
				
				//printVisited(g);
			}
		}
		createDotPOT(g, "bellman");
	}



	for (i = 0; i < g->nbSommets; i++) {
		if (dp.dist[i] != INT_MAX) {
			printf("d[%d]=%d (s%d) \n", i, dp.dist[i], abs(dp.pred[i]));
		}
	}

	
	return dp;
}


void affiche_ppcBellman(T_graphMD * g, int s){
	// Affiche le chemin le plus court entre s et t

	int i, j;
	T_dp dp = bellmanFord(g, s);


	printf("Plus courts chemins (Bellman) depuis le sommet %d\n", s);
	printf("+-------------+----------+---------------------------+\n");
	printf("| org -> dest | Distance | Chemin                    |\n");
	printf("+-------------+----------+---------------------------+\n");

	for (i = 0; i < g->nbSommets; i++) {
		if (dp.dist[i] != INT_MAX) {
			printf("| %d -> %d      |  %-8d| %d", s, i, dp.dist[i], i);
			j = i;
			while (j != s) {
				printf("\t%d", dp.pred[j]);
				j = dp.pred[j];
			}
			printf("\n");
		}
	}
	printf("+-----------------+----------+-----------------------+\n");
}


void printVisited(T_graphMD * g) {
	for (int i = 0; i < g->nbSommets; i++) {
		printf("V[%d] = %d \n",i, g->visited[i]);
	}
}
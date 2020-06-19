#include <stdlib.h>

//Fonction permettant d'ajouter une partie aléatoire, elle prend le nb de base et lui ajoute un valeur comprise entre 0 et ce nombre de base
int randompart(int nb)
{
	return ((rand()%nb)+nb);
}
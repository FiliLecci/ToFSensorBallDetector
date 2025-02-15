#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    int x;
    int y;
} Coordinata;

typedef struct CoordList{
    Coordinata *pos;
    int lenght;
} CoordList;

CoordList *init_list()
{
    CoordList *list;

    if ((list = malloc(sizeof(CoordList))) == NULL)
    {
        perror("Errore malloc struct lista\n");
        abort();
    }
    if((list->pos = malloc(sizeof(Coordinata))) == NULL)
    {
        perror("Errore malloc lista coord\n");
        abort();
    }

    list->lenght = 0;

    return list;
}

CoordList *inserisci_punto(CoordList *list, Coordinata punto)
{
    (list->lenght)++;
    if((list->pos = (Coordinata*)realloc(list->pos, list->lenght * sizeof(Coordinata))) == NULL)
    {
        perror("Errore realloc lista coord\n");
        abort();
    }

    list->pos[(list->lenght)-1].x = punto.x;
    list->pos[(list->lenght)-1].y = punto.y;

    return list;
}


void distruggi_lista(CoordList *list)
{
    free(list->pos);
    free(list);
}
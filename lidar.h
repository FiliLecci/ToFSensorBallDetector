#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coordList.h"

typedef struct {
    float angolo, distanza;
} LidarMeasure;

// Struttura per rappresentare il lidar
typedef struct {
    Coordinata pos;
    float risoluzione_ang;     // la risoluzione angolare del LIDAR in gradi
    int numero_punti;       // indica il numero di misurazioni che il lidar effettua ogni 360°
    int numero_misurazioni; // il numero di misurazioni effettuate
    LidarMeasure *misure;   // un array di misure effettate dal lidar

    float errore_misurazione_perc; // l'errore medio del lidar in percentuale sulla distanza
} Lidar;

void init_measures(Lidar *lidar)
{
    lidar->numero_misurazioni = 0;
    lidar->misure = (LidarMeasure*)malloc(sizeof(LidarMeasure));
}

// Inizializza il lidar
void *init_lidar(float risoluzione_ang)
{
    Lidar *lidar = (Lidar*)malloc(sizeof(Lidar));

    lidar->pos.x = 0;
    lidar->pos.y = 0;
    lidar->risoluzione_ang = risoluzione_ang;
    lidar->numero_punti = 500;
    
    init_measures(lidar);

    return lidar;
}

// Inizializza il lidar specificando la posizione
void *init_lidar_with_position(float risoluzione_ang, int x, int y)
{
    Lidar *lidar = (Lidar*)malloc(sizeof(Lidar));

    lidar->pos.x = x;
    lidar->pos.y = y;
    lidar->risoluzione_ang = risoluzione_ang;
    lidar->numero_punti = (int)round(360/risoluzione_ang + 0.5);    // aggiungo 0.5 così arrotonda sempre all'intero successivo

    printf("%f, %d\n", lidar->risoluzione_ang, lidar->numero_punti);
    
    init_measures(lidar);

    return lidar;
}

void distruggi_lidar(Lidar *lidar)
{
    free(lidar->misure);
    free(lidar);
}

void svuota_misure(Lidar *lidar)
{
    lidar->numero_misurazioni = 0;
    lidar->misure = realloc(lidar->misure, 0);
}

void add_misura(Lidar *lidar, float angolo, float distanza)
{
    // se ho passato il numero massimo di misurazioni riparto dall'inizio
    if(lidar->numero_misurazioni == lidar->numero_punti){
        svuota_misure(lidar);
    }

    lidar->numero_misurazioni ++;
    lidar->misure = (LidarMeasure*)realloc(lidar->misure, sizeof(LidarMeasure)*lidar->numero_misurazioni);

    lidar->misure[lidar->numero_misurazioni-1].angolo = angolo;
    lidar->misure[lidar->numero_misurazioni-1].distanza = distanza;
}
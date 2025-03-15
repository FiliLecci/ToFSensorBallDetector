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
    int numero_punti;    // indica il numero di misurazioni che il lidar effettua ogni 360°
    int numero_misurazioni; // il numero di misurazioni effettuate
    LidarMeasure *misure;  // un array di misure effettate dal lidar
} Lidar;

void init_measures(Lidar *lidar)
{
    lidar->numero_misurazioni = 0;
    lidar->misure = (LidarMeasure*)malloc(sizeof(LidarMeasure));
}

// Inizializza il lidar
void *init_lidar(int numero_punti)
{
    Lidar *lidar = (Lidar*)malloc(sizeof(Lidar));

    lidar->pos.x = 0;
    lidar->pos.y = 0;
    lidar->numero_punti = numero_punti;
    
    init_measures(lidar);

    return lidar;
}

// Inizializza il lidar usando le dimensioni dello schermo
void *init_lidar_with_screen_size(int numero_punti, int window_w, int window_h)
{
    Lidar *lidar = (Lidar*)malloc(sizeof(Lidar));

    lidar->pos.x = window_w/2;
    lidar->pos.y = window_h/2;
    lidar->numero_punti = numero_punti;
    
    init_measures(lidar);

    return lidar;
}

// Inizializza il lidar specificando la posizione
void *init_lidar_with_position(int numero_punti, int x, int y)
{
    Lidar *lidar = (Lidar*)malloc(sizeof(Lidar));

    lidar->pos.x = x;
    lidar->pos.y = y;
    lidar->numero_punti = numero_punti;
    
    init_measures(lidar);

    return lidar;
}

void distruggi_lidar(Lidar *lidar)
{
    free(lidar->misure);
    free(lidar);
}

void add_misura(Lidar *lidar, float angolo, float distanza)
{
    lidar->numero_misurazioni ++;
    lidar->misure = (LidarMeasure*)realloc(lidar->misure, sizeof(LidarMeasure)*lidar->numero_misurazioni);

    lidar->misure[lidar->numero_misurazioni-1].angolo = angolo;
    lidar->misure[lidar->numero_misurazioni-1].distanza = distanza;
}

void svuota_misure(Lidar *lidar)
{
    lidar->numero_misurazioni = 0;
    lidar->misure = realloc(lidar->misure, 0);
}
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "coordList.h"

// 1mm = 1pixel

#define N 20  // Numero di sensori
#define SENSOR_DISTANCE 50 // Distranza tra i sensori 5cm = 0.05m
#define R 109  // Raggio della sfera (109mm)
#define SENSOR_MAX_DISTANCE 1096 //la distanza massima di rilevamento di una palla: 1 gutter (~90mm) + pista (1006mm)
#define WINDOW_W 1200
#define WINDOW_H 1000

// Struttura per rappresentare un sensore
typedef struct {
    Coordinata pos;
    int dist;   // distanza misurata
    int sensor_id; // id del sensore
} Sensor;

// Struttura per la sfera
typedef struct {
    Coordinata pos;
} Sphere;

typedef enum {false, true} bool;

// Inizializza i sensori lungo l'asse Y
void init_sensors(Sensor sensors[], int n)
{
    for (int i = 0; i < n; i++) {
        sensors[i].pos.x = 10;  // I sensori sono fissi a sinistra
        sensors[i].pos.y = SENSOR_DISTANCE * i + 25;
        sensors[i].dist = SENSOR_MAX_DISTANCE;
        sensors[i].sensor_id = i;
    }
}

SDL_Window *init_window()
{
    SDL_Window *window = SDL_CreateWindow("Simulazione Sensori Laser", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    
    if (!window) {
        printf("Errore nella creazione della finestra: %s\n", SDL_GetError());
        SDL_Quit();
        return NULL;
    }

    return window;
}

SDL_Renderer *init_renderer(SDL_Window *window)
{
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Errore nella creazione del renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return NULL;
    }

    return renderer;
}

long double get_millis(struct timespec ts)
{
    return (ts.tv_nsec / 1e6) + (ts.tv_sec * 1e3);
}

// Genera una posizione casuale della sfera
void generate_sphere(Sphere *s)
{
    s->pos.x = ((double) rand() / RAND_MAX) * SENSOR_MAX_DISTANCE + 1.25*R;
    s->pos.y = ((double) rand() / RAND_MAX) * (SENSOR_DISTANCE * N);
}

void disegna_sfera(SDL_Renderer *renderer, int cx, int cy, int r)
{
    for (int w = 0; w < r * 2; w++)
    {
        for (int h = 0; h < r * 2; h++)
        {
            int dx = r - w;
            int dy = r - h;
            if ((dx * dx + dy * dy) <= (r * r))
            {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

// Per ogni sensore calcola il punto, se esiste, in cui incontra la sfera (precisione 1mm = 1px)
void calcola_distanza(Sensor *sensor, Sphere s)
{
    int dy = sensor->pos.y;

    for (int w = 0; w < SENSOR_MAX_DISTANCE; w++)
    {
        int dx = w+5;

        if (pow(dx - s.pos.x, 2) + pow(dy - s.pos.y, 2) <= (R * R))
        {
            sensor->dist = dx;
            return;
        }
    }

    sensor->dist = SENSOR_MAX_DISTANCE;
}

/* Determina le coordinate dei punti a contatto con la sfera e sensori ad essi associati. 
 * @param *lista_punti l'array nel quale vengono inseriti i punti calolati
 * @param sensors[] la lista dei sensori da verificare
 * @param id_sensori[] array nel quale mettere gli id dei sensori che rilevano la palla
 * 
 * @return numero di punti trovati
 */
void seleziona_punti(CoordList *lista_punti, Sensor sensors[], int *id_ultimo_sensore)
{
    for (int i = 0; i < N; i++)
    {
        if(sensors[i].dist < SENSOR_MAX_DISTANCE)
        {
            Coordinata c;
            c.x = sensors[i].dist;
            c.y = sensors[i].pos.y;
            inserisci_punto(lista_punti, c);

            *id_ultimo_sensore = sensors[i].sensor_id;
        }
    }
}

// Funzione per calcolare il centro della circonferenza data da tre punti
Coordinata calcola_centro(Coordinata p1, Coordinata p2, Coordinata p3)
{
    Coordinata centro;
    // trovo i coefficienti del sistema ed uso Cramer
    double A1 = -2 * (p2.x - p1.x);
    double B1 = -2 * (p2.y - p1.y);
    double C1 = p2.x * p2.x - p1.x * p1.x + p2.y * p2.y - p1.y * p1.y;

    double A2 = -2 * (p3.x - p1.x);
    double B2 = -2 * (p3.y - p1.y);
    double C2 = p3.x * p3.x - p1.x * p1.x + p3.y * p3.y - p1.y * p1.y;

    double det = A1 * B2 - A2 * B1;
    if (det == 0) {
        centro.x = 0;
        centro.y = 0;
        return centro;
    }

    // Calcolo le coordinate del centro
    centro.x = abs((C1 * B2 - C2 * B1) / det);
    centro.y = abs((A1 * C2 - A2 * C1) / det);
    return centro;
}

/* Calcola il centro della circonferenza due volte e restituisce il punto medio tra le due
 * per cercare di avere una misurazione più precisa
 * @returns 0 se il centro non può essere calcolato, 1 altrimenti
*/
int trova_centro(CoordList *lista_punti, Coordinata *centro)
{
    Coordinata c1, c2;
    int list_len = lista_punti->lenght;

    if(list_len < 3)
    {
        centro->x = 0;
        centro->y = 0;
        return 0;
    }

    if(list_len >= 3)
    {
        c1 = calcola_centro(lista_punti->pos[0], lista_punti->pos[1], lista_punti->pos[2]);

        centro->x = c1.x;
        centro->y = c1.y;
    }

    if(list_len > 3)
    {
        c2 = calcola_centro(lista_punti->pos[list_len-1], lista_punti->pos[list_len-2], lista_punti->pos[list_len-3]);

        centro->x = (c1.x+c2.x)/2;
        centro->y = (c1.y+c2.y)/2;
    }

    return 1;
}

// Funzione per disegnare la scena
void draw_scene(SDL_Renderer *renderer, Sensor sensors[], Sphere *s, CoordList posizioni[])
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Disegna i sensori
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < N; i++)
    {
        SDL_Rect rect = {sensors[i].pos.x - 5, sensors[i].pos.y-5, 10, 10};
        SDL_RenderFillRect(renderer, &rect);
    }

    // sposta il cerchio in basso e gli fa seguire una curva sinusoidale
    s->pos.y = s->pos.y - 2;
    if(s->pos.y > SENSOR_DISTANCE*N-1 || s->pos.y <= 0)  // Arrivo a fondo schermo
    {
        s->pos.y = SENSOR_DISTANCE*N-1;
        svuota_lista(posizioni);    // Svuota la lista delle posizioni del centro
    }

    // s->pos.x = log((double)s->pos.y*0.02 + 1.1)*260 + 2*R + 10;
    s->pos.x = -atan((double)s->pos.y*0.01)*200 + 600;

    // Disegna i raggi laser
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < N; i++)
    {
        calcola_distanza(&sensors[i], *s);
        SDL_RenderDrawLine(renderer, (sensors[i].pos.x+5), sensors[i].pos.y, (sensors[i].dist+5), sensors[i].pos.y);
    }

    // Disegna la sfera
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    disegna_sfera(renderer, s->pos.x, s->pos.y, R);

    // Ottiene i punti di contatto come lista
    CoordList *lista_punti = (CoordList*)init_list();
    static int id_ultimo_sensore;   // Static mantiene il valore della variabile fino alla terminazione del programma
    static struct timespec last_checkpoint_ts;   // Il momento del passaggio sull'ultimo sensore
    float velocita = 0.0; // Km/h
    int new_ultimo_sensore; // Contiene l'id dell'ultimo sensore attuale

    seleziona_punti(lista_punti, sensors, &new_ultimo_sensore);

    if(new_ultimo_sensore != id_ultimo_sensore)
    {
        // Calcola la velocità
        struct timespec last_ts;
        timespec_get(&last_ts, TIME_UTC);

        long double start_ms = get_millis(last_checkpoint_ts);
        long double end_ms = get_millis(last_ts);

        double delta_t = (end_ms - start_ms) / 1e3;

        velocita = (SENSOR_DISTANCE / delta_t)/1e6*3600;   // (mm/s)/1000*3600 = Km/h

        printf("%f Km/h\n", velocita);

        last_checkpoint_ts = last_ts;

        id_ultimo_sensore = new_ultimo_sensore;
    }

    // Disegna i punti di contatto
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    int l = lista_punti->lenght;

    for (int i = 0; i < l; i++)
    {
        disegna_sfera(renderer, lista_punti->pos[i].x, lista_punti->pos[i].y, 5);
    }

    // Calcola il centro del cerchio usando i primi 3 punti e gli ultimi 3 per ridondanza, poi usa il punto medio
    Coordinata centro;

    if(trova_centro(lista_punti, &centro))
    {
        disegna_sfera(renderer, centro.x, centro.y, 5); // Disegna il centro calcolato
        inserisci_punto(posizioni, centro); // Memorizza il nuovo punto centrale calcolato
    }

    // Disegno il centro reale dopo così si vede anche se è sovrapposto
    SDL_SetRenderDrawColor(renderer, 199, 34, 191, 255);
    disegna_sfera(renderer, s->pos.x, s->pos.y, 5);

    // Disegna le linee tra le varie posizioni
    SDL_SetRenderDrawColor(renderer, 255, 124, 124, 255);
    for (int i = 0; i < posizioni->lenght-1; i++)
    {
        Coordinata p1 = posizioni->pos[i];
        Coordinata p2 = posizioni->pos[i+1];

        SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);   // Disegna una linea tra i punti
    }

    distruggi_lista(lista_punti);   // Libera la lista

    SDL_RenderPresent(renderer);
}

int start_scene()
{
    SDL_Window *window = init_window();

    SDL_Renderer *renderer = init_renderer(window);

    Sensor sensors[N];
    Sphere s;

    init_sensors(sensors, N);
    generate_sphere(&s);

    // inizializza lista dello storico delle posizioni 
    CoordList *posizioni = (CoordList*)init_list();

    int running = 1;
    SDL_Event event;

    while (running) {
        draw_scene(renderer, sensors, &s, posizioni);
        SDL_Delay(10);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                printf("Chiusura programmo uga buga\n");
                running = 0;
            }
        }
    }

    distruggi_lista(posizioni);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 1;
}

int main()
{
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Errore nell'inizializzazione di SDL: %s\n", SDL_GetError());
        return -1;
    }

    if(start_scene() == -1)
        perror("Errore nell'inizializzazione della scena\n");


    return 0;
}

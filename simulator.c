#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

// 1mm = 1pixel

#define N 20  // Numero di sensori
#define SENSOR_DISTANCE 50// Distranza tra i sensori
#define R 109  // Raggio della sfera (109mm)
#define SENSOR_MAX_DISTANCE 1096 //la distanza massima di rilevamento di una palla: 1 gutter (~90mm) + pista (1006mm)
#define WINDOW_W 1200
#define WINDOW_H 900

typedef struct{
    int x;
    int y;
} Coordinata;

// Struttura per rappresentare un sensore
typedef struct {
    Coordinata pos;
    int dist;   //distanza misurata
} Sensor;

// Struttura per la sfera
typedef struct {
    Coordinata pos;
} Sphere;

typedef struct{
    Coordinata pos;
    struct CoordList *nex_pos;
} CoordList;


void inserisci_punto(CoordList *lista, Coordinata punto)
{
    while (lista != NULL)
    {
        lista = lista->nex_pos;
    }

    CoordList *nodo = malloc(sizeof(CoordList));
}

// Inizializza i sensori lungo l'asse Y
void init_sensors(Sensor sensors[], int n)
{
    for (int i = 0; i < n; i++) {
        sensors[i].pos.x = 10;  // I sensori sono fissi a sinistra
        sensors[i].pos.y = SENSOR_DISTANCE * i;
        sensors[i].dist = SENSOR_MAX_DISTANCE;
    }
}

// Inizializza array di punti
void init_coord(Coordinata punti[], int n)
{
    for (int i = 0; i < n; i++)
    {
        punti[i].x = 0;
        punti[i].y = 0;
    }
}

// Genera una posizione casuale della sfera
void generate_sphere(Sphere *s)
{
    s->pos.x = ((double) rand() / RAND_MAX) * SENSOR_MAX_DISTANCE + R;
    s->pos.y = R + ((double) rand() / RAND_MAX) * (SENSOR_DISTANCE * N - 2 * R);
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

// Determina le coordinate dei punti a contatto con la sfera
CoordList* seleziona_punti(Sensor sensors[])
{
    CoordList *c, *c1;

    for (int i = 0; i < N; i++)
    {
        if(sensors[i].dist < SENSOR_MAX_DISTANCE)
        {
            c->pos.x = sensors[i].dist;
            c->pos.y = sensors[i].pos.y;
        }
    }

}

// Funzione per disegnare la scena
void draw_scene(SDL_Renderer *renderer, Sensor sensors[], Sphere *s, int n)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Disegna i sensori
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < n; i++)
    {
        SDL_Rect rect = {sensors[i].pos.x - 5, sensors[i].pos.y, 10, 10};
        SDL_RenderFillRect(renderer, &rect);
    }

    // Disegna i raggi laser
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < n; i++)
    {
        calcola_distanza(&sensors[i], *s);
        SDL_RenderDrawLine(renderer, (sensors[i].pos.x+5), (sensors[i].pos.y+5), (sensors[i].dist+5), (sensors[i].pos.y+5));
    }

    // Sposta la sfera
    s->pos.y = (s->pos.y + 1) % (N*SENSOR_DISTANCE);

    // Disegna la sfera
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int w = 0; w < R * 2; w++)
    {
        for (int h = 0; h < R * 2; h++)
        {
            int dx = R - w;
            int dy = R - h;
            if ((dx * dx + dy * dy) <= (R * R))
            {
                SDL_RenderDrawPoint(renderer, s->pos.x + dx, s->pos.y + dy);
            }
        }
    }

    // Disegna i punti di contatto
    //seleziona_punti(sensors, );

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

    SDL_RenderPresent(renderer);
}

int main()
{
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Errore nell'inizializzazione di SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Simulazione Sensori Laser", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Errore nella creazione della finestra: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Errore nella creazione del renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    int max_contact_points = round((2*R)/SENSOR_DISTANCE); // Il numero massimo di sensori che possono vedere la sfera nello stesso momento

    Sensor sensors[N];
    Sphere s;

    init_sensors(sensors, N);
    generate_sphere(&s);

    int running = 1;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = 0;
            }
        }
        draw_scene(renderer, sensors, &s, N);
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

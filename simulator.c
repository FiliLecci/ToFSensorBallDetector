#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>

// 1mm = 1pixel

#define N 20  // Numero di sensori
#define SENSOR_DISTANCE 50// Distranza tra i sensori
#define R 109.0  // Raggio della sfera (109mm)
#define SENSOR_MAX_DISTANCE 1096 //la distanza massima di rilevamento di una palla: 1 gutter (~90mm) + pista (1006mm)
#define WINDOW_W 1900
#define WINDOW_H 1080

// Struttura per rappresentare un sensore
typedef struct {
    double x;   //posizione x
    double y;   //posizione y
    int dist;   //distanza misurata
} Sensor;

// Struttura per la sfera
typedef struct {
    double x;
    double y;
} Sphere;

// Inizializza i sensori lungo l'asse Y
void init_sensors(Sensor sensors[], int n) 
{
    for (int i = 0; i < n; i++) {
        sensors[i].x = 10;  // I sensori sono fissi a sinistra
        sensors[i].y = SENSOR_DISTANCE * i;
        sensors[i].dist = SENSOR_MAX_DISTANCE;
    }
}

// Genera una posizione casuale della sfera
void generate_sphere(Sphere *s) 
{
    s->x = ((double) rand() / RAND_MAX) * (SENSOR_DISTANCE * N - 2 * R);
    s->y = R + ((double) rand() / RAND_MAX) * (SENSOR_DISTANCE * N - 2 * R);
}

// Per ogni sensore calcola il punto, se esiste, in cui incontra la sfera (precisione 1mm = 1px)
void calcola_distanza(Sensor *sensor, Sphere s)
{
    int dy = sensor->y;

    for (int w = 0; w < SENSOR_MAX_DISTANCE; w++)
    {
        int dx = w+5;

        // printf("Testing sensor %d at dist %d\n\r", dy/SENSOR_DISTANCE, dx);

        if (pow(dx - s.x, 2) + pow(dy - s.y, 2) <= (R * R)) 
        {
            printf("Sensor %d --> %d\n\r", dy/SENSOR_DISTANCE, dx);
            sensor->dist = dx;
            return;
        }
    }
}

// Funzione per disegnare la scena
void draw_scene(SDL_Renderer *renderer, Sensor sensors[], Sphere s, int n) 
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Disegna i sensori
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int i = 0; i < n; i++) 
    {
        SDL_Rect rect = {sensors[i].x - 5, sensors[i].y, 10, 10};
        SDL_RenderFillRect(renderer, &rect);
    }

    // Disegna i raggi laser
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < n; i++) 
    {
        calcola_distanza(&sensors[i], s);
        printf("Sensore %d --> %d mm\n\r", i, sensors[i].dist);
        SDL_RenderDrawLine(renderer, (sensors[i].x+5), (sensors[i].y+5), (sensors[i].dist+5), (sensors[i].y+5));
    }

    // Disegna la sfera
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (int w = 0; w < R * 2; w++) 
    {
        for (int h = 0; h < R * 2; h++) 
        {
            int dx = R - w;
            int dy = R - h;
            if ((dx * dx + dy * dy) <= (R * R)) 
            {
                SDL_RenderDrawPoint(renderer, s.x + dx, s.y + dy);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

int main() 
{
    srand(time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Errore nell'inizializzazione di SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("Simulazione Sensori Laser", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_W, WINDOW_H, SDL_WINDOW_SHOWN+SDL_WINDOW_FULLSCREEN_DESKTOP);
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
        draw_scene(renderer, sensors, s, N);
        SDL_Delay(100);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

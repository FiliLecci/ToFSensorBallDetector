#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "lidar.h"

// 1mm = 1pixel

#ifndef M_PI
#define M_PI 3.141592653589793238462643383279502884
#endif

#define R 109  // Raggio della sfera (109mm)
#define WINDOW_W 1920
#define WINDOW_H 1080
#define NUMERO_PUNTI 400
#define SENSOR_MAX_DISTANCE 800 //la distanza massima di rilevamento di una palla: 1 gutter (~90mm) + pista (1006mm)
#define LIDAR_ANGLE 180 // il FoV del LiDAR in °, simmetrico rispetto all'asse x (90° = 45° sopra e 45° sotto l'asse x)

#define toRad(deg) (deg*M_PI/180)

// Struttura per la sfera
typedef struct {
    Coordinata pos;
} Sphere;

typedef enum {False, True} bool;


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
    s->pos.x = ((double) rand() / RAND_MAX)*(SENSOR_MAX_DISTANCE*2)+(WINDOW_W/2)-SENSOR_MAX_DISTANCE;
    s->pos.y = ((double) rand() / RAND_MAX)*(SENSOR_MAX_DISTANCE*2)+(WINDOW_H/2)-SENSOR_MAX_DISTANCE;
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

void disegna_lidar(Lidar *lidar, SDL_Renderer *renderer)
{
    // Disegna i raggi laser
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int i = 0; i < lidar->numero_misurazioni; i++)
    {
        float angolo = (360.0/NUMERO_PUNTI)*i;

        float distanza = lidar->misure[i].distanza;

        SDL_RenderDrawLine(renderer, lidar->pos.x, lidar->pos.y, lidar->pos.x+distanza*cos(toRad(angolo)), lidar->pos.y+distanza*sin(toRad(angolo)));
    }

    // Disegna il LIDAR
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_Rect rect = {lidar->pos.x - 10, lidar->pos.y - 10, 20, 20};
    SDL_RenderFillRect(renderer, &rect);
}

// Calcola la distanza tra p1 e p2
float distanza_punti(Coordinata p1, Coordinata p2)
{
    if(p1.x == p2.x)
        return p2.y - p1.y;
    
    if(p1.y == p2.y)
        return p2.x - p1.x;

    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

// Funzione per calcolare le soluzioni dell'intersezione
void calcola_distanza(float angle, Lidar *lidar, Sphere s) {
    // Coefficiente angolare della retta
    float rad = angle * M_PI / 180;
    double m;
    bool vertical_line = False;

    // Gestione esplicita dei casi in cui l'angolo è 90° o 270°
    if (angle == 90 || angle == 270) {
        vertical_line = True;
    } else {
        m = tan(rad);  // Tangente dell'angolo in radianti
    }

    float yl = lidar->pos.y, xl = lidar->pos.x;
    float xc = s.pos.x, yc = s.pos.y;

    double a, b, c;

    if (vertical_line) {
        // Gestione del caso di linea verticale
        a = 1;
        b = -2 * yc;
        c = yc * yc + (xl - xc) * (xl - xc) - R * R;
    } else {
        // Espandiamo l'equazione per ottenere una forma quadratica rispetto a x
        a = 1 + m * m;  // Coefficiente di x^2
        b = -(2 * xc) - (2 * m * m * xl) + (2 * m * yl) - (2 * m * yc);  // Coefficiente di x
        c = (xc * xc) + ((xl * xl) * (m * m)) - (2 * xl * yl * m) + (2 * xl * yc * m) + (yl * yl) - (2 * yl * yc) + (yc * yc) - (R * R);  // Termine costante
    }

    // Risolviamo l'equazione quadratica ax^2 + bx + c = 0
    double discriminante = b * b - 4 * a * c;

    if (discriminante < 0) {
        add_misura(lidar, angle, SENSOR_MAX_DISTANCE);
        return;
    }

    double x1, x2, y1, y2;

    if (vertical_line) {
        x1 = xl;
        x2 = xl;
        y1 = yc + sqrt(discriminante) / (2 * a);
        y2 = yc - sqrt(discriminante) / (2 * a);
    } else {
        x1 = (-b + sqrt(discriminante)) / (2 * a);
        x2 = (-b - sqrt(discriminante)) / (2 * a);
        y1 = m * (x1 - xl) + yl;
        y2 = m * (x2 - xl) + yl;
    }

    Coordinata c1, c2;
    c1.x = x1;
    c1.y = y1;
    c2.x = x2;
    c2.y = y2;

    float dist1 = distanza_punti(c1, lidar->pos);
    float dist2 = distanza_punti(c2, lidar->pos);

    if (dist1 > SENSOR_MAX_DISTANCE && dist2 > SENSOR_MAX_DISTANCE) {
        add_misura(lidar, angle, SENSOR_MAX_DISTANCE);
        return;
    }

    // Calcola il vettore direzione del raggio
    float dx = cos(rad);
    float dy = sin(rad);

    // Determina se i due punti sono nella direzione del raggio
    bool c1_in_direction = ((c1.x - xl) * dx + (c1.y - yl) * dy) > 0;
    bool c2_in_direction = ((c2.x - xl) * dx + (c2.y - yl) * dy) > 0;

    if (c1_in_direction && c2_in_direction) {
        // Entrambi i punti sono nella direzione del raggio, prende il più vicino
        if (dist1 < dist2) {
            add_misura(lidar, angle, dist1);
        } else {
            add_misura(lidar, angle, dist2);
        }
    } else if (c1_in_direction) {
        add_misura(lidar, angle, dist1);
    } else if (c2_in_direction) {
        add_misura(lidar, angle, dist2);
    } else {
        // Nessun punto è nella direzione (credo sia impossibile ma non si sa mai)
        add_misura(lidar, angle, SENSOR_MAX_DISTANCE);
    }
}

// Aggiunge per ogni angolo delle misurazioni il dato relativo
void fetch_lidar(Lidar *lidar, Sphere *s)
{
    for (int i = 0; i < NUMERO_PUNTI; i++)
    {
        float angolo = (360.0 / NUMERO_PUNTI)*i;
        calcola_distanza(angolo, lidar, *s);
    }
}

/* Determina le coordinate dei punti a contatto con la sfera e sensori ad essi associati. 
 * @param *lista_punti l'array nel quale vengono inseriti i punti calolati
 * @param *lidar il lidar che effettua le misurazioni
 */
void seleziona_punti(CoordList *lista_punti, Lidar *lidar)
{
    for (int i = 0; i < lidar->numero_punti; i++)
    {
        if(lidar->misure[i].distanza < SENSOR_MAX_DISTANCE)
        {
            Coordinata c;
            c.x = lidar->pos.x+(cos(toRad(lidar->misure[i].angolo))*lidar->misure[i].distanza);
            c.y = lidar->pos.y+(sin(toRad(lidar->misure[i].angolo))*lidar->misure[i].distanza);

            inserisci_punto(lista_punti, c);
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
        c1 = calcola_centro(lista_punti->pos[0], lista_punti->pos[(int)(list_len/2)], lista_punti->pos[list_len-1]);

        centro->x = c1.x;
        centro->y = c1.y;
    }

    if(list_len > 3)
    {
        c2 = calcola_centro(lista_punti->pos[0], lista_punti->pos[(int)(list_len/2 - 1)], lista_punti->pos[list_len-1]);

        centro->x = (c1.x+c2.x)/2;
        centro->y = (c1.y+c2.y)/2;
    }

    return 1;
}

// Funzione per disegnare la scena
void draw_scene(SDL_Renderer *renderer, Lidar *lidar, Sphere *s, CoordList posizioni[])
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // sposta il cerchio in basso e gli fa seguire una curva
    s->pos.y = s->pos.y - 2;
    if(s->pos.y > WINDOW_H || s->pos.y <= 0)  // Arrivo a fondo schermo
    {
        s->pos.y = WINDOW_H;
        svuota_lista(posizioni);    // Svuota la lista delle posizioni del centro
    }
    s->pos.x = -atan((double)s->pos.y*0.01)*200 + 600;

    // Disegna la sfera
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    disegna_sfera(renderer, s->pos.x, s->pos.y, R);

    // Calcola le misurazioni del lidar
    fetch_lidar(lidar, s);
    // Diesegna i raggi del lidar e il lidar stesso
    disegna_lidar(lidar, renderer);

    // Ottiene i punti di contatto come lista
    CoordList *lista_punti = init_list();
    float velocita = 0.0; // Km/h

    seleziona_punti(lista_punti, lidar);

    for (int i = 0; i < lista_punti->lenght; i++)
    {
        disegna_sfera(renderer, lista_punti->pos[i].x, lista_punti->pos[i].y, 3); // disegna i punti di contatto
    }
    

    /*
    // TODO Calcola la velocità usando la storia delle posizioni
    struct timespec last_ts;
    timespec_get(&last_ts, TIME_UTC);

    long double start_ms = get_millis(last_checkpoint_ts);
    long double end_ms = get_millis(last_ts);

    double delta_t = (end_ms - start_ms) / 1e3;

    // FIX velocita = (SENSOR_DISTANCE / delta_t)/1e6*3600;   // (mm/s)/1000*3600 = Km/h

    printf("%f Km/h\n", velocita);
    */

    // Calcola il centro del cerchio usando i primi 3 punti e gli ultimi 3 per ridondanza, poi usa il punto medio
    Coordinata centro;

    if(trova_centro(lista_punti, &centro) == True)
    {
        disegna_sfera(renderer, centro.x, centro.y, 6); // Disegna il centro calcolato
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
    svuota_misure(lidar);

    SDL_RenderPresent(renderer);
}

int start_scene()
{
    SDL_Window *window = init_window();

    SDL_Renderer *renderer = init_renderer(window);

    Sphere s;

    Lidar *lidar = (Lidar*)init_lidar_with_screen_size(NUMERO_PUNTI, WINDOW_W, WINDOW_H);
    generate_sphere(&s);

    // inizializza lista dello storico delle posizioni
    CoordList *posizioni = init_list();

    int running = 1;
    SDL_Event event;

    while (running) {
        draw_scene(renderer, lidar, &s, posizioni);
        SDL_Delay(10);
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                printf("Chiusura programmo uga buga pt.2\n");
                running = 0;
            }
        }

        //getchar();    // Per procedere a step
    }

    distruggi_lidar(lidar);
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

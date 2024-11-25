#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL_ttf.h>


// Dimensiones de la ventana
const int ANCHO_PANTALLA = 1280;
const int LARGO_PANTALLA = 720;

// Dimensiones de la nave
const int ANCHO_NAVE = 80;
const int LARGO_NAVE = 60;

// Dimensiones de las balas
const int ANCHO_BALAS = 5;
const int LARGO_BALAS = 10;

// Dimensiones de las naves enemigas
const int ANCHO_ENEMIGOS = 100;
const int LARGO_ENEMIGOS = 80;

// Dimensiones y configuración del orbe de vida
const int TAMANIO_ORB = 30;
const float ORB_VIDA_HEAL = 20.0f;
const float ORB_VELOCIDAD = 1.0f;
const int MAX_ORBS = 10;

// Dimensiones y configuración del orbe de velocidad
const int FIRE_TAMANIO_ORB = 30;
const float FIRE_RATE_BOOST = 0.6f;
const float FIRE_ORB_VELOCIDAD = 1.0f;
const int MAX_FIRE_ORBS = 10;

// Máximo número de balas en pantalla
const int MAX_BALAS = 100;
const int MAX_BALAS_ENEMIGOS = 50;

// Máximo número de asteroides y naves enemigas
const int MAX_ASTEROIDES = 16;
const int MAX_ENEMIGOS =10;

// Cooldown entre disparos
const int BALAS_COOLDOWN = 15;
const int BOOSTED_BALAS_COOLDOWN =5;
const int ENEMIGOS_BALAS_COOLDOWN = 120;

// Velocidades
const float VELOCIDAD_NAVE = 2.2222f;
const int VELOCIDAD_BALAS = 5;
const int ENEMIGO_VELOCIDAD_BALAS = 3;
const float VELOCIDAD_ASTEROIDES = 2.0f;
const float VELOCIDAD_ENEMIGOS = 1.9999f;

// Sistema de vida y escudo
const float VIDA_INICIAL = 100.0f;
const float ESCUDO_MAX = 120.0f;
const float DANIO_BALA = 20.0f;
const float DANIO_ASTEROIDES = 150.0f;
const float DANIO_COLISION_ENEMIGO = 150.0f;

// Sistema de puntuación
const int PUNTOS_ASTEROIDES = 1;
const int PUNTOS_ENEMIGOS = 3;

TTF_Font* font = NULL;
typedef struct {
    int x, y;
    int active;
    int size;
} Asteroide;

typedef struct {
    int x, y;
    int active;
    int speed;
} Balas;

typedef struct {
    int x, y;
    int active;
    int BalasCooldown;
} Enemigo;

typedef struct {
    int x, y;
    int active;
    int speed;
} EnemigoBalas;

typedef struct {
    float x, y;
    float vida;
    int fireRateBoosted;
    Uint32 fireRateBoostStartTime;  // Tiempo cuando se obtuvo el power-up
    int puntos;
} Jugador;

typedef struct {
    float x, y;
    int active;
} Orbe_Vida;

typedef struct {
    float x, y;
    int active;
} Orbe_Velocidad;

typedef struct {
    SDL_Texture* Textura_Jugador;
    SDL_Texture* enemy;
    SDL_Texture* asteroid;
    SDL_Texture* bullet;
    SDL_Texture* enemyBullet;
    SDL_Texture* healthOrb;
    SDL_Texture* fireRateOrb;
    SDL_Texture* background;
    SDL_Texture* healthIcon; 
    SDL_Texture* shieldIcon;   
} GameTextures;

// Función para cargar una textura
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        printf("Error al cargar imagen %s: %s\n", path, IMG_GetError());
        return NULL;
    }
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        printf("Error al crear textura de %s: %s\n", path, SDL_GetError());
    }
    
    SDL_FreeSurface(surface);
    return texture;
}
// Función para cargar todas las texturas
	GameTextures loadGameTextures(SDL_Renderer* renderer) {
    GameTextures textures;
    
    textures.Textura_Jugador = loadTexture(renderer, "assets/NA.png");
    textures.enemy = loadTexture(renderer, "assets/NE.png");
    textures.asteroid = loadTexture(renderer, "assets/meteorito.png");
    textures.bullet = loadTexture(renderer, "assets/laserA.png");
    textures.enemyBullet = loadTexture(renderer, "assets/laserE.png");
    textures.healthOrb = loadTexture(renderer, "assets/OA.png");
    textures.fireRateOrb = loadTexture(renderer, "assets/ON.png");
    textures.background = loadTexture(renderer, "assets/BACK.png");
    textures.healthIcon = loadTexture(renderer, "assets/vida.png"); 
    textures.shieldIcon = loadTexture(renderer, "assets/escudo.png");
    
    return textures;
}    
    
int VerifColosion(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}


// Función para liberar texturas
void freeGameTextures(GameTextures* textures) {
    SDL_DestroyTexture(textures->Textura_Jugador);
    SDL_DestroyTexture(textures->enemy);
    SDL_DestroyTexture(textures->asteroid);
    SDL_DestroyTexture(textures->bullet);
    SDL_DestroyTexture(textures->enemyBullet);
    SDL_DestroyTexture(textures->healthOrb);
    SDL_DestroyTexture(textures->fireRateOrb);
    SDL_DestroyTexture(textures->background);
    SDL_DestroyTexture(textures->healthIcon);    
    SDL_DestroyTexture(textures->shieldIcon); 
}


void dibujoBarraVida(SDL_Renderer *renderer, float vida, GameTextures *textures) {
    const int ICON_SIZE = 35;           
    const int BAR_WIDTH = 200;          
    const int BAR_HEIGHT = 20;          
    const int MARGIN = 5;               
    const int START_X = 10 + ICON_SIZE + MARGIN;  
    const int START_Y = 10;             
    
    //dibujo barra de vida
    SDL_Rect healthIconRect = {10, START_Y, ICON_SIZE, ICON_SIZE};
    SDL_RenderCopy(renderer, textures->healthIcon, NULL, &healthIconRect);
    
    // Fondo de la barra de vida 
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_Rect backgroundBar = {START_X, START_Y + 2, BAR_WIDTH, BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &backgroundBar);
    
    // Barra de vida 
    SDL_SetRenderDrawColor(renderer, 255,0, 0, 255);
    int healthWidth = (int)(BAR_WIDTH * fminf(vida, VIDA_INICIAL) / VIDA_INICIAL);
    SDL_Rect healthBar = {START_X, START_Y + 2, healthWidth, BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &healthBar);
     
	 // Si hay escudo (vida extra)
    if (vida > VIDA_INICIAL) {
        // Dibujar ícono de escudo
        SDL_Rect shieldIconRect = {10, START_Y + ICON_SIZE + MARGIN, ICON_SIZE, ICON_SIZE};
        SDL_RenderCopy(renderer, textures->shieldIcon, NULL, &shieldIconRect);
        
        // Fondo de la barra de escudo
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_Rect shieldBackgroundBar = {START_X, START_Y + ICON_SIZE + MARGIN + 2, BAR_WIDTH, BAR_HEIGHT};
        SDL_RenderFillRect(renderer, &shieldBackgroundBar);
        
        // Barra de escudo (azul brillante)
        SDL_SetRenderDrawColor(renderer, 30, 144, 255, 255);
        float shieldPercentage = (vida - VIDA_INICIAL) / (ESCUDO_MAX - VIDA_INICIAL);
        int shieldWidth = (int)(BAR_WIDTH * shieldPercentage);
        SDL_Rect shieldBar = {START_X, START_Y + ICON_SIZE + MARGIN + 2, shieldWidth, BAR_HEIGHT};
        SDL_RenderFillRect(renderer, &shieldBar);
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect healthBorder = {START_X, START_Y + 2, BAR_WIDTH, BAR_HEIGHT};
    SDL_RenderDrawRect(renderer, &healthBorder);
    
    if (vida > VIDA_INICIAL) {
        SDL_Rect shieldBorder = {START_X, START_Y + ICON_SIZE + MARGIN + 2, BAR_WIDTH, BAR_HEIGHT};
        SDL_RenderDrawRect(renderer, &shieldBorder);
    }
}

void drawPowerUpTimer(SDL_Renderer *renderer, Uint32 startTime) {
    const int TIMER_Ancho = 100;
    const int TIMER_Largo = 10;
    const int TIMER_X = ANCHO_PANTALLA - TIMER_Ancho - 10;
    const int TIMER_Y = 40;
    
    // Calcular tiempo restante (10 segundos máximo)
    Uint32 currentTime = SDL_GetTicks();
    float timeLeft = 10.0f - (currentTime - startTime) / 1000.0f;
    float percentage = timeLeft / 10.0f;
    
    if (percentage > 0) {
        // Fondo del temporizador
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_Rect timerBackground = {TIMER_X, TIMER_Y, TIMER_Ancho, TIMER_Largo};
        SDL_RenderFillRect(renderer, &timerBackground);
        
        // Barra de tiempo restante
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
        int timerAncho = (int)(TIMER_Ancho * percentage);
        SDL_Rect timerBar = {TIMER_X, TIMER_Y, timerAncho, TIMER_Largo};
        SDL_RenderFillRect(renderer, &timerBar);
    }
}

void dibujoPuntos(SDL_Renderer *renderer, int puntos) {
    char puntosText[32];
    snprintf(puntosText, sizeof(puntosText), "Puntos: %d", puntos);
    
    SDL_Color textColor = {255, 255, 255, 255}; // Color blanco
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, puntosText, textColor);
    
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        
        if (textTexture) {
            SDL_Rect textRect = {
                ANCHO_PANTALLA - textSurface->w - 20, 
                10,
                textSurface->w,
                textSurface->h
            };
            
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        
        SDL_FreeSurface(textSurface);
    }
}


// Función para mostrar las instrucciones
void mostrarInstrucciones() {
    system("clear");  
    printf("\n=== INSTRUCCIONES DEL JUEGO ===\n\n");
    printf("Controles:\n");
    printf("- W: Mover nave hacia arriba\n");
    printf("- S: Mover nave hacia abajo\n");
    printf("- A: Mover nave hacia la izquierda\n");
    printf("- D: Mover nave hacia la derecha\n");
    printf("- ESPACIO: Disparar\n\n");
    
    printf("Objetivos:\n");
    printf("- Destruye asteroides (+1 punto)\n");
    printf("- Destruye naves enemigas (+3 puntos)\n");
    printf("- Recoge orbes azules para recuperar vida\n");
    printf("- Recoge orbes naranjas para aumentar la velocidad de disparo\n\n");
    
    printf("Advertencias:\n");
    printf("- Evita chocar con asteroides\n");
    printf("- Esquiva los disparos enemigos\n");
    printf("- No dejes que tu vida llegue a 0\n\n");
    
    printf("Presiona ENTER para volver al menú...");
    getchar();
}

int mostrarMenu() {
    int opcion = 0;
    
    while (opcion < 1 || opcion > 3) {
        system("clear");  
        printf("\n=== SPACE SHOOTER ===\n\n");
        printf("1. Iniciar juego\n");
        printf("2. Ver instrucciones\n");
        printf("3. Salir\n\n");
        printf("Selecciona una opción (1-3): ");
        
        if (scanf("%d", &opcion) != 1) {
            while (getchar() != '\n');  // Limpiar el buffer
            opcion = 0;
            continue;
        }
        
        if (opcion < 1 || opcion > 3) {
            printf("\nOpción inválida. Presiona ENTER para continuar...");
            while (getchar() != '\n');  // Limpiar el buffer
            getchar();
        }
    }
    
    return opcion;
}



int iniciarJuego() {
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Error al iniciar SDL: %s\n", SDL_GetError());
        return 1;
    }
	
	// Inicializar SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        printf("Error al iniciar SDL_image: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }
	
    SDL_Window *window = SDL_CreateWindow(
        "Space Shooter - Texture Edition",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        ANCHO_PANTALLA, LARGO_PANTALLA,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        printf("Error al crear la ventana: %s\n", SDL_GetError());
        IMG_Quit();
        SDL_Quit();
        return 1;
    }
	
	//Inicializar SDL_tff
	if (TTF_Init() < 0) {
        printf("Error al iniciar SDL_ttf: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Cargar la fuente después de inicializar TTF
    font = TTF_OpenFont("assets/font.ttf", 24);
    if (!font) {
        printf("Error al cargar la fuente: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // Cargar texturas
    GameTextures textures = loadGameTextures(renderer);

    int quit = 0;
    SDL_Event event;

    Jugador Jugador = {
        .x = ANCHO_PANTALLA / 2,
        .y = LARGO_PANTALLA - 50,
        .vida = VIDA_INICIAL,
        .fireRateBoosted = 0,
        .fireRateBoostStartTime = 0,
        .puntos = 0
    };
    
    int BalasCooldown = 0;

    Balas balas[MAX_BALAS];
    for (int i = 0; i < MAX_BALAS; i++) {
        balas[i].active = 0;
    }

    EnemigoBalas Enemigobalas[MAX_BALAS_ENEMIGOS];
    for (int i = 0; i < MAX_BALAS_ENEMIGOS; i++) {
        Enemigobalas[i].active = 0;
    }

    Asteroide asteroides[MAX_ASTEROIDES];
    for (int i = 0; i < MAX_ASTEROIDES; i++) {
        asteroides[i].active = 0;
    }

    Enemigo Enemigos[MAX_ENEMIGOS];
    for (int i = 0; i < MAX_ENEMIGOS; i++) {
        Enemigos[i].active = 0;
        Enemigos[i].BalasCooldown = rand() % ENEMIGOS_BALAS_COOLDOWN;
    }

    Orbe_Vida Orbe_Vidas[MAX_ORBS];
    for (int i = 0; i < MAX_ORBS; i++) {
        Orbe_Vidas[i].active = 0;
    }

    Orbe_Velocidad VelocidadOrbes[MAX_FIRE_ORBS];
    for (int i = 0; i < MAX_FIRE_ORBS; i++) {
        VelocidadOrbes[i].active = 0;
    }

    int keys[SDL_NUM_SCANCODES] = {0};
    Uint32 startTime, frameTime;
    
    srand(time(NULL));

    while (!quit) {
        startTime = SDL_GetTicks();

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            } else if (event.type == SDL_KEYDOWN) {
                keys[event.key.keysym.scancode] = 1;
            } else if (event.type == SDL_KEYUP) {
                keys[event.key.keysym.scancode] = 0;
            }
        }

        // Verificar si el power-up de velocidad de disparo ha expirado
        if (Jugador.fireRateBoosted) {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - Jugador.fireRateBoostStartTime >= 10000) { // 10 segundos
                Jugador.fireRateBoosted = 0;
            }
        }

        // Movimiento del jugador
        if (keys[SDL_SCANCODE_A]) Jugador.x -= VELOCIDAD_NAVE;
        if (keys[SDL_SCANCODE_D]) Jugador.x += VELOCIDAD_NAVE;
        if (keys[SDL_SCANCODE_W]) Jugador.y -= VELOCIDAD_NAVE;
        if (keys[SDL_SCANCODE_S]) Jugador.y += VELOCIDAD_NAVE;

        // Disparos del jugador
        if (keys[SDL_SCANCODE_SPACE] && BalasCooldown == 0) {
            for (int i = 0; i < MAX_BALAS; i++) {
                if (!balas[i].active) {
                    balas[i].x = Jugador.x + ANCHO_NAVE / 2 - ANCHO_BALAS / 2;
                    balas[i].y = Jugador.y;
                    balas[i].active = 1;
                    balas[i].speed = VELOCIDAD_BALAS;
                    BalasCooldown = Jugador.fireRateBoosted ? BOOSTED_BALAS_COOLDOWN : BALAS_COOLDOWN;
                    break;
                }
            }
        }

        if (BalasCooldown > 0) BalasCooldown--;

        // Límites de la pantalla para la nave
        if (Jugador.x < 0) Jugador.x = 0;
        if (Jugador.x + ANCHO_NAVE > ANCHO_PANTALLA) Jugador.x = ANCHO_PANTALLA - ANCHO_NAVE;
        if (Jugador.y < 0) Jugador.y = 0;
        if (Jugador.y + LARGO_NAVE > LARGO_PANTALLA) Jugador.y = LARGO_PANTALLA - LARGO_NAVE;

        // Actualizar balas del jugador
        for (int i = 0; i < MAX_BALAS; i++) {
            if (balas[i].active) {
                balas[i].y -= balas[i].speed;
                if (balas[i].y + LARGO_BALAS < 0) {
                    balas[i].active = 0;
                }
            }
        }

        // Actualizar balas enemigas
        for (int i = 0; i < MAX_BALAS_ENEMIGOS; i++) {
            if (Enemigobalas[i].active) {
                Enemigobalas[i].y += Enemigobalas[i].speed;
                if (Enemigobalas[i].y > LARGO_PANTALLA) {
                    Enemigobalas[i].active = 0;
                }
                if (VerifColosion(Jugador.x, Jugador.y, ANCHO_NAVE, LARGO_NAVE,
                                 Enemigobalas[i].x, Enemigobalas[i].y,
                                 ANCHO_BALAS, LARGO_BALAS)) {
                    Jugador.vida -= DANIO_BALA;
                    Enemigobalas[i].active = 0;
                    if (Jugador.vida <= 0) {
                        printf("¡Game Over! Puntuación final: %d\n", Jugador.puntos);
                        quit = 1;
                    }
                }
            }
        }

        // Actualizar asteroides
        int activeAsteroides = 0;
        for (int i = 0; i < MAX_ASTEROIDES; i++) {
            if (asteroides[i].active) activeAsteroides++;
        }
        if (activeAsteroides < MAX_ASTEROIDES) {
            for (int i = 0; i < MAX_ASTEROIDES; i++) {
                if (!asteroides[i].active) {
                    asteroides[i].size = rand() % 30 + 20;
                    asteroides[i].x = rand() % (ANCHO_PANTALLA - asteroides[i].size);
                    asteroides[i].y = -asteroides[i].size;
                    asteroides[i].active = 1;
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_ASTEROIDES; i++) {
            if (asteroides[i].active) {
                asteroides[i].y += VELOCIDAD_ASTEROIDES;
                if (asteroides[i].y > LARGO_PANTALLA) {
                    asteroides[i].active = 0;
                }
                // Colisión con jugador
                if (VerifColosion(Jugador.x, Jugador.y, ANCHO_NAVE, LARGO_NAVE,
                                 asteroides[i].x, asteroides[i].y,
                                 asteroides[i].size, asteroides[i].size)) {
                    Jugador.vida -= DANIO_ASTEROIDES;
                    asteroides[i].active = 0;
                    if (Jugador.vida <= 0) {
                        printf("¡Game Over! Puntuación final: %d\n", Jugador.puntos);
                        quit = 1;
                    }
                }
                // Colisión con balas del jugador
                for (int j = 0; j < MAX_BALAS; j++) {
                    if (balas[j].active && VerifColosion(balas[j].x, balas[j].y,
                                                          ANCHO_BALAS, LARGO_BALAS,
                                                          asteroides[i].x, asteroides[i].y,
                                                          asteroides[i].size, asteroides[i].size)) {
                        balas[j].active = 0;
                        asteroides[i].active = 0;
                        Jugador.puntos += PUNTOS_ASTEROIDES;
                        printf("¡Asteroide destruido! +%d puntos (Total: %d)\n", 
                               PUNTOS_ASTEROIDES, Jugador.puntos);
                        break;
                    }
                }
            }
        }
        // Actualizar enemigos
        int activeEnemigos = 0;
        for (int i = 0; i < MAX_ENEMIGOS; i++) {
            if (Enemigos[i].active) activeEnemigos++;
        }

        if (activeEnemigos < MAX_ENEMIGOS) {
            for (int i = 0; i < MAX_ENEMIGOS; i++) {
                if (!Enemigos[i].active) {
                    Enemigos[i].x = rand() % (ANCHO_PANTALLA - ANCHO_ENEMIGOS);
                    Enemigos[i].y = -LARGO_ENEMIGOS;
                    Enemigos[i].active = 1;
      		    Enemigos[i].BalasCooldown = rand() % ENEMIGOS_BALAS_COOLDOWN;
                    break;
                }
            }
        }
        for (int i = 0; i < MAX_ENEMIGOS; i++) {
            if (Enemigos[i].active) {
                Enemigos[i].y += VELOCIDAD_ENEMIGOS;
                if (Enemigos[i].y > LARGO_PANTALLA) {
                    Enemigos[i].active = 0;
                }
                // Movimiento lateral de los enemigos
                Enemigos[i].x += sinf(Enemigos[i].y * 0.05f) * 2;
                if (Enemigos[i].x < 0) Enemigos[i].x = 0;
                if (Enemigos[i].x + ANCHO_ENEMIGOS > ANCHO_PANTALLA) Enemigos[i].x = ANCHO_PANTALLA - ANCHO_ENEMIGOS;
                // Disparos enemigos
                if (--Enemigos[i].BalasCooldown <= 0) {
                    for (int j = 0; j < MAX_BALAS_ENEMIGOS; j++) {
                        if (!Enemigobalas[j].active) {
                            Enemigobalas[j].x = Enemigos[i].x + ANCHO_ENEMIGOS / 2;
                            Enemigobalas[j].y = Enemigos[i].y + LARGO_ENEMIGOS;
                            Enemigobalas[j].active = 1;
                            Enemigobalas[j].speed = ENEMIGO_VELOCIDAD_BALAS;
                            Enemigos[i].BalasCooldown = ENEMIGOS_BALAS_COOLDOWN;
                            break;
                        }
                    }
                }
                // Colisión con jugador
                if (VerifColosion(Jugador.x, Jugador.y, ANCHO_NAVE, LARGO_NAVE,
                                 Enemigos[i].x, Enemigos[i].y, ANCHO_ENEMIGOS, LARGO_ENEMIGOS)) {
                    Jugador.vida -= DANIO_COLISION_ENEMIGO;
                    Enemigos[i].active = 0;
                    if (Jugador.vida <= 0) {
                        printf("¡Game Over! Puntuación final: %d\n", Jugador.puntos);
                        quit = 1;
                    }
                }
                // Colisión con balas del jugador
                for (int j = 0; j < MAX_BALAS; j++) {
                    if (balas[j].active && VerifColosion(balas[j].x, balas[j].y,
                                                          ANCHO_BALAS, LARGO_BALAS,
                                                          Enemigos[i].x, Enemigos[i].y,
                                                          ANCHO_ENEMIGOS, LARGO_ENEMIGOS)) {
                        balas[j].active = 0;
                        Enemigos[i].active = 0;
                        Jugador.puntos += PUNTOS_ENEMIGOS;
                        printf("¡Nave enemiga destruida! +%d puntos (Total: %d)\n", 
                               PUNTOS_ENEMIGOS, Jugador.puntos);
                        break;
                    }
                }
            }
        }
        // Generar orbes de vida
        if (rand() % 1000 < 2) { // 0.2% de probabilidad por frame
            for (int i = 0; i < MAX_ORBS; i++) {
                if (!Orbe_Vidas[i].active) {
                    Orbe_Vidas[i].x = rand() % (ANCHO_PANTALLA - TAMANIO_ORB);
                    Orbe_Vidas[i].y = -TAMANIO_ORB;
                    Orbe_Vidas[i].active = 1;
                    break;
                }
            }
        }
        // Actualizar orbes de vida
        for (int i = 0; i < MAX_ORBS; i++) {
            if (Orbe_Vidas[i].active) {
                Orbe_Vidas[i].y += ORB_VELOCIDAD;
                if (Orbe_Vidas[i].y > LARGO_PANTALLA) {
                    Orbe_Vidas[i].active = 0;
                }
                // Colisión con jugador
                if (VerifColosion(Jugador.x, Jugador.y, ANCHO_NAVE, LARGO_NAVE,
                                 Orbe_Vidas[i].x, Orbe_Vidas[i].y, TAMANIO_ORB, TAMANIO_ORB)) {
                    Jugador.vida = fminf(Jugador.vida + ORB_VIDA_HEAL, ESCUDO_MAX);
                    Orbe_Vidas[i].active = 0;
                }
            }
        }
        // Generar orbes de velocidad de disparo
        if (rand() % 1000 < 1) { // 0.1% de probabilidad por frame
            for (int i = 0; i < MAX_FIRE_ORBS; i++) {
                if (!VelocidadOrbes[i].active) {
                    VelocidadOrbes[i].x = rand() % (ANCHO_PANTALLA - FIRE_TAMANIO_ORB);
                    VelocidadOrbes[i].y = -FIRE_TAMANIO_ORB;
                    VelocidadOrbes[i].active = 1;
                    break;
                }
            }
        }

        /* Actualizar orbes de velocidad de disparo*/
        for (int i = 0; i < MAX_FIRE_ORBS; i++) {
    		if (VelocidadOrbes[i].active) {
        		VelocidadOrbes[i].y += FIRE_ORB_VELOCIDAD;
        		if (VelocidadOrbes[i].y > LARGO_PANTALLA) {
            		VelocidadOrbes[i].active = 0;
        }

                /* Colisión con jugador*/
                if (VerifColosion(Jugador.x, Jugador.y, ANCHO_NAVE, LARGO_NAVE,
                         VelocidadOrbes[i].x, VelocidadOrbes[i].y, FIRE_TAMANIO_ORB, FIRE_TAMANIO_ORB)) {
            		Jugador.fireRateBoosted = 1;
            		Jugador.fireRateBoostStartTime = SDL_GetTicks(); 
            		VelocidadOrbes[i].active = 0;
                }
            }
        }
        // Renderizado
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
		
		// Dibujar fondo
        SDL_RenderCopy(renderer, textures.background, NULL, NULL);
		
        // Dibujar nave del jugador
			SDL_Rect Textura_JugadorRect = {(int)Jugador.x, (int)Jugador.y, ANCHO_NAVE, LARGO_NAVE};
        	SDL_RenderCopy(renderer, textures.Textura_Jugador, NULL, &Textura_JugadorRect);
        	
        	
        // Dibujar balas del jugador
        for (int i = 0; i < MAX_BALAS; i++) {
            if (balas[i].active) {
                SDL_Rect bulletRect = {balas[i].x, balas[i].y, ANCHO_BALAS, LARGO_BALAS};
                SDL_RenderCopy(renderer, textures.bullet, NULL, &bulletRect);
            }
        }

        // Dibujar balas enemigas
        for (int i = 0; i < MAX_BALAS_ENEMIGOS; i++) {
            if (Enemigobalas[i].active) {
                SDL_Rect enemyBulletRect = {Enemigobalas[i].x, Enemigobalas[i].y, 
                                          ANCHO_BALAS, LARGO_BALAS};
                SDL_RenderCopy(renderer, textures.enemyBullet, NULL, &enemyBulletRect);
            }
        }

        // Dibujar asteroides
        for (int i = 0; i < MAX_ASTEROIDES; i++) {
            if (asteroides[i].active) {
                SDL_Rect asteroidRect = {asteroides[i].x, asteroides[i].y,
                                       asteroides[i].size, asteroides[i].size};
                SDL_RenderCopy(renderer, textures.asteroid, NULL, &asteroidRect);
            }
        }


        // Dibujar enemigos
        for (int i = 0; i < MAX_ENEMIGOS; i++) {
            if (Enemigos[i].active) {
                SDL_Rect enemyRect = {Enemigos[i].x, Enemigos[i].y, 
                                    ANCHO_ENEMIGOS, LARGO_ENEMIGOS};
                SDL_RenderCopy(renderer, textures.enemy, NULL, &enemyRect);
            }
        }

        // Dibujar orbes de vida
        for (int i = 0; i < MAX_ORBS; i++) {
            if (Orbe_Vidas[i].active) {
                SDL_Rect healthOrbRect = {Orbe_Vidas[i].x, Orbe_Vidas[i].y, 
                                        TAMANIO_ORB, TAMANIO_ORB};
                SDL_RenderCopy(renderer, textures.healthOrb, NULL, &healthOrbRect);
            }
        }

        // Dibujar orbes de velocidad de disparo
        for (int i = 0; i < MAX_FIRE_ORBS; i++) {
            if (VelocidadOrbes[i].active) {
                SDL_Rect fireOrbRect = {VelocidadOrbes[i].x, VelocidadOrbes[i].y, 
                                      FIRE_TAMANIO_ORB, FIRE_TAMANIO_ORB};
                SDL_RenderCopy(renderer, textures.fireRateOrb, NULL, &fireOrbRect);
            }
        }

        // Dibujar barra de vida y puntuación
		dibujoBarraVida(renderer, Jugador.vida,&textures);
		dibujoPuntos(renderer, Jugador.puntos);
        SDL_RenderPresent(renderer);

        // Control de FPS
        frameTime = SDL_GetTicks() - startTime;
        if (frameTime < 16) {
            SDL_Delay(16 - frameTime);
        }
    }
    void dibujoPuntos(SDL_Renderer *renderer, int puntos) {
    char puntosText[32];
    snprintf(puntosText, sizeof(puntosText), "Puntos: %d", puntos);
    
    SDL_Color textColor = {255, 255, 255, 255}; 
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, puntosText, textColor);
    
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        
        if (textTexture) {
            SDL_Rect textRect = {
                ANCHO_PANTALLA - textSurface->w - 20, 
                10, 
                textSurface->w,
                textSurface->h
            };
            
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        
        SDL_FreeSurface(textSurface);
    }
}
	if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
    freeGameTextures(&textures);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}

int main(int argc, char *argv[]) {
    int ejecutando = 1;
    
    while (ejecutando) {
        int opcion = mostrarMenu();
        switch (opcion) {
            case 1:
                printf("\nIniciando juego...\n");
                SDL_Delay(1000);  // Pequeña pausa antes de iniciar
                iniciarJuego();
                break;
            case 2:
                mostrarInstrucciones();
                break;
            case 3:
                printf("\n¡Gracias por jugar!\n");
                ejecutando = 0;
                break;
        }
    }
    return 0;
}
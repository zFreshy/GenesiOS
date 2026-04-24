/*
 * nocsd.c — Genesi OS CSD Suppressor
 * ====================================
 * Biblioteca LD_PRELOAD que intercepta chamadas de CSD em apps teimosos,
 * mentindo pro apps dizendo: "você não tem permissão de desenhar borda".
 *
 * Intercepta:
 *   • gtk_window_set_decorated  → força FALSE  (sem frame GTK)
 *   • gtk_window_set_titlebar   → força NULL   (sem headerbar customizada)
 *   • gtk_header_bar_new        → retorna NULL (mata headerbar do GTK4)
 *   • libdecor_new              → retorna NULL (mata o decorador do Firefox no Wayland)
 *   • libdecor_frame_set_title  → no-op silencioso
 *   • secure_getenv(GTK_CSD)    → retorna "0"
 *   • secure_getenv(MOZ_GTK_TITLEBAR_DECORATION) → "system"
 *   • getenv                    → intercepta também (fallback)
 *
 * Compilar:
 *   cc -shared -fPIC -ldl -o /tmp/genesi_nocsd.so nocsd.c
 *
 * Usar:
 *   LD_PRELOAD=/tmp/genesi_nocsd.so firefox
 *   LD_PRELOAD=/tmp/genesi_nocsd.so gedit
 */

#define _GNU_SOURCE
#include <dlfcn.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

/* ============================================================ */
/*  GTK 3 / GTK 4 — Decorações de janela                       */
/* ============================================================ */

/*
 * gtk_window_set_decorated(window, TRUE) sempre vira FALSE.
 * GTK usa esta função para habilitar o frame CSD da janela.
 * Ao forçar FALSE, o GTK para de desenhar borda própria.
 */
void gtk_window_set_decorated(void *window, int setting)
{
    typedef void (*fn_t)(void *, int);
    static fn_t real = NULL;
    if (!real) real = (fn_t)dlsym(RTLD_NEXT, "gtk_window_set_decorated");
    if (real) real(window, 0);  /* 0 = sem decoração */
}

/*
 * gtk_window_set_titlebar(window, widget) sempre recebe NULL.
 * GTK usa esta função para instalar a HeaderBar CSD customizada.
 * Ao passar NULL, remove a barra customizada.
 */
void gtk_window_set_titlebar(void *window, void *titlebar)
{
    typedef void (*fn_t)(void *, void *);
    static fn_t real = NULL;
    if (!real) real = (fn_t)dlsym(RTLD_NEXT, "gtk_window_set_titlebar");
    if (real) real(window, NULL);  /* NULL = sem titlebar customizada */
}

/*
 * gtk_header_bar_new() retorna NULL.
 * GTK4 usa HeaderBar como titlebar padrão. Ao retornar NULL,
 * o app não consegue criar a barra customizada.
 */
void *gtk_header_bar_new(void)
{
    return NULL;  /* sem headerbar = sem CSD */
}

/*
 * gtk_window_get_titlebar() retorna NULL.
 * Alguns apps checam se já existe titlebar antes de criar.
 * Mentimos dizendo que não existe.
 */
void *gtk_window_get_titlebar(void *window)
{
    (void)window;
    return NULL;
}

/* ============================================================ */
/*  libdecor — Biblioteca Wayland CSD usada pelo Firefox        */
/* ============================================================ */

/*
 * libdecor_new() retorna NULL.
 *
 * libdecor é a biblioteca que o Firefox (e alguns apps GTK4) usa para
 * desenhar decorações CSD no Wayland. Quando libdecor_new() retorna NULL,
 * o cliente fica sem decorador e depende 100% do compositor (nosso SSD).
 *
 * Isso é o "veneno" principal contra o Firefox CSD.
 */
void *libdecor_new(void *display, const void *iface)
{
    (void)display;
    (void)iface;
    fprintf(stderr, "[nocsd] libdecor_new() bloqueado - forçando SSD\n");
    return NULL;  /* sem decorador = sem CSD */
}

/*
 * Funções auxiliares do libdecor — todas viram no-op silenciosas.
 * Necessário pois apps podem chamar estas funções mesmo após libdecor_new falhar.
 */
void libdecor_frame_set_title(void *frame, const char *title)
{
    (void)frame;
    (void)title;
}

void libdecor_frame_set_app_id(void *frame, const char *app_id)
{
    (void)frame;
    (void)app_id;
}

void libdecor_frame_commit(void *frame, void *state, void *configuration)
{
    (void)frame;
    (void)state;
    (void)configuration;
}

void libdecor_frame_unref(void *frame)
{
    (void)frame;
}

void libdecor_unref(void *context)
{
    (void)context;
}

/* ============================================================ */
/*  secure_getenv — GTK usa para ler variáveis de ambiente      */
/* ============================================================ */

/*
 * GTK chama secure_getenv("GTK_CSD") para decidir se usa CSD.
 * Interceptamos e retornamos "0" sempre, independentemente do ambiente.
 *
 * Nota: usamos secure_getenv (não getenv) porque GTK prefere a versão
 * segura que respeita setuid. Isso garante que nossa interceptação
 * funciona mesmo em processos com privilégios.
 */
char *secure_getenv(const char *name)
{
    typedef char *(*fn_t)(const char *);
    static fn_t real = NULL;
    if (!real) real = (fn_t)dlsym(RTLD_NEXT, "secure_getenv");

    if (name) {
        /* GTK_CSD=0 → desativa CSD em todos os apps GTK3/GTK4 */
        if (__builtin_strcmp(name, "GTK_CSD") == 0)
            return "0";
        /* MOZ_GTK_TITLEBAR_DECORATION=system → Firefox usa deco do sistema */
        if (__builtin_strcmp(name, "MOZ_GTK_TITLEBAR_DECORATION") == 0)
            return "system";
        /* GDK_BACKEND=wayland → força Wayland */
        if (__builtin_strcmp(name, "GDK_BACKEND") == 0)
            return "wayland";
    }

    return real ? real(name) : NULL;
}

/*
 * getenv — Fallback para apps que não usam secure_getenv
 */
char *getenv(const char *name)
{
    typedef char *(*fn_t)(const char *);
    static fn_t real = NULL;
    if (!real) real = (fn_t)dlsym(RTLD_NEXT, "getenv");

    if (name) {
        if (__builtin_strcmp(name, "GTK_CSD") == 0)
            return "0";
        if (__builtin_strcmp(name, "MOZ_GTK_TITLEBAR_DECORATION") == 0)
            return "system";
        if (__builtin_strcmp(name, "GDK_BACKEND") == 0)
            return "wayland";
    }

    return real ? real(name) : NULL;
}

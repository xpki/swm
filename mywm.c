/* mywm.c - Minimal X11 window manager */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

/* Arg and Key definitions must go before including config.h */
typedef struct {
    const void *v;
} Arg;

void spawn(const void *arg);
void quit(const void *arg);

typedef struct {
    unsigned int mod;
    KeySym keysym;
    void (*func)(const void *arg);
    const void *arg;
} Key;

#include "config.h"

typedef struct Client {
    Window win;
    struct Client *next;
} Client;

Display *dpy;
Window root;
Client *clients = NULL;

void spawn(const void *arg) {
    const Arg *a = arg;
    if (fork() == 0) {
        if (dpy) close(ConnectionNumber(dpy));
        setsid();
        execvp(((char **)a->v)[0], (char **)a->v);
        fprintf(stderr, "mywm: execvp failed\n");
        exit(1);
    }
}

void quit(const void *arg) {
    XCloseDisplay(dpy);
    exit(0);
}

void arrange(void) {
    int x = 0, y = 0;
    int w = DisplayWidth(dpy, DefaultScreen(dpy));
    int h = DisplayHeight(dpy, DefaultScreen(dpy));
    int count = 0;

    for (Client *c = clients; c; c = c->next) count++;
    if (count == 0) return;

    int i = 0;
    for (Client *c = clients; c; c = c->next, i++) {
        int ch = h / count;
        XMoveResizeWindow(dpy, c->win, x, y + i * ch, w, ch - borderpx);
    }
}

void manage(Window w) {
    Client *c = malloc(sizeof(Client));
    if (!c) return;
    c->win = w;
    c->next = clients;
    clients = c;

    XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask);
    XMapWindow(dpy, w);
    arrange();
}

void unmanage(Window w) {
    Client **pc = &clients;
    while (*pc) {
        if ((*pc)->win == w) {
            Client *tmp = *pc;
            *pc = tmp->next;
            free(tmp);
            break;
        }
        pc = &(*pc)->next;
    }
    arrange();
}

void keypress(XKeyEvent *e) {
    KeySym keysym = XLookupKeysym(e, 0);
    for (unsigned int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        if (keysym == keys[i].keysym && (e->state & keys[i].mod)) {
            keys[i].func(keys[i].arg);
        }
    }
}

int main(void) {
    XEvent ev;

    dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "mywm: cannot open display\n");
        exit(1);
    }

    root = DefaultRootWindow(dpy);
    XSelectInput(dpy, root, SubstructureRedirectMask | SubstructureNotifyMask);

    for (unsigned int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++) {
        KeyCode code = XKeysymToKeycode(dpy, keys[i].keysym);
        XGrabKey(dpy, code, keys[i].mod, root, True, GrabModeAsync, GrabModeAsync);
    }

    XSync(dpy, False);

    while (!XNextEvent(dpy, &ev)) {
        switch (ev.type) {
        case MapRequest:
            manage(ev.xmaprequest.window);
            break;
        case UnmapNotify:
            unmanage(ev.xunmap.window);
            break;
        case KeyPress:
            keypress(&ev.xkey);
            break;
        }
    }

    XCloseDisplay(dpy);
    return 0;
}

/* mywm.c - Minimal X11 window manager with layout toggle, focus, workspaces */

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

typedef struct {
    const void *v;
} Arg;

void spawn(const void *arg);
void quit(const void *arg);
void toggle_layout(const void *arg);
void killclient(const void *arg);
void focusnext(const void *arg);
void focusprev(const void *arg);
void view(const void *arg);
void movetows(const void *arg);
void arrange(void);

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
    int workspace;
} Client;

Display *dpy;
Window root;
Client *clients = NULL;
Client *sel = NULL;

enum { LAYOUT_HORIZ = 0, LAYOUT_VERT } layout = LAYOUT_HORIZ;
int current_ws = 1;

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

void toggle_layout(const void *arg) {
    layout = (layout == LAYOUT_HORIZ) ? LAYOUT_VERT : LAYOUT_HORIZ;
    arrange();
}

void killclient(const void *arg) {
    Window focused;
    int revert;
    XGetInputFocus(dpy, &focused, &revert);
    if (focused != None && focused != PointerRoot && focused != root) {
        XKillClient(dpy, focused);
    }
}

void focusnext(const void *arg) {
    Client *start = sel;
    for (Client *c = sel ? sel->next : clients; c; c = c->next) {
        if (c->workspace == current_ws) {
            sel = c;
            XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
            return;
        }
    }
    for (Client *c = clients; c && c != start; c = c->next) {
        if (c->workspace == current_ws) {
            sel = c;
            XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
            return;
        }
    }
}

void focusprev(const void *arg) {
    Client *prev = NULL;
    for (Client *c = clients; c; c = c->next) {
        if (c == sel)
            break;
        if (c->workspace == current_ws)
            prev = c;
    }
    if (prev) {
        sel = prev;
        XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
    } else {
        Client *last = NULL;
        for (Client *c = clients; c; c = c->next)
            if (c->workspace == current_ws)
                last = c;
        if (last) {
            sel = last;
            XSetInputFocus(dpy, sel->win, RevertToPointerRoot, CurrentTime);
        }
    }
}

void view(const void *arg) {
    int ws = *(const int *)arg;
    if (ws != current_ws) {
        current_ws = ws;
        sel = NULL;
        arrange();
    }
}

void movetows(const void *arg) {
    if (!sel)
        return;

    int ws = *(const int *)arg;
    sel->workspace = ws;

    if (ws != current_ws) {
        XUnmapWindow(dpy, sel->win);
        sel = NULL;
    }

    arrange();
}

void arrange(void) {
    int x = 0, y = 0;
    int w = DisplayWidth(dpy, DefaultScreen(dpy));
    int h = DisplayHeight(dpy, DefaultScreen(dpy));
    int count = 0;

    for (Client *c = clients; c; c = c->next)
        if (c->workspace == current_ws)
            count++;

    if (count == 0)
        return;

    int i = 0;
    for (Client *c = clients; c; c = c->next) {
        if (c->workspace != current_ws)
            continue;

        if (layout == LAYOUT_HORIZ) {
            int ch = h / count;
            XMoveResizeWindow(dpy, c->win, x, y + i * ch, w, ch - borderpx);
        } else {
            int cw = w / count;
            XMoveResizeWindow(dpy, c->win, x + i * cw, y, cw - borderpx, h);
        }

        XMapWindow(dpy, c->win);
        i++;
    }

    for (Client *c = clients; c; c = c->next)
        if (c->workspace != current_ws)
            XUnmapWindow(dpy, c->win);

    XFlush(dpy);
}

void manage(Window w) {
    Client *c = malloc(sizeof(Client));
    if (!c) return;
    c->win = w;
    c->workspace = current_ws;
    c->next = clients;
    clients = c;

    sel = c;
    XSelectInput(dpy, w, EnterWindowMask | FocusChangeMask);
    XMapWindow(dpy, w);
    XSetInputFocus(dpy, w, RevertToPointerRoot, CurrentTime);
    arrange();
}

void unmanage(Window w) {
    Client **pc = &clients;
    while (*pc) {
        if ((*pc)->win == w) {
            Client *tmp = *pc;
            *pc = tmp->next;
            if (sel == tmp) sel = NULL;
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
        if (keysym == keys[i].keysym && (e->state & keys[i].mod) == keys[i].mod) {
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

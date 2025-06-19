/* config.h */
static const int ws1 = 1;
static const int ws2 = 2;
static const int ws3 = 3;
static const int ws4 = 4;
static const int ws5 = 5;
static const unsigned int borderpx = 2;
static const char *termcmd[] = { "st", NULL };
static const char *dmenucmd[] = { "dmenu_run", NULL };
#define MODKEY Mod1Mask  // Alt

static const Key keys[] = {
    { MODKEY, XK_Return, spawn, &(Arg){.v = termcmd} },
    { MODKEY, XK_p, spawn, &(Arg){.v = dmenucmd} },
    { MODKEY, XK_q, quit, NULL },
    { MODKEY, XK_t, toggle_layout, NULL },
    { MODKEY, XK_c, killclient, NULL },
    { MODKEY, XK_j, focusnext, NULL },
    { MODKEY, XK_k, focusprev, NULL },

    // Switch workspace
    { MODKEY,             XK_1, view,     &(Arg){.v = &ws1} },
    { MODKEY,             XK_2, view,     &(Arg){.v = &ws2} },
    { MODKEY,             XK_3, view,     &(Arg){.v = &ws3} },
    { MODKEY,             XK_4, view,     &(Arg){.v = &ws4} },
    { MODKEY,             XK_5, view,     &(Arg){.v = &ws5} },

    // Move focused window to workspace
    { MODKEY | ShiftMask, XK_1, movetows, &(Arg){.v = &ws1} },
    { MODKEY | ShiftMask, XK_2, movetows, &(Arg){.v = &ws2} },
    { MODKEY | ShiftMask, XK_3, movetows, &(Arg){.v = &ws3} },
    { MODKEY | ShiftMask, XK_4, movetows, &(Arg){.v = &ws4} },
    { MODKEY | ShiftMask, XK_5, movetows, &(Arg){.v = &ws5} },
};

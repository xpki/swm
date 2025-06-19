/* config.h */

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
};

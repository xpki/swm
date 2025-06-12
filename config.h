static const unsigned int borderpx = 2;
static const char *termcmd[] = { "xterm", NULL };
#define MODKEY Mod1Mask  // Alt

static const Key keys[] = {
    { MODKEY, XK_Return, spawn, &(Arg){.v = termcmd} },
    { MODKEY, XK_q, quit, NULL },
};

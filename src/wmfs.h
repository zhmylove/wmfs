
/*
*      wmfs.h
*      Copyright © 2008, 2009 Martin Duquesnoy <xorg62@gmail.com>
*      All rights reserved.
*
*      Redistribution and use in source and binary forms, with or without
*      modification, are permitted provided that the following conditions are
*      met:
*
*      * Redistributions of source code must retain the above copyright
*        notice, this list of conditions and the following disclaimer.
*      * Redistributions in binary form must reproduce the above
*        copyright notice, this list of conditions and the following disclaimer
*        in the documentation and/or other materials provided with the
*        distribution.
*      * Neither the name of the  nor the names of its
*        contributors may be used to endorse or promote products derived from
*        this software without specific prior written permission.
*
*      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WMFS_H
#define WMFS_H

/* Lib headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <getopt.h>
#include <dirent.h>
#include <err.h>
#include <pthread.h>
#include <locale.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

/* Optional dependencies */
#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif /* HAVE_XFT */

#ifdef HAVE_XINERAMA
#include <X11/extensions/Xinerama.h>
#endif /* HAVE_XINERAMA */

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif /* HAVE_XRANDR */

#ifdef HAVE_IMLIB
#include <Imlib2.h>
#endif /* HAVE_IMLIB */

/* Local headers */
#include "parse.h"
#include "structs.h"

/* MACRO */
#define ButtonMask   (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define MouseMask    (ButtonMask | PointerMotionMask)
#define KeyMask      (KeyPressMask | KeyReleaseMask)
#define SCREEN       DefaultScreen(dpy)
#define ROOT         RootWindow(dpy, SCREEN)
#define MAXH         DisplayHeight(dpy, DefaultScreen(dpy))
#define MAXW         DisplayWidth(dpy, DefaultScreen(dpy))
#define INFOBARH     ((conf.bars.height > 0) ? conf.bars.height : (font.height * 1.5))
#define FHINFOBAR    ((font.height - font.de) + (((int)INFOBARH - font.height) >> 1))
#define SHADH        (1)
#define BORDH        conf.client.borderheight
#define CBORDH       ((c->flags & HideBordersFlag) ? 0 : conf.client.borderheight)
#define TBARH        ((conf.titlebar.height < BORDH) ? BORDH : conf.titlebar.height)
#define CTBARH       ((conf.titlebar.height < CBORDH) ? CBORDH : conf.titlebar.height)
#define RESHW        (6 * (BORDH))
#define BUTTONWH     (TBARH >> 1)
#define DEF_CONF     ".config/wmfs/wmfsrc"
#define DEF_STATUS   ".config/wmfs/status.sh"
#define PAD          conf.pad
#define MAXSTATUS    (4096)

#define CWIN(win, parent, x, y, w, h, b, mask, col, at)                             \
    do {                                                                            \
        win = XCreateWindow(dpy, (parent), (x), (y), (w), (h), (b), CopyFromParent, \
        InputOutput, CopyFromParent, (mask), (at));                                 \
        XSetWindowBackground(dpy, win, (col));                                      \
    } while(/* CONSTCOND */ 0)

#define HANDLE_EVENT(e)    event_handle[(e)->type](e);
#define ATOM(a)            XInternAtom(dpy, (a), False)
#define FRAMEW(w)          ((w) + (BORDH << 1))
#define FRAMEH(h)          ((h) + (BORDH  + TBARH))
#define CFRAMEW(w)         ((w) + (CBORDH << 1))
#define CFRAMEH(h)         ((h) + (CBORDH  + TBARH))
#define CHECK(x)           if(!(x)) return
#define LEN(x)             (sizeof(x) / sizeof((x)[0]))
#define MAXCLIST           (64)
#define RPOS(x)            (x & 1 ? x - 1 : x + 1)
#define LDIR(x)            (x < Top)
#define FLAGAPPLY(v, b, f) ((b) ? (v |= (f)) : (v &= ~(f)))
#define INAREA(i, j, a)    ((i) >= (a).x && (i) <= (a).x + (a).width && (j) >= (a).y && (j) <= (a).y + (a).height)

/* Cfactor define */
#define CFACTOR_CHECK2(g1, g2, p) (LDIR(p) ? (g1.height == g2.height) : (g1.width == g2.width))
#define CFACTOR_PARENTROW(g1, g2, p)                                           \
     (LDIR(p)                                                                  \
      ? (p == Left ? (g1.x == g2.x) : (g1.x + g1.width  == g2.x + g2.width))   \
      : (p == Top  ? (g1.y == g2.y) : (g1.y + g1.height == g2.y + g2.height))) \

/* Barwin define, wrappers for simple function */
#define barwin_delete_subwin(bw) XDestroySubwindows(dpy, bw->win)
#define barwin_map_subwin(bw)    XMapSubwindows(dpy, bw->win)
#define barwin_unmap_subwin(bw)  XUnmapSubwindows(dpy, bw->win)
#define barwin_refresh(bw)       XCopyArea(dpy, bw->dr, bw->win, gc, 0, 0, bw->geo.width, bw->geo.height, 0, 0)
#define barwin_map(bw)              \
     do {                           \
          XMapWindow(dpy, bw->win); \
          bw->flags |= MappedFlag;  \
     } while(/* CONSTCOND */ 0)
#define barwin_unmap(bw)              \
     do {                             \
          XUnmapWindow(dpy, bw->win); \
          bw->flags &= ~MappedFlag;   \
     } while(/* CONSTCOND */ 0)

/* barwin.c */
BarWindow *barwin_create(Window parent,
                         int x, int y,
                         int w, int h,
                         uint bg, char*fg,
                         bool entermask,
                         bool stipple,
                         bool border);
void barwin_draw_text(BarWindow *bw, int x, int y, char *text);
void barwin_color_set(BarWindow *bw, uint bg, char *fg);
void barwin_delete(BarWindow *bw);
void barwin_move(BarWindow *bw, int x, int y);
void barwin_resize(BarWindow *bw, int w, int h);
void barwin_refresh_color(BarWindow *bw);

/* draw.c */
void draw_text(Drawable d, int x, int y, char* fg, char *str);
void draw_rectangle(Drawable dr, int x, int y, int w, int h, uint color);
void draw_graph(Drawable dr, int x, int y, int w, int h, uint color, char *data);

ushort textw(char *text);

/* infobar.c */
void infobar_init(void);
void infobar_draw_layout(InfoBar *i);
void _infobar_draw(InfoBar *i);
void infobar_draw(InfoBar *i);
void infobar_draw_selbar(InfoBar *i);
void infobar_draw_taglist(InfoBar *i);
void infobar_update_taglist(InfoBar *i);
void infobar_destroy(void);
void infobar_set_position(int pos);
void uicb_infobar_togglepos(uicb_t);
void uicb_infobar_toggledisplay(uicb_t);
void uicb_toggle_tagautohide(uicb_t);

/* cfactor.c */
void cfactor_clean(Client *c);
Geo cfactor_geo(Geo geo, int fact[4], int *err);
void cfactor_set(Client *c, Position p, int fac);
void cfactor_multi_set(Client *c, int fac[4]);
/* Generated with macro {{{ */
void uicb_client_resize_Right(uicb_t cmd);
void uicb_client_resize_Left(uicb_t cmd);
void uicb_client_resize_Top(uicb_t cmd);
void uicb_client_resize_Bottom(uicb_t cmd);
/* }}} */

/* client.c */
void client_attach(Client *c);
void client_configure(Client *c);
void client_detach(Client *c);
void client_focus(Client *c);
Client *client_get_next(void);
Client *client_get_prev(void);
/* client_gb_*() {{{ */
Client* client_gb_win(Window w);
Client* client_gb_frame(Window w);
Client* client_gb_titlebar(Window w);
Client* client_gb_resize(Window w);
Client* client_gb_button(Window w, int *n);
Client* client_gb_pos(Client *c, int x, int y);
/* }}} */
void client_get_name(Client *c);
void client_hide(Client *c);
void client_kill(Client *c);
bool ishide(Client *c, int screen);
void client_map(Client *c);
Client* client_manage(Window w, XWindowAttributes *wa, bool ar);
void client_geo_hints(Geo *geo, Client *c);
void client_moveresize(Client *c, Geo geo, bool r);
void client_maximize(Client *c);
void client_size_hints(Client *c);
void client_swap(Client *c1, Client *c2);
void client_raise(Client *c);
void client_unhide(Client *c);
void client_focus_next(Client *c);
void client_unmanage(Client *c);
void client_unmap(Client *c);
void client_update_attributes(Client *c);
void client_urgent(Client *c, bool u);
Client* client_get_next_with_direction(Client *bc, Position pos);
void uicb_client_raise(uicb_t);
/* Generated with macro {{{ */
void uicb_client_focus_next(uicb_t);
void uicb_client_focus_prev(uicb_t);
void uicb_client_swapsel_next(uicb_t);
void uicb_client_swapsel_prev(uicb_t);
void uicb_client_swapsel_Right(uicb_t);
void uicb_client_swapsel_Left(uicb_t);
void uicb_client_swapsel_Top(uicb_t);
void uicb_client_swapsel_Bottom(uicb_t);
void uicb_client_focus_Right(uicb_t cmd);
void uicb_client_focus_Left(uicb_t cmd);
void uicb_client_focus_Top(uicb_t cmd);
void uicb_client_focus_Bottom(uicb_t cmd);
/* }}} */
void uicb_client_kill(uicb_t);
void uicb_client_screen_next(uicb_t);
void uicb_client_screen_prev(uicb_t);
void uicb_client_screen_set(uicb_t);
void uicb_client_move(uicb_t cmd);
void uicb_client_resize(uicb_t cmd);
void uicb_ignore_next_client_rules(uicb_t cmd);
void uicb_clientlist(uicb_t cmd);
bool uicb_checkclist(uicb_t);
void uicb_client_ignore_tag(uicb_t);
void uicb_client_set_master(uicb_t);

/* ewmh.c */
void ewmh_init_hints(void);
void ewmh_send_message(Window d, Window w, char *atom, long d0, long d1, long d2, long d3, long d4);
long ewmh_get_xembed_state(Window win);
void ewmh_get_number_of_desktop(void);
void ewmh_update_current_tag_prop(void);
void ewmh_get_client_list(void);
void ewmh_get_desktop_names(void);
void ewmh_set_desktop_geometry(void);
void ewmh_manage_net_wm_state(long data_l[], Client *c);
void ewmh_manage_window_type(Client *c);

/* frame.c */
void frame_create(Client *c);
void frame_delete(Client *c);
void frame_moveresize(Client *c, Geo geo);
void frame_update_color(Client *c, bool focused);
void frame_update(Client *c);

/* config.c */
void init_conf(void);

/* color.c */
uint color_shade(uint, double);

/* event.c */
void grabkeys(void);
void event_make_array(void);

#ifdef HAVE_XRANDR
void xrandrevent(XEvent *e);
#endif /* HAVE_XRANDR */

/* menu.c */
void menu_init(Menu *menu, char *name, int nitem, uint bg_f, char *fg_f, uint bg_n, char *fg_n);
void menu_new_item(MenuItem *mi, char *name, void *func, char *cmd);
void menu_draw(Menu menu, int x, int y);
void uicb_menu(uicb_t cmd);
void menu_clear(Menu *menu);

/* launcher.c */
void uicb_launcher(uicb_t);

/* mouse.c */
void mouse_resize(Client *c);
void mouse_grabbuttons(Client *c, bool focused);
void uicb_mouse_move(uicb_t);
void uicb_mouse_resize(uicb_t);

/* util.c */
void *xmalloc(size_t, size_t);
void *xcalloc(size_t, size_t);
void *xrealloc(void *, size_t, size_t);
/* simples wrappers for allocating only one object */
#define zmalloc(size) xmalloc(1, (size))
#define zcalloc(size) xcalloc(1, (size))
#define zrealloc(ptr, size) xrealloc((ptr), 1, (size))
char *xstrdup(const char *);
int xasprintf(char **, const char *, ...);
long getcolor(char *color);
void setwinstate(Window win, long state);
/* Conf usage {{{ */
void* name_to_func(char *name, const func_name_list_t *l);
ulong char_to_modkey(char *name, key_name_list_t key_l[]);
uint char_to_button(char *name, name_to_uint_t blist[]);
Layout layout_name_to_struct(Layout lt[], char *name, int n, const func_name_list_t llist[]);
/* }}} */
char *char_to_str(const char c);
pid_t spawn(const char *str, ...);
void swap_ptr(void **x, void **y);
void uicb_spawn(uicb_t);
char* patht(char *path);
int qsort_string_compare (const void * a, const void * b);

/* tag.c */
void tag_set(int tag);
void tag_transfert(Client *c, int tag);
void uicb_tag(uicb_t);
void uicb_tag_next(uicb_t);
void uicb_tag_prev(uicb_t);
void uicb_tag_next_visible(uicb_t);
void uicb_tag_prev_visible(uicb_t);
void uicb_tagtransfert(uicb_t);
void uicb_tag_prev_sel(uicb_t);
void uicb_tagtransfert_next(uicb_t);
void uicb_tagtransfert_prev(uicb_t);
void uicb_tag_urgent(uicb_t cmd);
void tag_additional(int sc, int tag, int adtag);
void uicb_tag_toggle_additional(uicb_t);
void uicb_tag_swap(uicb_t);
void uicb_tag_swap_next(uicb_t);
void uicb_tag_swap_previous(uicb_t);
void uicb_tag_new(uicb_t);
void uicb_tag_del(uicb_t);
void uicb_tag_rename(uicb_t cmd);
void uicb_tag_last(uicb_t cmd);
void uicb_tag_stay_last(uicb_t cmd);
void uicb_tag_toggle_expose(uicb_t cmd);

/* screen.c */
int screen_count(void);
Geo screen_get_geo(int s);
int screen_get_with_geo(int x, int y);
int screen_get_sel(void);
void screen_set_sel(int screen);
void screen_init_geo(void);
void uicb_screen_select(uicb_t);
void uicb_screen_next(uicb_t);
void uicb_screen_prev(uicb_t);
void uicb_screen_prev_sel(uicb_t);

/* status.c */
void statustext_mouse(char *str, Geo area, InfoBar *infobar);
void statustext_handle(InfoBar *ib);

/* systray.c */
bool systray_acquire(void);
void systray_add(Window win);
void systray_del(Systray *s);
void systray_state(Systray *s);
void systray_freeicons(void);
Systray* systray_find(Window win);
int systray_get_width(void);
void systray_update(void);

/* layout.c */
void arrange(int screen, bool update_layout);
void layout_func(int screen, int tag);
Client *tiled_client(int screen, Client *c);
void freelayout(int screen);
void layoutswitch(bool b);
void maxlayout(int screen);
/* tile {{{ */
 void tile(int screen);
 void tile_left(int screen);
 void tile_top(int screen);
 void tile_bottom(int screen);
 void mirror_vertical(int screen);
 void mirror_horizontal(int screen);
 void grid_vertical(int screen);
 void grid_horizontal(int screen);
 /* }}} */
void uicb_togglemax(uicb_t);
void uicb_togglefree(uicb_t);
void uicb_layout_prev(uicb_t);
void uicb_layout_next(uicb_t);
void uicb_set_mwfact(uicb_t);
void uicb_set_nmaster(uicb_t);
void uicb_set_layout(uicb_t);
void uicb_toggle_resizehint(uicb_t);
void uicb_toggle_abovefc(uicb_t cmd);
void layout_set_client_master(Client *c);
void layout_split_client(Client *c, bool p);
void layout_split_apply(Client *c);
void layout_split_arrange_closed(int screen);
void uicb_split_client_vertical(uicb_t);
void uicb_split_client_horizontal(uicb_t);
bool uicb_checkmax(uicb_t);
bool uicb_checkfree(uicb_t);
bool uicb_checklayout(uicb_t);

/* init.c */
void init(void);

/* split.c */
void split_store_geo(int screen, int tag);
void split_set_current(Client *nc, Client *ghost);
void split_apply_current(int screen, int tag);
void split_arrange_closed(Client *ghost);
Geo split_client(Client *c, bool p);
void split_client_fill(Client *c, Geo geo);
void split_client_integrate(Client *c, Client *sc, int screen, int tag);
void split_move_dir(Client *c, Position p);
void uicb_split_toggle(uicb_t cmd);
/* Generated with macro {{{ */
void uicb_split_move_Right(uicb_t);
void uicb_split_move_Left(uicb_t);
void uicb_split_move_Top(uicb_t);
void uicb_split_move_Bottom(uicb_t);
/* }}} */

/* wmfs.c */
int errorhandler(Display *d, XErrorEvent *event);
int errorhandlerdummy(Display *d, XErrorEvent *event);
void quit(void);
void *thread_process(void *arg);
bool check_wmfs_running(void);
void exec_uicb_function(char *func, char *cmd);
void handle_signal(int signum);
void uicb_quit(uicb_t);
void uicb_reload(uicb_t);

/* Variables */

/* Main */
Display *dpy;
GC gc, gc_stipple;
int selscreen;
int prevselscreen;
Conf conf;
Key *keys;
Geo *sgeo;
Geo *spgeo;
Cursor cursor[CurLast];
char *argv_global;
char **all_argv;
int xrandr_event;
uint timing;

/* Fonts */
FontStruct font;

/* Atoms list */
Atom *net_atom;
Atom trayatom;

/* InfoBar/Tags */
InfoBar *infobar;
Tag **tags;
int *seltag;
int *prevseltag;
Menu menulayout;

/* ClientList */
Menu clientlist;
struct clndx {
     char key[4];
     Client *client;
} clist_index[MAXCLIST];

/* Client */
SLIST_HEAD(, Client) clients;
Client *sel;

/* Event */
int nevent;
void (**event_handle)(XEvent*);
extern const func_name_list_t func_list[];
extern const func_name_list_t layout_list[];
uint numlockmask;

/* Systray */
SLIST_HEAD(, Systray) trayicons;
Window traywin;
int tray_width;

/* BarWindow */
SLIST_HEAD(, BarWindow) bwhead;

/* Status */
SLIST_HEAD(, StatusMouse) smhead;

#endif /* WMFS_H */



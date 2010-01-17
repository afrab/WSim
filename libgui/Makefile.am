
noinst_LIBRARIES=libgui.a

INCLUDES=-I$(top_srcdir)

if BUILD_GUI_SDL
GUI_SRCADD=sdl/sdl_bkend.c
endif

if BUILD_GUI_X11
GUI_SRCADD=x11/x11_bkend.c
endif

if BUILD_GUI_WIN
GUI_SRCADD=win/win_bkend.c
endif

if BUILD_GUI_NONE
GUI_SRCADD=none/none_bkend.c
endif

libgui_a_CFLAGS=
libgui_a_SOURCES=ui.h ui.c ${GUI_SRCADD}


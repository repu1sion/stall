dist_doc_DATA = AUTHORS INSTALL README
AM_CFLAGS = -I /usr/include/SDL/ -O2

bin_PROGRAMS = stall
stall_SOURCES = stall_main.c stall_ai.c stall_render.c
stall_LDADD = -lSDL -lSDL_gfx -lSDL_ttf -lSDL_image

imgdir = $(datadir)/stall/img
img_DATA = img/*.png

fontdir = $(datadir)/stall/fonts
font_DATA = courbd.ttf

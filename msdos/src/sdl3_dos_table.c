/*
 * sdl3_dos_table.c: where sdl2-compat finds SDL3's functions on DOS.
 *
 * Elsewhere sdl2-compat loads SDL3 as a shared library and looks each
 * function up with dlsym(). DOS has no shared libraries, so SDL3 is linked
 * into the same program, with its SDL_* symbols renamed S3_SDL_* (see the
 * Makefile: SDL2's API keeps the plain names). This table maps each name
 * sdl2-compat asks for to the renamed function.
 */
#include <string.h>
#include <SDL3/SDL_platform_defines.h>

#define SDL3_SYM(rc,fn,params,args,ret) extern char S3_SDL_##fn[];
#include "sdl3_syms.h"
extern char S3_SDL_GetVersion[];

void *SDL2COMPAT_DOS_LookupSDL3Sym(const char *name)
{
#define SDL3_SYM(rc,fn,params,args,ret) if (strcmp(name, "SDL_" #fn) == 0) return (void *)S3_SDL_##fn;
#include "sdl3_syms.h"
    if (strcmp(name, "SDL_GetVersion") == 0) return (void *)S3_SDL_GetVersion;
    return NULL;
}

/*
 * dos_main.c: the program's entry point on DOS.
 *
 * The game's main() is compiled as rnd_main(). SDL3's DOS port needs its
 * SDL_RunApp() to start the program (it enables near pointers, which its
 * video and sound code use, and sets DJGPP's memory locking), so main()
 * hands over to it. SDL3's functions are renamed S3_SDL_* in this build.
 */
#include <stdio.h>

extern int rnd_main(int argc, char *argv[]);
extern int S3_SDL_RunApp(int argc, char *argv[], int (*mainFunction)(int, char **), void *reserved);
extern void S3_SDL_SetMainReady(void);

int main(int argc, char *argv[])
{
    S3_SDL_SetMainReady();
    return S3_SDL_RunApp(argc, argv, rnd_main, NULL);
}

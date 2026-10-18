/*
 * Coverity Scan models for psi29a/d2decomped.
 *
 * Upload this file in the project's Coverity Scan settings under Tune /
 * Analysis Settings / Modeling File. Coverity analyzes it as model code; it
 * is not part of the application build.
 *
 * The models below describe ownership of selected opaque handles returned by
 * SDL, OpenAL, and StormLib. The application owns these handles until the
 * matching destroy/close call. Modeling them as pointer-sized resources lets
 * Coverity track leaks without pretending to know the libraries' layouts.
 *
 * Deliberately not modeled here: sanitization of MPQ, save, network, WAV, or
 * other resource data. No input data is sanitized here; any library-provided
 * taint models remain active. SDL audio output buffers are left to library
 * models because their allocation sizes are returned through output
 * parameters. FFmpeg's context/buffer lifetimes are omitted because its
 * custom AVIO ownership rules and pointer-to-pointer cleanup need
 * library-aware summaries rather than guessed allocator pairs.
 */

typedef struct SDL_Window SDL_Window;
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;
typedef struct SDL_Surface SDL_Surface;
typedef struct SDL_IOStream SDL_IOStream;
typedef unsigned long SDL_WindowFlags;
typedef enum SDL_PixelFormat {
	SDL_PIXEL_FORMAT_MODEL = 0
} SDL_PixelFormat;
typedef enum SDL_TextureAccess {
	SDL_TEXTURE_ACCESS_MODEL = 0
} SDL_TextureAccess;

SDL_Window *SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags)
{
	(void)title;
	(void)w;
	(void)h;
	(void)flags;
	return (SDL_Window *)__coverity_alloc__(sizeof(void *));
}

void SDL_DestroyWindow(SDL_Window *window)
{
	__coverity_free__(window);
}

SDL_Renderer *SDL_CreateRenderer(SDL_Window *window, const char *name)
{
	(void)name;
	if (!window)
		return 0;
	return (SDL_Renderer *)__coverity_alloc__(sizeof(void *));
}

void SDL_DestroyRenderer(SDL_Renderer *renderer)
{
	__coverity_free__(renderer);
}

SDL_Texture *SDL_CreateTexture(SDL_Renderer *renderer, SDL_PixelFormat format,
                               SDL_TextureAccess access, int w, int h)
{
	(void)format;
	(void)access;
	(void)w;
	(void)h;
	if (!renderer)
		return 0;
	return (SDL_Texture *)__coverity_alloc__(sizeof(void *));
}

void SDL_DestroyTexture(SDL_Texture *texture)
{
	__coverity_free__(texture);
}

SDL_Surface *SDL_LoadBMP_IO(SDL_IOStream *src, _Bool closeio)
{
	(void)closeio;
	if (!src)
		return 0;
	return (SDL_Surface *)__coverity_alloc__(sizeof(void *));
}

void SDL_DestroySurface(SDL_Surface *surface)
{
	__coverity_free__(surface);
}

typedef struct ALCdevice ALCdevice;
typedef struct ALCcontext ALCcontext;
typedef char ALCboolean;
typedef char ALCchar;
typedef int ALCint;

ALCdevice *alcOpenDevice(const ALCchar *devicename)
{
	(void)devicename;
	return (ALCdevice *)__coverity_alloc__(sizeof(void *));
}

ALCboolean alcCloseDevice(ALCdevice *device)
{
	__coverity_free__(device);
	return 1;
}

ALCcontext *alcCreateContext(ALCdevice *device, const ALCint *attrlist)
{
	(void)attrlist;
	if (!device)
		return 0;
	return (ALCcontext *)__coverity_alloc__(sizeof(void *));
}

void alcDestroyContext(ALCcontext *context)
{
	__coverity_free__(context);
}

typedef void *HANDLE;
typedef unsigned int DWORD;

/* Keep API failure paths possible; allocate an output handle only on success. */
_Bool SFileOpenArchive(const char *filename, DWORD priority, DWORD flags, HANDLE *archive)
{
	_Bool success;
	(void)filename;
	(void)priority;
	(void)flags;
	__coverity_tainted_data_sanitize__(&success);
	if (success && archive)
		*archive = __coverity_alloc__(sizeof(void *));
	return success;
}

_Bool SFileCloseArchive(HANDLE archive)
{
	__coverity_free__(archive);
	return 1;
}

_Bool SFileOpenFileEx(HANDLE archive, const char *filename, DWORD scope, HANDLE *file)
{
	_Bool success;
	(void)filename;
	(void)scope;
	if (!archive)
		return 0;
	__coverity_tainted_data_sanitize__(&success);
	if (success && file)
		*file = __coverity_alloc__(sizeof(void *));
	return success;
}

_Bool SFileCloseFile(HANDLE file)
{
	__coverity_free__(file);
	return 1;
}

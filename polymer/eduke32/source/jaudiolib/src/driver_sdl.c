/*
 Copyright (C) 2009 Jonathon Fowler <jf@jonof.id.au>
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 
 See the GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 
 */
 
/**
 * libSDL output driver for MultiVoc
 */


#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "driver_sdl.h"

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) (P)=(P)
#endif

enum {
   SDLErr_Warning = -2,
   SDLErr_Error   = -1,
   SDLErr_Ok      = 0,
   SDLErr_Uninitialised,
   SDLErr_InitSubSystem,
   SDLErr_OpenAudio
};

static int32_t ErrorCode = SDLErr_Ok;
static int32_t Initialised = 0;
static int32_t Playing = 0;
static int32_t StartedSDL = -1;

static char *MixBuffer = 0;
static int32_t MixBufferSize = 0;
static int32_t MixBufferCount = 0;
static int32_t MixBufferCurrent = 0;
static int32_t MixBufferUsed = 0;
static void ( *MixCallBack )( void ) = 0;

static Mix_Chunk *DummyChunk = NULL;
static uint8_t *DummyBuffer = NULL;
// static int32_t InterruptsDisabled = 0;

static void fillData(int32_t chan, void *ptr, int32_t remaining, void *udata)
{
	int32_t len;
	char *sptr;

    UNREFERENCED_PARAMETER(chan);
    UNREFERENCED_PARAMETER(udata);

	while (remaining > 0) {
		if (MixBufferUsed == MixBufferSize) {
			MixCallBack();
			
			MixBufferUsed = 0;
			MixBufferCurrent++;
			if (MixBufferCurrent >= MixBufferCount) {
				MixBufferCurrent -= MixBufferCount;
			}
		}
		
		while (remaining > 0 && MixBufferUsed < MixBufferSize) {
			sptr = MixBuffer + (MixBufferCurrent * MixBufferSize) + MixBufferUsed;
			
			len = MixBufferSize - MixBufferUsed;
			if (remaining < len) {
				len = remaining;
			}
			
			memcpy(ptr, sptr, len);
			
			ptr += len;
			MixBufferUsed += len;
			remaining -= len;
		}
	}
}


int32_t SDLDrv_GetError(void)
{
    return ErrorCode;
}

const char *SDLDrv_ErrorString( int32_t ErrorNumber )
{
    const char *ErrorString;
	
    switch( ErrorNumber ) {
        case SDLErr_Warning :
        case SDLErr_Error :
            ErrorString = SDLDrv_ErrorString( ErrorCode );
            break;

        case SDLErr_Ok :
            ErrorString = "SDL Audio ok.";
            break;
			
        case SDLErr_Uninitialised:
            ErrorString = "SDL Audio uninitialised.";
            break;

        case SDLErr_InitSubSystem:
            ErrorString = "SDL Audio: error in Init or InitSubSystem.";
            break;

        case SDLErr_OpenAudio:
            ErrorString = "SDL Audio: error in OpenAudio.";
            break;

        default:
            ErrorString = "Unknown SDL Audio error code.";
            break;
    }

    return ErrorString;
}

int32_t SDLDrv_Init(int32_t mixrate, int32_t numchannels, int32_t samplebits, void * initdata)
{
    uint32_t inited;
    int32_t err = 0;
    int32_t chunksize;

    UNREFERENCED_PARAMETER(numchannels);
    UNREFERENCED_PARAMETER(initdata);

    if (Initialised) {
        SDLDrv_Shutdown();
    }

    inited = SDL_WasInit(SDL_INIT_EVERYTHING);

    if (inited == 0) {
        // nothing was initialised
        err = SDL_Init(SDL_INIT_AUDIO);
        StartedSDL = 0;
    } else if (inited & SDL_INIT_AUDIO) {
        err = SDL_InitSubSystem(SDL_INIT_AUDIO);
        StartedSDL = 1;
    }

    if (err < 0) {
        ErrorCode = SDLErr_InitSubSystem;
        return SDLErr_Error;
    }

    chunksize = 512;

    if (mixrate >= 16000) chunksize *= 2;
    if (mixrate >= 32000) chunksize *= 2;

    // allocate 4 channels: 2 for the game's SFX, 1 for music, and 1 for fillData()
    err = Mix_OpenAudio(mixrate, (samplebits == 8) ? AUDIO_U8 : AUDIO_S16SYS, 4, chunksize);

    if (err < 0) {
        ErrorCode = SDLErr_OpenAudio;
        return SDLErr_Error;
    }

    //Mix_SetPostMix(fillData, NULL);

    // channel 0 and 1 are actual sounds
    // dummy channel 2 runs our fillData() callback as an effect
    Mix_RegisterEffect(2, fillData, NULL, NULL);

    DummyBuffer = (uint8_t *) malloc(chunksize);
    memset(DummyBuffer, 0, chunksize);

    DummyChunk = Mix_QuickLoad_RAW(DummyBuffer, chunksize);

    Mix_PlayChannel(2, DummyChunk, -1);

    Initialised = 1;

    return SDLErr_Ok;
}

void SDLDrv_Shutdown(void)
{
    if (!Initialised) {
        return;
    }

    if (StartedSDL > 0) {
        if (Initialised)
        {
            Mix_HaltChannel(0);
        }

        if (DummyChunk != NULL)
        {
            Mix_FreeChunk(DummyChunk);
        }

        DummyChunk = NULL;

        if (DummyBuffer  != NULL)
        {
            free(DummyBuffer);
        }

        DummyBuffer = NULL;

        if (Initialised)
        {
            Mix_CloseAudio();
        }

        Initialised = 0;

        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    } 
    else if (StartedSDL == 0) {
        SDL_Quit();
    }


    StartedSDL = -1;
}

int32_t SDLDrv_BeginPlayback(char *BufferStart, int32_t BufferSize,
						int32_t NumDivisions, void ( *CallBackFunc )( void ) )
{
	if (!Initialised) {
		ErrorCode = SDLErr_Uninitialised;
		return SDLErr_Error;
	}
	
	if (Playing) {
		SDLDrv_StopPlayback();
	}
    
	MixBuffer = BufferStart;
	MixBufferSize = BufferSize;
	MixBufferCount = NumDivisions;
	MixBufferCurrent = 0;
	MixBufferUsed = 0;
	MixCallBack = CallBackFunc;
	
	// prime the buffer
	MixCallBack();
    
	Mix_Resume(-1);
    
    Playing = 1;
    
	return SDLErr_Ok;
}

void SDLDrv_StopPlayback(void)
{
	if (!Initialised || !Playing) {
		return;
	}

    Mix_Pause(-1);
	
	Playing = 0;
}

void SDLDrv_Lock(void)
{
/*
        if (InterruptsDisabled++)
            return;
    
        SDL_LockAudio();*/
    
}

void SDLDrv_Unlock(void)
{
/*
        if (--InterruptsDisabled)
            return;
    
        SDL_UnlockAudio();*/
    
}


/*
   Copyright 2013 Ritvik Choudhary

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
   IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __scrobsol_H__
#define __scrobsol_H__

#if defined(_MSC_VER) && _MSC_VER <= 1400
 #define scrobsol_NO_C99 1
#endif

#if scrobsol_NO_C99
 #if !__cplusplus && !defined(bool)
  #define bool int
  #define false 0
  #define true 1
 #endif
#else
 #include <stdbool.h>
#endif


#if __cplusplus
extern "C" {
#endif

/** Indeed, call this at your earliest convenience! */
void scrobsol_init(void(*callback)(int event, const char* message));

/** @returns true if scrobsol is relaying scrobbles to the official Last.fm
  * Audioscrobbler application */
bool scrobsol_query_relay();

/** @see scrobsol_auth. Scrobbling will not function until your application is
  * auth'd. However we will be caching scrobbles in memory. */
#define scrobsol_AUTH_REQUIRED 0
/** The char* paramater will be the error string */
#define scrobsol_ERROR_RESPONSE 1


/** The URL at url needs to be visited by the user in a web browser withing an
  * hour to authorize your application for scrobbling. @see
  * scrobsol_AUTH_REQUIRED. */
void scrobsol_auth(char url[110]);
/** Once the user has visited the above page, we need to request their session
  * key, you can either do this yourself, or scrobsol will do it as required,
  * mostly you won't need to call this function
  * @returns true if authentication was successful */
bool scrobsol_finish_auth();


/** A new track started.
  * We dup' the strings, so don't worry yourself with memory woes.
  * All strings must be UTF8.
  * artist, track and duration are mandatory.
  * album and mbid can be "" (do *not* pass NULL or 0).
  * A value of 0 for track_number will be ignored.
  * You should note that valid MBIDs are always 38 characters. */
void scrobsol_start(const char* artist, const char* track, unsigned int duration, const char* album, unsigned int track_number, const char* mbid);
/** The track we are scrobbling became paused. This is NOT a toggle! When/if
  * the track is unpaused, call resume. We insist on this distinction because
  * we want you to be exact! */
void scrobsol_pause();
/** The thing that we're scrobbling was unpaused */
void scrobsol_resume();
/** Only call this when playback stops, if a new track is about to start, call
  * scrobsol_start() instead */
void scrobsol_stop();


/** Marks the current track as loved, it is worth noting, you also have to call
  * the Last.fm track.love webservice separately (scrobsol doesn't do it for
  * you). This should be fixed with Audioscrobbler protocol 2.0
  */
void scrobsol_love();

/** Support this if you can and it is appropriate */
void scrobsol_change_metadata(const char* artist, const char* track, const char* album);


#define scrobsol_STOPPED 0
#define scrobsol_PLAYING 1
#define scrobsol_PAUSED 2

int scrobsol_state();


/** If these are NULL then authorization has not yet been completed */
extern char* scrobsol_session_key;
extern char* scrobsol_username;


/** For your convenience, we need it, so maybe you can use it too. We use the
  * platform native library to generate this md5 */
void scrobsol_md5(char out[33], const char* in);


/** This will always be the same as the protocol version we implement */
#define scrobsol_VERSION 0x00010201
#define scrobsol_VERSION_STRING "1.2.1"
#define scrobsol_MAJOR_VERSION 1
#define scrobsol_MINOR_VERSION 2
#define scrobsol_PATCH_VERSION 1

#if __cplusplus
}
#endif
#endif

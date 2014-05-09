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
   
   This file was originally created by Ritvik Choudhary
*/
#include "scrobsol.h"
#if __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#endif

#if __APPLE__
bool scrobsol_fsref(FSRef* fsref)
{
    OSStatus err = LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("fm.last.Audioscrobbler"), NULL, fsref, NULL);
    return err != kLSApplicationNotFoundErr;
}
#endif

/** returns false if Audioscrobbler is not installed */
bool scrobsol_launch_audioscrobbler()
{
#if __APPLE__
    FSRef fsref;
    if (!scrobsol_fsref(&fsref))
        return false;
    
    LSApplicationParameters p = {0};
    p.flags = kLSLaunchDontSwitch | kLSLaunchAsync;
    p.application = &fsref;
    LSOpenApplication( &p, NULL ); //won't launch if already running
    return true; //TODO if failed to launch we should log it
#endif
}

#if __APPLE__
static void script(const char* cmd)
{
    // the $ allows us to escape single quotes inside a single quoted string
    char a[] = "osascript -e $'tell application \"Audioscrobbler\" to ";
    char b[sizeof(a)+strlen(cmd)+2];
    strcpy(b, a);
    strcat(b, cmd);
    strcat(b, "'");
    system(b);
}

void scrobsol_relay(int state)
{
    switch(state){
        case scrobsol_PLAYING: script("resume \""scrobsol_CLIENT_ID"\""); break;
        case scrobsol_PAUSED:  script("pause \""scrobsol_CLIENT_ID"\""); break;
        case scrobsol_STOPPED: script("stop \""scrobsol_CLIENT_ID"\""); break;
    }
}

static inline void strcat_escape_quotes(char* dst, const char* src)
{
    // get to the end of the dst string first
    while(*dst)
        dst++;
    
    char c;
    while(c = *src++){
        if(c == '\'' || c == '"')
            *dst++ = '\\';
        *dst++ = c;
    }
    *dst = '\0';
}

void scrobsol_relay_start(const char* artist, const char* title, int durationi)
{    
    #define START "start \"" scrobsol_CLIENT_ID "\" with \""
    #define BY "\" by \""
    #define DURATION "\" duration %d   " //strlen("%d")+3 = 5 digits, thus up to 99,999 seconds

    const uint N = sizeof(START BY DURATION) +
                   strlen(artist)*2 + // double the length of maybe-quoted strings
                   strlen(title)*2;
    char s[N];
    strcpy(s, START);
    strcat_escape_quotes(s, title);
    strcat(s, BY);
    strcat_escape_quotes(s, artist);
    
    char durations[] = DURATION;
    snprintf(durations, sizeof(durations), DURATION, durationi);
    strcat(s, durations);

    script(s);
}

#else //TODO!

void scrobsol_relay_start(const char* artist, const char* title, int duration)
{}

void scrobsol_relay(int state)
{}

#endif //__APPLE__

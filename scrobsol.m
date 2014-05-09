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
#import "scrobsol.h"
#import <Cocoa/Cocoa.h>

//TODO should be per application, not per machine
#define KEYCHAIN_NAME "fm.last.Audioscrobbler"

static NSString* token;
extern void(*scrobsol_callback)(int event, const char* message);


bool scrobsol_retrieve_credentials()
{
#ifdef __AS_DEBUGGING__
    scrobsol_username = "testuser";
    scrobsol_session_key = "d20e0c83aa4252d8bcb945fbaa4aec2a";
    return true;
#else
    NSString* username = [[NSUserDefaults standardUserDefaults] stringForKey:@"Username"];
    if(!username) return false;
    scrobsol_username = strdup([username UTF8String]);
    
    void* key;
    UInt32 n;
    OSStatus err = SecKeychainFindGenericPassword(NULL, //default keychain
                                                  sizeof(KEYCHAIN_NAME),
                                                  KEYCHAIN_NAME,
                                                  strlen(scrobsol_username),
                                                  scrobsol_username,
                                                  &n,
                                                  &key,
                                                  NULL);
    if(err != noErr)return false;

    scrobsol_session_key = malloc(n+1);
    memcpy(scrobsol_session_key, key, n);
    scrobsol_session_key[n] = '\0';

    SecKeychainItemFreeContent(NULL, key);
    return true;
#endif
}

void scrobsol_get(char response[256], const char* url)
{
    NSStringEncoding encoding;
    NSString *output = [NSString stringWithContentsOfURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]
                                            usedEncoding:&encoding
                                                   error:nil];
    if(output)
        strncpy(response, [output UTF8String], 256);
}

void scrobsol_post(char response[256], const char* url, const char* post_data)
{   
    int const n = strlen(post_data);
    NSData *body = [NSData dataWithBytes:post_data length:n];    
    
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:[NSString stringWithUTF8String:url]]
                                                           cachePolicy:NSURLRequestReloadIgnoringCacheData
                                                       timeoutInterval:10];    
    [request setHTTPMethod:@"POST"];
    [request setHTTPBody:body];
    [request setValue:@"fm.last.Audioscrobbler" forHTTPHeaderField:@"User-Agent"];
    [request setValue:[[NSNumber numberWithInteger:n] stringValue] forHTTPHeaderField:@"Content-Length"];
    [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
    
    NSURLResponse* headers = NULL;
    NSError* error = NULL;
    NSData* data = [NSURLConnection sendSynchronousRequest:request returningResponse:&headers error:&error];
    
    [data getBytes:response length:256];
}

void scrobsol_auth(char out_url[110])
{
    if(token == nil){
        NSURL* url = [NSURL URLWithString:@"http://ws.audioscrobbler.com/2.0/?method=auth.gettoken&api_key=" scrobsol_API_KEY ];
        NSXMLDocument* xml = [[NSXMLDocument alloc] initWithContentsOfURL:url options:0 error:nil];
        token = [[[[xml rootElement] elementsForName:@"token"] lastObject] stringValue];
        [token retain];
        [xml release];
    }

    strcpy(out_url, "http://www.last.fm/api/auth/?api_key=" scrobsol_API_KEY "&token=");
    strcat(out_url, [token UTF8String]);
}

//TODO localise and get webservice error
//TODO error handling
bool scrobsol_finish_auth()
{
    if(!token) return false;
    if(scrobsol_session_key) return true;
    
    char sig[33];
    NSString* format = @"api_key" scrobsol_API_KEY "methodauth.getSessiontoken%@" scrobsol_SHARED_SECRET;
    scrobsol_md5(sig, [[NSString stringWithFormat:format, token] UTF8String]);
    NSURL* url = [NSURL URLWithString:[NSString stringWithFormat:@"http://ws.audioscrobbler.com/2.0/?method=auth.getSession&api_key=" scrobsol_API_KEY "&token=%@&api_sig=%s", token, sig]];

    NSXMLDocument* xml = [[NSXMLDocument alloc] initWithContentsOfURL:url options:0 error:nil];
    NSXMLElement* session = [[[xml rootElement] elementsForName:@"session"] lastObject];
    NSString* sk = [[[session elementsForName:@"key"] lastObject] stringValue];
    NSString* username = [[[session elementsForName:@"name"] lastObject] stringValue];
    [xml release];
       
    if(!username || !sk)return false;

    scrobsol_session_key = strdup([sk UTF8String]);
    scrobsol_username = strdup([username UTF8String]);
    
    OSStatus err = SecKeychainAddGenericPassword(NULL, //default keychain
                                                 sizeof(KEYCHAIN_NAME),
                                                 KEYCHAIN_NAME,
                                                 strlen(scrobsol_username),
                                                 scrobsol_username,
                                                 32,
                                                 scrobsol_session_key,
                                                 NULL);
    NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:username forKey:@"Username"];
    [defaults synchronize];

    (void)err; //TODO
    
    return true;
}

/* SPDX-License-Identifier: GPL-3.0-only; Copyright (C) 2026 Karsei */
/* Exercise the real native server with a host filesystem and fragmented socket adapter.
 * This verifies HTTP logic; Windows API/runtime qualification is separate. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../desktop/red_mass_app.c"
static const char *request_data;static unsigned request_at,response_n;
static char response_data[2000000];
H GetProcessHeap(void){return 0;}
H HeapAlloc(H h,U flags,P size){(void)h;(void)flags;return malloc((size_t)size);}
int HeapFree(H h,U flags,H memory){(void)h;(void)flags;free(memory);return 1;}
int recv(P sock,char *data,int size,int flags){(void)sock;(void)flags;unsigned left=(unsigned)strlen(request_data)-request_at;if(!left)return 0;unsigned n=left<7?left:7;if(n>(unsigned)size)n=(unsigned)size;memcpy(data,request_data+request_at,n);request_at+=n;return (int)n;}
int send(P sock,const char *data,int size,int flags){(void)sock;(void)flags;if(response_n+(unsigned)size>=sizeof response_data)return -1;unsigned n=(unsigned)size>23?23:(unsigned)size;memcpy(response_data+response_n,data,n);response_n+=n;response_data[response_n]=0;return (int)n;}
int setsockopt(P a,int b,int c,const char*d,int e){(void)a;(void)b;(void)c;(void)d;(void)e;return 0;}
int shutdown(P a,int b){(void)a;(void)b;return 0;}
int closesocket(P a){(void)a;return 0;}
H CreateFileW(const W *path,U a,U b,H c,U d,U e,H f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;char name[4096];unsigned i;for(i=0;path[i]&&i+1<sizeof name;i++)name[i]=(char)(path[i]=='\\'?'/':path[i]);name[i]=0;FILE*file=fopen(name,"rb");return file?(H)file:(H)-1;}
U GetFileSize(H f,U *hi){long now=ftell((FILE*)f);fseek((FILE*)f,0,SEEK_END);long size=ftell((FILE*)f);fseek((FILE*)f,now,SEEK_SET);*hi=0;return (U)size;}
int ReadFile(H f,H data,U n,U *got,H ignored){(void)ignored;*got=(U)fread(data,1,n,(FILE*)f);return !ferror((FILE*)f);}
int CloseHandle(H f){return fclose((FILE*)f)==0;}
static int check(const char *query,const char *status,const char *mime_type,int body){request_data=query;request_at=response_n=0;serve((H)1);char *start=strstr(response_data,"\r\n\r\n");int ok=strstr(response_data,status)==response_data&&start&&(!mime_type||strstr(response_data,mime_type))&&(body?response_n>(unsigned)(start-response_data)+4:response_n==(unsigned)(start-response_data)+4);printf("Desktop server %.60s: %s\n",query,ok?"ok":"FAIL");return !ok;}
int main(int argc,char**argv){if(argc!=2)return 2;folder_n=(unsigned)strlen(argv[1]);if(folder_n>=1900)return 2;for(unsigned i=0;i<=folder_n;i++)folder[i]=(W)argv[1][i];int bad=0;
 bad+=check("GET / HTTP/1.1\r\nHost: localhost\r\n\r\n","HTTP/1.1 200 OK","text/html",1);
 bad+=check("GET /web-tools/webapp/editor/ HTTP/1.1\r\n\r\n","HTTP/1.1 200 OK","text/html",1);
 bad+=check("HEAD /web-tools/webapp/editor/index.html HTTP/1.1\r\n\r\n","HTTP/1.1 200 OK","text/html",0);
 bad+=check("GET /web-tools/webapp/installer/ HTTP/1.1\r\n\r\n","HTTP/1.1 200 OK","text/html",1);
 bad+=check("GET /source/web/fukiai.ttf HTTP/1.1\r\n\r\n","HTTP/1.1 200 OK","font/ttf",1);
 bad+=check("GET /%2e%2e/secret HTTP/1.1\r\n\r\n","HTTP/1.1 400 Bad Request",0,0);
 bad+=check("GET /a/..%20/secret HTTP/1.1\r\n\r\n","HTTP/1.1 400 Bad Request",0,0);
 bad+=check("GET /missing HTTP/1.1\r\n\r\n","HTTP/1.1 404 Not Found",0,0);
 bad+=check("POST / HTTP/1.1\r\n\r\n","HTTP/1.1 405 Method Not Allowed",0,0);
 printf("Desktop server checks: %d failures\n",bad);return bad!=0;
}

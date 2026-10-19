/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Karsei
 * Native Windows x64 launcher and read-only loopback HTTP server.
 * Edge/Chrome app mode hosts the COMPLETE bundled editor and installer.
 * No Python, Node, .NET runtime, npm or remotely hosted application code. */
typedef unsigned int U;
typedef unsigned short W;
typedef unsigned long long P;
typedef long long R;
typedef void *H;
#define API extern
API U GetModuleFileNameW(H,W*,U);
API U ExpandEnvironmentStringsW(const W*,W*,U);
API U GetFileAttributesW(const W*);
API H GetModuleHandleW(const W*);
API H GetProcessHeap(void);
API H HeapAlloc(H,U,P);
API int HeapFree(H,U,H);
API H CreateFileW(const W*,U,U,H,U,U,H);
API int ReadFile(H,H,U,U*,H);
API U GetFileSize(H,U*);
API int CloseHandle(H);
API H CreateThread(H,P,U(*)(H),H,U,U*);
API void ExitProcess(U);
API void Sleep(U);
API int MessageBoxW(H,const W*,const W*,U);
API H ShellExecuteW(H,const W*,const W*,const W*,const W*,int);
API int WSAStartup(W,H);
API int WSACleanup(void);
API P socket(int,int,int);
API int bind(P,const H,int);
API int listen(P,int);
API P accept(P,H,int*);
API int getsockname(P,H,int*);
API int closesocket(P);
API int setsockopt(P,int,int,const char*,int);
API int send(P,const char*,int,int);
API int recv(P,char*,int,int);
API int shutdown(P,int);
typedef struct {int x,y;} POINT;
typedef struct {H hwnd;U message;P wParam;R lParam;U time;POINT pt;U private;} MSG;
typedef R(*PROC)(H,U,P,R);
typedef struct {U style;PROC proc;int cbClsExtra,cbWndExtra;H instance,icon,cursor,brush;const W *menu,*name;} CLASS;
API W RegisterClassW(const CLASS*);
API H CreateWindowExW(U,const W*,const W*,U,int,int,int,int,H,H,H,H);
API R DefWindowProcW(H,U,P,R);
API int DestroyWindow(H);
API int GetMessageW(MSG*,H,U,U);
API int TranslateMessage(const MSG*);
API R DispatchMessageW(const MSG*);
API void PostQuitMessage(int);
API H LoadIconW(H,const W*);
API H CreatePopupMenu(void);
API int DestroyMenu(H);
API int AppendMenuW(H,U,P,const W*);
API U TrackPopupMenu(H,U,int,int,int,H,H);
API int GetCursorPos(POINT*);
API int SetForegroundWindow(H);
typedef struct {U size;H hwnd;U id,flags,callback;H icon;W tip[64];} TRAY;
API int Shell_NotifyIconW(U,TRAY*);

#include "http_path.h"
static W folder[2048], browser[2048], app_args[256];
static unsigned folder_n;
static P listener;
static H window;
static volatile int stopping;
static TRAY tray;
static const W title[]=L"RED MASS VAMPIRE OS";
static unsigned slen(const char*s){unsigned n=0;while(s[n])n++;return n;}
static void copy(char*d,const char*s){while((*d++=*s++));}
static void wcopy(W*d,const W*s){while((*d++=*s++));}
static int ends(const char*s,const char*t){unsigned a=slen(s),b=slen(t);if(a<b)return 0;while(b){if(s[--a]!=t[--b])return 0;}return 1;}
static char* number(char*d,U n){char b[12];unsigned k=0;do{b[k++]=(char)('0'+n%10);n/=10;}while(n);while(k)*d++=b[--k];*d=0;return d;}
static int send_all(P sock,const char *data,unsigned size){while(size){int n=send(sock,data,(int)size,0);if(n<=0)return 0;data+=n;size-=(U)n;}return 1;}
static void fail(P sock,const char*status){char b[256];copy(b,"HTTP/1.1 ");copy(b+slen(b),status);copy(b+slen(b),"\r\nContent-Length: 0\r\nConnection: close\r\n\r\n");send_all(sock,b,slen(b));}
static const char* mime(const char*p){
 if(ends(p,".html"))return "text/html; charset=utf-8";
 if(ends(p,".js"))return "text/javascript; charset=utf-8";
 if(ends(p,".json"))return "application/json";
 if(ends(p,".css"))return "text/css";
 if(ends(p,".png"))return "image/png";
 if(ends(p,".svg"))return "image/svg+xml";
 if(ends(p,".ico"))return "image/x-icon";
 if(ends(p,".ttf"))return "font/ttf";
 if(ends(p,".woff"))return "font/woff";
 if(ends(p,".woff2"))return "font/woff2";
 if(ends(p,".mp3"))return "audio/mpeg";
 if(ends(p,".wav"))return "audio/wav";
 if(ends(p,".txt")||ends(p,".md")||ends(p,"LICENSE"))return "text/plain; charset=utf-8";
 return "application/octet-stream";
}
static U serve(H arg){
 P client=(P)arg;char *buf=(char*)HeapAlloc(GetProcessHeap(),0,16384),*url,*end,*p;
 W *path=(W*)HeapAlloc(GetProcessHeap(),0,8192);char relative[1024],head[512];
 U got=0,size,high=0,count;H file=(H)-1;int head_only=0;
 if(!buf||!path)goto done;
 {U ms=5000;setsockopt(client,0xffff,0x1006,(char*)&ms,4);setsockopt(client,0xffff,0x1005,(char*)&ms,4);}
 while(got<4095){int n=recv(client,buf+got,(int)(4095-got),0);if(n<=0)goto done;got+=(U)n;buf[got]=0;for(p=buf;*p&&*p!='\n';p++) {} if(*p)break;}
 if(buf[0]=='G'&&buf[1]=='E'&&buf[2]=='T'&&buf[3]==' ')url=buf+4;
 else if(buf[0]=='H'&&buf[1]=='E'&&buf[2]=='A'&&buf[3]=='D'&&buf[4]==' '){url=buf+5;head_only=1;}
 else{fail(client,"405 Method Not Allowed");goto done;}
 for(end=url;*end&&*end!=' '&&*end!='\r'&&*end!='\n';end++);
 if(*end!=' '){fail(client,"400 Bad Request");goto done;}*end=0;
 if(!local_path(url,relative,sizeof relative)){fail(client,"400 Bad Request");goto done;}
 if(!relative[0])copy(relative,"web-tools/webapp/app/index.html");
 else if(ends(relative,"/")){if(slen(relative)+11>=sizeof relative){fail(client,"414 URI Too Long");goto done;}copy(relative+slen(relative),"index.html");}
 wcopy(path,folder);path[folder_n]='\\';
 for(count=0;relative[count];count++)path[folder_n+1+count]=(W)(relative[count]=='/'?'\\':relative[count]);
 path[folder_n+1+count]=0;
 file=CreateFileW(path,0x80000000,1,0,3,0x80,0);
 if(file==(H)-1){fail(client,"404 Not Found");goto done;}
 size=GetFileSize(file,&high);if(high||size==0xffffffff){fail(client,"413 Content Too Large");goto done;}
 copy(head,"HTTP/1.1 200 OK\r\nContent-Type: ");copy(head+slen(head),mime(relative));
 copy(head+slen(head),"\r\nContent-Length: ");number(head+slen(head),size);
 copy(head+slen(head),"\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nConnection: close\r\n\r\n");
 if(!send_all(client,head,slen(head))||head_only)goto done;
 while(size){U n=size>16384?16384:size;if(!ReadFile(file,buf,n,&got,0)||!got||!send_all(client,buf,got))break;size-=got;}
done:
 if(file!=(H)-1)CloseHandle(file);
 if(buf)HeapFree(GetProcessHeap(),0,buf);
 if(path)HeapFree(GetProcessHeap(),0,path);
 shutdown(client,1);closesocket(client);return 0;
}
static U server(H unused){(void)unused;while(!stopping){P client=accept(listener,0,0);if(client==(P)-1)break;H t=CreateThread(0,0,serve,(H)client,0,0);if(t)CloseHandle(t);else closesocket(client);}return 0;}
static void open_app(void){if((P)ShellExecuteW(window,L"open",browser,app_args,folder,1)<=32)MessageBoxW(window,L"Could not open Edge/Chrome. Open the English guide for troubleshooting.",title,0x10);}
static R proc(H hwnd,U msg,P a,R b){
 if(msg==0x8001){if(b==0x202||b==0x203)open_app();else if(b==0x205){POINT pt;H menu=CreatePopupMenu();GetCursorPos(&pt);AppendMenuW(menu,0,1,L"Open RED MASS VAMPIRE OS");AppendMenuW(menu,0,2,L"Exit application");SetForegroundWindow(hwnd);U choice=TrackPopupMenu(menu,0x102,pt.x,pt.y,0,hwnd,0);DestroyMenu(menu);if(choice==1)open_app();if(choice==2&&MessageBoxW(hwnd,L"Finish any firmware update before exiting. Exit RED MASS VAMPIRE OS?",title,0x34)==6)DestroyWindow(hwnd);}return 0;}
 if(msg==2){stopping=1;Shell_NotifyIconW(2,&tray);closesocket(listener);PostQuitMessage(0);return 0;}
 return DefWindowProcW(hwnd,msg,a,b);
}
void entry(void){
 static const W *paths[]={L"%ProgramFiles(x86)%\\Microsoft\\Edge\\Application\\msedge.exe",L"%ProgramFiles%\\Microsoft\\Edge\\Application\\msedge.exe",L"%LOCALAPPDATA%\\Microsoft\\Edge\\Application\\msedge.exe",L"%ProgramFiles%\\Google\\Chrome\\Application\\chrome.exe",L"%ProgramFiles(x86)%\\Google\\Chrome\\Application\\chrome.exe",L"%LOCALAPPDATA%\\Google\\Chrome\\Application\\chrome.exe"};
 static W check[2048];unsigned i;CLASS cls={0};MSG msg;char url[128];static unsigned char wsadata[512];
 struct {W family,port;U ip;char zero[8];} addr={2,0,0x0100007f,{0}};int addrlen=16;
 U module_len=GetModuleFileNameW(0,folder,2048);
 if(!module_len||module_len>=1900)ExitProcess(1);
 for(folder_n=module_len;folder_n&&folder[folder_n]!='\\';folder_n--) {}
 folder[folder_n]=0;
 for(i=0;i<sizeof paths/sizeof paths[0];i++){U n=ExpandEnvironmentStringsW(paths[i],browser,2048);if(n&&n<=2048&&GetFileAttributesW(browser)!=0xffffffff)break;}
 if(i==sizeof paths/sizeof paths[0]){MessageBoxW(0,L"This application needs Microsoft Edge or Google Chrome for USB-MIDI. Python is not required. Install Edge/Chrome, then launch again.",title,0x10);ExitProcess(2);}
 {wcopy(check,folder);wcopy(check+folder_n,L"\\web-tools\\webapp\\app\\index.html");if(GetFileAttributesW(check)==0xffffffff){MessageBoxW(0,L"Extract the complete ZIP into one folder before launching. Keep the EXE next to web-tools, firmware, docs and source.",title,0x10);ExitProcess(3);}}
 if(WSAStartup(0x202,wsadata)){MessageBoxW(0,L"Windows networking could not start.",title,0x10);ExitProcess(4);}
 listener=socket(2,1,6);if(listener==(P)-1||bind(listener,&addr,16)||getsockname(listener,&addr,&addrlen)||listen(listener,8)){MessageBoxW(0,L"Could not start the local application server.",title,0x10);ExitProcess(5);}
 copy(url,"--app=http://127.0.0.1:");number(url+slen(url),((U)addr.port>>8)|(((U)addr.port&255)<<8));copy(url+slen(url),"/web-tools/webapp/app/");
 for(i=0;url[i];i++)app_args[i]=(W)url[i];
 app_args[i]=0;
 cls.proc=proc;cls.instance=GetModuleHandleW(0);cls.name=L"RedMassVampireHost";
 if(!RegisterClassW(&cls)){MessageBoxW(0,L"Application window could not start.",title,0x10);ExitProcess(6);}
 window=CreateWindowExW(0,cls.name,title,0,0,0,0,0,0,0,cls.instance,0);
 if(!window)ExitProcess(7);
 tray.size=sizeof tray;tray.hwnd=window;tray.id=1;tray.flags=7;tray.callback=0x8001;tray.icon=LoadIconW(0,(const W*)32512);wcopy(tray.tip,title);
 if(!Shell_NotifyIconW(0,&tray)){MessageBoxW(0,L"Tray icon could not start. Restart the application.",title,0x10);ExitProcess(8);}
 {H thread=CreateThread(0,0,server,0,0,0);if(!thread)ExitProcess(9);CloseHandle(thread);}
 open_app();
 while(GetMessageW(&msg,0,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
 WSACleanup();ExitProcess(0);
}

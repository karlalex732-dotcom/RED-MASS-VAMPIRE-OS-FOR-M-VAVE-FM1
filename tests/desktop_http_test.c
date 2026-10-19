/* SPDX-License-Identifier: GPL-3.0-only; Copyright (C) 2026 Karsei */
#include <stdio.h>
#include <string.h>
#include "../desktop/http_path.h"
int main(void){
 char out[1024];int failures=0;
 const char*valid[]={"/","/web-tools/webapp/editor/","/web-tools/webapp/installer/index.html","/docs/RED-MASS-English-Guide.html","/preview/audio/001.mp3?cache=1","/a%20b/file.txt"};
 const char*bad[]={"/../LICENSE","/%2e%2e/README","/a/./b","/a/.. /b","/a/.%20/b","/a%5c..%5cx","/a%00b","/a%3ab","/a%","/a%xx","/a//b","/a/..","/a/.","/a%20","/a%2e","/a|b","/a%2fb%2f..%2fc"};
 for(unsigned i=0;i<sizeof valid/sizeof valid[0];i++)if(!local_path(valid[i],out,sizeof out)){printf("valid path rejected: %s\n",valid[i]);failures++;}
 for(unsigned i=0;i<sizeof bad/sizeof bad[0];i++)if(local_path(bad[i],out,sizeof out)){printf("unsafe path accepted: %s\n",bad[i]);failures++;}
 if(local_path("/long",out,3))failures++;
 printf("Desktop HTTP path checks: %d failures\n",failures);return failures!=0;
}

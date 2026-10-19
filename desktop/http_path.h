/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Karsei */
/* Decode a URL path into a relative, ASCII bundle path. No filesystem mutation. */
static int hx(char c) {
    if(c>='0'&&c<='9')return c-'0';
    if(c>='a'&&c<='f')return c-'a'+10;
    if(c>='A'&&c<='F')return c-'A'+10;
    return -1;
}
static int local_path(const char *url, char *out, unsigned cap) {
    unsigned n=0,seg=0,i=0;
    if(url[0]!='/')return 0;
    for(i=1;url[i]&&url[i]!='?'&&url[i]!='#';i++) {
        unsigned char c=(unsigned char)url[i];
        if(c=='%') {
            int a,b;
            if(!url[i+1]||!url[i+2]||(a=hx(url[i+1]))<0||(b=hx(url[i+2]))<0)return 0;
            c=(unsigned char)(a*16+b);i+=2;
        }
        if(c<32||c>=127||c=='\\'||c==':'||c=='*'||c=='?'||c=='"'||c=='<'||c=='>'||c=='|')return 0;
        if(c=='/') {
            if((n>seg&&(out[n-1]=='.'||out[n-1]==' '))||n==seg||(n-seg==1&&out[seg]=='.')||(n-seg==2&&out[seg]=='.'&&out[seg+1]=='.'))return 0;
            seg=n+1;
        }
        if(n+1>=cap)return 0;
        out[n++]=(char)c;
    }
    if(n>seg&&((n-seg==1&&out[seg]=='.')||(n-seg==2&&out[seg]=='.'&&out[seg+1]=='.')))return 0;
    if(n>seg&&(out[n-1]=='.'||out[n-1]==' '))return 0; /* Windows path normalization */
    out[n]=0;
    return 1;
}

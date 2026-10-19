/* SPDX-License-Identifier: GPL-3.0-only; Copyright (C) 2026 Karsei */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include <stdio.h>
#include <stdlib.h>
static int bad;
static void setup(unsigned kit){ui_power_on();song.master_q12=4096;song.groove=1;track_select(5);set_engine_of(TSEL,ENGI_SMPKIT);TSEL->engine=TSEL->eng_req;TSEL->p[P_E0]=(int16_t)kit;panic_req=0;}
static void put32(FILE*f,uint32_t n){for(unsigned i=0;i<4;i++)fputc(n>>(i*8),f);}
static void put16(FILE*f,uint16_t n){fputc(n,f);fputc(n>>8,f);}
static FILE*wav(const char*path,uint32_t frames){FILE*f=fopen(path,"wb");if(!f)return 0;fwrite("RIFF",1,4,f);put32(f,36+frames*4);fwrite("WAVEfmt ",1,8,f);put32(f,16);put16(f,1);put16(f,2);put32(f,FS);put32(f,FS*4);put16(f,4);put16(f,16);fwrite("data",1,4,f);put32(f,frames*4);return f;}
int main(int argc,char**argv){
 uint64_t signature[2]={0,0};int32_t out[CTL*2];
 for(unsigned k=0;k<2;k++)for(unsigned pad=0;pad<16;pad++){
  setup(k?4:0);trk_note_on(TSEL,GROOVE_NOTE[pad],100);int peak=0;int64_t energy=0;
  for(unsigned b=0;b<FS/CTL+1;b++){mix_block(out,CTL);for(unsigned j=0;j<CTL*2;j++){int v=out[j],a=v<0?-v:v;if(a>peak)peak=a;energy+=(int64_t)v*v;signature[k]=signature[k]*1099511628211ull+(uint32_t)v;}}
  unsigned active=0;for(unsigned v=0;v<NVOICE;v++)active+=TSEL->v[v].active;
  int ok=peak>250&&peak<30000&&!active&&energy>100000;
  printf("Trap kit %s pad %02u: peak=%d active=%u %s\n",k?"RAGE":"NIGHT",pad+1,peak,active,ok?"ok":"FAIL");bad+=!ok;
 }
 bad+=signature[0]==signature[1];
 setup(4);project_t p,q;project_store_t wire;project_capture(&p);int ok=proj_pack(&wire,&p)&&proj_import(&q,&wire,sizeof wire);bad+=!ok||q.t[5].p[P_E0]!=4;
 printf("RAGE factory kit survives project round-trip: %s\n",ok&&q.t[5].p[P_E0]==4?"ok":"FAIL");
 for(unsigned k=0;k<2;k++){
  setup(k?4:0);trk_note_on(TSEL,GROOVE_NOTE[4],100);trk_note_on(TSEL,GROOVE_NOTE[3],100);
  unsigned open=0;for(unsigned v=0;v<NVOICE;v++)open+=TSEL->v[v].active&&TSEL->v[v].note==GROOVE_NOTE[4]&&!TSEL->v[v].s[6];
  bad+=open!=0;
 }
 if(argc>1)for(unsigned k=0;k<2;k++){
  setup(k?4:0);char path[1024];snprintf(path,sizeof path,"%s/%s-demo.wav",argv[1],k?"RAGE-TRAP":"NIGHT-TRAP");
  unsigned beat=FS*60/140,frames=(beat*24/CTL)*CTL;FILE*f=wav(path,frames);if(!f){bad++;continue;}
  unsigned next=0,tick=0;
  for(unsigned frame=0;frame<frames;frame+=CTL){
   if(frame>=next){unsigned step=tick%32;
    trk_note_on(TSEL,GROOVE_NOTE[3],step%4==0?88:step%2?48:68);
    if(step%16==8)trk_note_on(TSEL,GROOVE_NOTE[1],105);
    if(step%16==8)trk_note_on(TSEL,GROOVE_NOTE[2],78);
    if(step%16==0||step%16==11||step%16==14)trk_note_on(TSEL,GROOVE_NOTE[0],106);
    if(step%16==0||step%16==11)trk_note_on(TSEL,GROOVE_NOTE[8],96);
    if(step%16==6)trk_note_on(TSEL,GROOVE_NOTE[4],57);
    if(step%32==23)trk_note_on(TSEL,GROOVE_NOTE[k?7:6],65);
    tick++;next=tick*beat/4;
   }
   mix_block(out,CTL);for(unsigned j=0;j<CTL*2;j++)put16(f,(uint16_t)clamp(out[j],-32768,32767));
  }fclose(f);
 }
 printf("Trap kit native checks: %d failures\n",bad);return bad!=0;
}

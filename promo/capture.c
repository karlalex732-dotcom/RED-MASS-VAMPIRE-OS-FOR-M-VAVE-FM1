/* SPDX-License-Identifier: GPL-3.0-only; short musical demos using native firmware DSP. */
#define VAMPIRE_TEST_NO_MAIN 1
#include "../tests/vampire_test.c"
int main(int argc,char **argv){
 if(argc!=4)return 2;unsigned n=atoi(argv[1]);const char *dst=argv[2],*shots=argv[3];
 unsigned k=n<=128?n-1:n-129,e=n<=128?TRAP_ENGINE[k]:CLUB_ENGINE[k],p=n<=128?TRAP_FACTORY[k]:CLUB_FACTORY[k];
 vt_fresh(0);mod_seed=0x2545F491u;palette_set(UI_REDMASS_INDEX);settings.palette=UI_REDMASS_INDEX;
 set_engine_of(TSEL,e);apply_preset_to(TSEL,p);TSEL->engine=TSEL->eng_req;panic_req=0;
 FILE *w=fopen(dst,"wb");if(!w)return 2;unsigned frames=FS*4u;wav_hdr(w,frames);int32_t out[CTL*2];
 unsigned peak=0,over=0,next=0,tick=0,shot=0;int held=-1;
 const uint8_t piano[]={48,55,60,63,67,63,60,55};const uint8_t harp[]={48,60,55,63,55,60,67,63,48,60,55,63,70,67,63,60};
 const uint8_t lead[]={60,60,63,55,60,67,63,58};const uint8_t bell[]={48,55,60,63,48,55,67,63};
 int pad=n==65||n==197;unsigned interval=pad?frames:(n==41?FS/4:FS/2);
 for(unsigned f=0;f<frames;f+=CTL){
  if(pad){if(f==0){trk_note_on(TSEL,48,94);trk_note_on(TSEL,55,88);trk_note_on(TSEL,63,82);}if(f<=FS*3u&&f+CTL>FS*3u){trk_note_off(TSEL,48);trk_note_off(TSEL,55);trk_note_off(TSEL,63);}}
  else if(f>=next&&tick<(n==41?16:8)){if(held>=0)trk_note_off(TSEL,held);held=n==249?piano[tick]:n==41?harp[tick]:n==225?lead[tick]:bell[tick];trk_note_on(TSEL,held,92+(tick%4==0?6:0));tick++;next=tick*interval;}
  if(!pad&&held>=0&&f+CTL>next-interval/4&&f<=next-interval/4){trk_note_off(TSEL,held);held=-1;}
  mix_block(out,CTL);
  if(!shot&&f>FS/8){char name[24];snprintf(name,sizeof name,"preset-%03u",n);screen_save(shots,name);shot=1;}
  for(unsigned i=0;i<CTL&&f+i<frames;i++){int l=out[i*2],r=out[i*2+1];unsigned a=l<0?-l:l,b=r<0?-r:r;if(a>peak)peak=a;if(b>peak)peak=b;over+=l<-32768||l>32767||r<-32768||r>32767;wav_put(w,l,r);}
 }
 fclose(w);printf("Preset %03u peak %u overrange %u\n",n,peak,over);return over!=0;
}

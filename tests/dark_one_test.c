/* SPDX-License-Identifier: GPL-3.0-only
 * One preset per process, so PHYS excitation RNG and any effect history
 * cannot leak between different presets in a before/after comparison. */
#define VAMPIRE_TEST_NO_MAIN 1
#include "vampire_test.c"
int main(int argc,char **argv)
{
    if(argc!=3)return 2;
    uint32_t n=(uint32_t)atoi(argv[1]);if(!n||n>256)return 2;
    uint32_t k=n<=128?n-1:n-129;
    uint32_t e=n<=128?TRAP_ENGINE[k]:CLUB_ENGINE[k];
    uint32_t p=n<=128?TRAP_FACTORY[k]:CLUB_FACTORY[k];
    vt_fresh(0);mod_seed=0x2545F491u;
    set_engine_of(TSEL,e);apply_preset_to(TSEL,p);TSEL->engine=TSEL->eng_req;panic_req=0;
    FILE *w=fopen(argv[2],"wb");if(!w)return 2;wav_hdr(w,FS*12u);
    uint8_t notes[4]={48,55,60,63};if(n<=32)for(uint32_t i=0;i<4;i++)notes[i]-=12;
    for(uint32_t i=0;i<4;i++)trk_note_on(TSEL,notes[i],96);
    int32_t out[CTL*2],peak=0;uint32_t over=0,maxv=0;double energy=0,dc=0;
    for(uint32_t f=0;f<FS*24u;f+=CTL){
        if(f<=FS*3u&&f+CTL>FS*3u)for(uint32_t i=0;i<4;i++)trk_note_off(TSEL,notes[i]);
        mix_block(out,CTL);uint32_t active=0;
        for(uint32_t j=0;j<NVOICE;j++)active+=TSEL->v[j].active;if(active>maxv)maxv=active;
        for(uint32_t i=0;i<CTL && f+i<FS*12u;i++){
            int32_t l=out[2*i],r=out[2*i+1],a=l<0?-l:l,b=r<0?-r:r;
            if(a>peak)peak=a;if(b>peak)peak=b;over+=l<-32768||l>32767||r<-32768||r>32767;
            energy+=(double)l*l+(double)r*r;dc+=l+r;wav_put(w,l,r);
        }
    }
    fclose(w);uint32_t left=0;for(uint32_t j=0;j<NVOICE;j++)left+=TSEL->v[j].active;
    energy=sqrt(energy/(FS*24.));dc/=FS*24.;
    const engine_t *eng=ENGINES[e];uint32_t want=eng->poly&&eng->poly<4u?eng->poly:4u;
    int ok=!over&&peak>350&&peak<32700&&energy>20&&fabs(dc)<350&&maxv==want&&!left;
    printf("{\"number\":%u,\"peak\":%d,\"rms\":%.3f,\"dc\":%.3f,\"overrange\":%u,\"max_voices\":%u,\"remaining\":%u,\"pass\":%s}\n",
        n,peak,energy,dc,over,maxv,left,ok?"true":"false");
    return !ok;
}

/* SPDX-License-Identifier: GPL-3.0-only
 * Every factory patch through actual loading, four-note allocation, native DSP,
 * modulation and FX. Source C is the target implementation, host HAL only. */
#define VAMPIRE_TEST_NO_MAIN 1
#include "vampire_test.c"

static void club_fresh(uint32_t k)
{
    vt_fresh(0); mod_seed = 0x2545F491u;
    set_engine_of(TSEL, CLUB_ENGINE[k]); apply_preset_to(TSEL, CLUB_FACTORY[k]);
    TSEL->engine = TSEL->eng_req; panic_req = 0; undo.trk = 0;
}
static int club_render(const char *dir)
{
    uint32_t k, f, i, j; int bad = 0; uint64_t hashes[CLUB_COUNT];
    int32_t out[CTL*2]; char path[512]; FILE *stats=NULL,*dump=NULL;
    if (dir) {
        snprintf(path,sizeof path,"%s/audio-health.csv",dir);stats=fopen(path,"w");
        snprintf(path,sizeof path,"%s/native-presets.json",dir);dump=fopen(path,"w");
        if(!stats||!dump)return 1;
        fprintf(stats,"number,name,engine,factory,peak,rms,dc,overrange,voices,remaining,hash\n");
        fputc('[',dump);
    }
    for(k=0;k<CLUB_COUNT;k++) {
        club_fresh(k); const engine_t *eng=ENGINES[CLUB_ENGINE[k]];
        const char *name=eng->presets[CLUB_FACTORY[k]].name;
        int bass=0; uint8_t notes[4]={48,55,60,63};
        if(bass)for(j=0;j<4;j++)notes[j]-=12;
        FILE *w=NULL; int32_t peak=0;uint32_t over=0,maxv=0;
        double energy=0,dc=0;uint64_t hash=1469598103934665603ull;
        if(dir) {
            snprintf(path,sizeof path,"%s/%03u.wav",dir,k+129);w=fopen(path,"wb");if(!w)return 1;wav_hdr(w,FS*12u);
            fprintf(dump,"%s{\"name\":\"%s\",\"engine\":%u,\"factory\":%u,\"params\":[",k?",":"",name,CLUB_ENGINE[k],CLUB_FACTORY[k]);
            for(i=0;i<P_COUNT;i++)fprintf(dump,"%s%d",i?",":"",TSEL->p[i]);
            fputc(']',dump);
            if(CLUB_ENGINE[k]==ENGI_FM6) {
                uint8_t pk[FM6_PACKED];fm6_pack(fm6_patch[trk_index(TSEL)],pk);
                fputs(",\"fm6\":[",dump);
                for(i=0;i<FM6_PACKED;i++)fprintf(dump,"%s%u",i?",":"",pk[i]);
                fputc(']',dump);
            }
            fputs("}",dump);
        }
        for(j=0;j<4;j++)trk_note_on(TSEL,notes[j],96);
        for(f=0;f<FS*24u;f+=CTL) {
            if(f<=FS*3u&&f+CTL>FS*3u)for(j=0;j<4;j++)trk_note_off(TSEL,notes[j]);
            mix_block(out,CTL);
            uint32_t active=0;for(j=0;j<NVOICE;j++)active+=TSEL->v[j].active;
            if(active>maxv)maxv=active;
            for(i=0;i<CTL&&f+i<FS*12u;i++) {
                int32_t l=out[2*i],r=out[2*i+1],al=l<0?-l:l,ar=r<0?-r:r;
                if(al>peak)peak=al;if(ar>peak)peak=ar;
                over+=l>32767||l<-32768||r>32767||r<-32768;
                energy+=(double)l*l+(double)r*r;dc+=l+r;
                hash=(hash^(uint16_t)l)*1099511628211ull;hash=(hash^(uint16_t)r)*1099511628211ull;
                if(w)wav_put(w,l,r);
            }
        }
        uint32_t left=0;for(j=0;j<NVOICE;j++)left+=TSEL->v[j].active;
        energy=sqrt(energy/(FS*24.));dc/=FS*24.;
        uint32_t expect=eng->poly&&eng->poly<4u?eng->poly:4u;
        int ok=!over&&peak>350&&peak<32700&&energy>20&&fabs(dc)<350&&maxv==expect&&!left&&TSEL->p[P_VOICE]==V_POLY;
        hashes[k]=hash;for(j=0;j<k;j++)if(hash==hashes[j]){bad++;ok=0;}
        if(!ok)bad++;
        printf("club128: %03u %-13s %-7s peak=%5d rms=%6.1f dc=%6.1f poly=%u left=%u %s\n",k+1,name,eng->name,peak,energy,dc,maxv,left,ok?"ok":"FAIL");
        fflush(stdout);
        if(stats)fprintf(stats,"%u,%s,%u,%u,%d,%.2f,%.2f,%u,%u,%u,%016llx\n",k+1,name,CLUB_ENGINE[k],CLUB_FACTORY[k],peak,energy,dc,over,maxv,left,(unsigned long long)hash);
        if(w)fclose(w);
    }
    if(stats)fclose(stats);if(dump){fputs("]\n",dump);fclose(dump);}
    return bad;
}
int main(int argc,char **argv)
{
    int bad=0;
    bad+=vt_check("Club: 128 unique factory references, all appended after original presets",CLUB_COUNT==128u);
    for(uint32_t k=0;k<CLUB_COUNT;k++) {
        club_fresh(k);
        const engine_t *e=ENGINES[CLUB_ENGINE[k]];
        if(CLUB_FACTORY[k]>=e->npresets||TSEL->p[P_VOICE]!=V_POLY)bad++;
        for(uint32_t i=0;i<8;i++)if(TSEL->p[P_E0+i]<e->edit[i].min||TSEL->p[P_E0+i]>e->edit[i].max)bad++;
        for(uint32_t i=0;i<NELEM(TRAP_IDS);i++)if(TSEL->p[TRAP_IDS[i]]!=CLUB_PARAMS[k][i])bad++;
        int16_t p[P_COUNT];memcpy(p,TSEL->p,sizeof p);project_t pr,back;project_store_t packed;
        project_capture(&pr);
        if(!proj_pack(&packed,&pr)||!proj_import(&back,&packed,sizeof packed)||
            back.t[0].engine!=CLUB_ENGINE[k]||memcmp(p,back.t[0].p,sizeof p))bad++;
        if(k%16==0) {
            int rc=up_store(k/16,e->presets[CLUB_FACTORY[k]].name);set_engine_of(TSEL,0);up_load(k/16);
            if((rc!=0&&rc!=3)||TSEL->eng_req!=CLUB_ENGINE[k]||memcmp(p,TSEL->p,sizeof p))bad++;
        }
    }
    bad+=club_render(argc>1?argv[1]:NULL);
    printf("RED_MASS Club 128: %s (%d failures)\n",bad?"FAILED":"PASS",bad);return bad!=0;
}

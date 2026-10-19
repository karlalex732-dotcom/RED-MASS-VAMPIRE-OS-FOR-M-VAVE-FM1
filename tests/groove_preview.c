/* SPDX-License-Identifier: GPL-3.0-only
 * Actual FM1 sample-kit DSP, no additional audio processing. */
#define GROOVE_TEST_NO_MAIN 1
#include "groove_test.c"
int main(int argc,char **argv)
{
    if(argc!=2)return 2;
    FILE *f=fopen(argv[1],"wb");if(!f)return 2;
    const uint32_t frames=FS/2u;wav_hdr(f,frames*NPAD);
    int32_t out[CTL*2];
    for(uint32_t p=0;p<NPAD;p++) {
        setup();TSEL->p[P_REV]=TSEL->p[P_DLY]=TSEL->p[P_CHOR]=0;
        trk_note_on(TSEL,GROOVE_NOTE[p],100);
        for(uint32_t at=0;at<frames;at+=CTL) {
            mix_block(out,CTL);
            for(uint32_t i=0;i<CTL && at+i<frames;i++)wav_put(f,out[2*i],out[2*i+1]);
        }
    }
    fclose(f);return 0;
}

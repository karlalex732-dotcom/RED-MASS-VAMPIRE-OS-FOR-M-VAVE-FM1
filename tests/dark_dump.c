/* SPDX-License-Identifier: GPL-3.0-only
 * Export the actual native preset loader independently of audio history. */
#define VAMPIRE_TEST_NO_MAIN 1
#include "vampire_test.c"
int main(void)
{
    fputs("[",stdout);
    for(uint32_t n=1;n<=256;n++){
        uint32_t k=n<=128?n-1:n-129,e=n<=128?TRAP_ENGINE[k]:CLUB_ENGINE[k],
          p=n<=128?TRAP_FACTORY[k]:CLUB_FACTORY[k];
        vt_fresh(0);set_engine_of(TSEL,e);apply_preset_to(TSEL,p);TSEL->engine=TSEL->eng_req;
        printf("%s{\"name\":\"%s\",\"engine\":%u,\"factory\":%u,\"params\":[",n==1?"":",",ENGINES[e]->presets[p].name,e,p);
        for(uint32_t i=0;i<P_COUNT;i++)printf("%s%d",i?",":"",TSEL->p[i]);
        fputs("]",stdout);
        if(e==ENGI_FM6){
            uint8_t pk[FM6_PACKED];fm6_pack(fm6_patch[trk_index(TSEL)],pk);fputs(",\"fm6\":[",stdout);
            for(uint32_t i=0;i<FM6_PACKED;i++)printf("%s%u",i?",":"",pk[i]);fputs("]",stdout);
        }
        fputs("}",stdout);
    }
    puts("]");return 0;
}

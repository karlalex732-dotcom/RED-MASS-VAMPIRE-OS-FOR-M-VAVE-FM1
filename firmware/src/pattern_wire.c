/* SPDX-License-Identifier: GPL-3.0-only
 * Audio-boundary reader of the canonical, validated, immutable SRAM song cache.
 * No flash access, hashing, allocation, or project import on this path. */
static void chain_read_steps(uint32_t slot,step_t out[NTRK][NSTEP],const uint8_t micro[NTRK][NSTEP])
{
    const uint8_t *b=proj_slot[slot].raw;uint32_t np=b[66],pos=68u;
    for(uint32_t t=0;t<NTRK;t++) {
        pos+=np+2u;
        for(uint32_t i=0;i<NSTEP;i++) {
            step_t *s=&out[t][i];const uint8_t *v=b+pos;pos+=9u;
            memset(s,0,sizeof *s);
            for(uint32_t j=0;j<4u;j++)s->note[j]=v[j]&127u;
            uint32_t meta=v[4],cond=(v[0]>>7)|((v[1]>>7)<<1)|((meta>>7)<<2)|((v[2]>>7)<<3);
            s->n=meta&7u;s->time=(meta>>3)&3u;s->flags=(meta>>5)&3u;
            s->vel=v[5]&127u;s->hit=v[6];s->acc=v[7];s->probability=v[8]&127u;
            step_set_condition(s,cond);step_set_ratchet(s,1u+(v[5]>>7)+((v[8]>>7)<<1));
            step_set_micro(s,(int32_t)micro[t][i]-7);
        }
    }
    pos+=sizeof(chain_v6_t)+sizeof(motion_store_t);
    for(uint32_t i=0;i<NSTEP;i++) {
        step_t *s=&out[NTRK-1u][i];s->hit|=(uint16_t)b[pos++]<<8;s->acc|=(uint16_t)b[pos++]<<8;
        s->ghost=b[pos]|(uint16_t)b[pos+1u]<<8;pos+=2u;
    }
}

static int16_t chain_wire_param(uint32_t slot,uint32_t track,uint32_t param) {
    const uint8_t *b=proj_slot[slot].raw;return (int16_t)b[68u+track*(b[66]+2u+NSTEP*9u)+param]-64;
}
static void chain_sound_wire(uint32_t slot)
{
    const uint8_t *b=proj_slot[slot].raw;uint32_t np=b[66],pos=68u;
    song.groove=b[65];
    for(uint32_t i=G_DTIME;i<=G_CDEPTH;i++)memcpy(&song.g[i],b+8u+i*2u,2);
    const uint8_t ids[]={G_SWING,G_TUNE,G_RTYPE,G_DRLVL,G_DRREV};
    for(uint32_t i=0;i<sizeof ids;i++)memcpy(&song.g[ids[i]],b+8u+ids[i]*2u,2);
    memcpy(upgrade.pad_chance,chain_chance[slot],sizeof upgrade.pad_chance);
    uint32_t routes=68u+NTRK*(np+2u+NSTEP*9u)+sizeof(chain_v6_t)+sizeof(motion_store_t)+NSTEP*4u;
    for(uint32_t k=0;k<NTRK;k++) {
        track_t *t=&trk[k];uint8_t patch[FP_SIZE+1u];
        for(uint32_t i=0;i<P_COUNT;i++)t->p[i]=(int16_t)b[pos++]-64;
        t->eng_req=b[pos++];t->preset=b[pos++];t->user=0;pos+=NSTEP*9u;
        t->route=b[routes++];t->midi_ch=b[routes++];t->midi_send=b[routes++];
        fm6_unpack(b+(5632u-4u-12u-NTRK*FM6_PACKED)+k*FM6_PACKED,patch);fm6_set_patch(k,patch);
        fm6_slot[k]=(uint8_t)t->p[P_E7];fm6_own_ok|=(uint8_t)(1u<<k);
        memcpy(fm6_own[k],b+(5632u-4u-12u-NTRK*FM6_PACKED)+k*FM6_PACKED,FM6_PACKED);
    }
}

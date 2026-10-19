/* SPDX-License-Identifier: GPL-3.0-only
 * Atomic project extension, with a frozen four-track reader. */
#define UPGRADE_AUX_MAGIC_4 0x4B503034u
#define UPGRADE_AUX_MAGIC 0x4B503035u
#define TRACK_MASK ((1u << NTRK)-1u)
typedef struct {
    uint32_t magic, size;
    chain_config_t chain;
    upgrade_state_t state;
    uint8_t micro[NTRK][NSTEP];
    uint32_t sum;
} upgrade_aux_t;
typedef struct {
    uint8_t valid[4], engine[4][2], blend[4];
    int16_t p[4][2][91]; uint32_t gen;
} upgrade_scenes_4_t;
typedef struct {
    upgrade_scenes_4_t scene;
    upgrade_macro_target_t macro[4][4][4];
    uint8_t macro_value[4][4],pad_chance[4][NPAD],mute_group[8];
} upgrade_state_4_t;
typedef struct {
    uint32_t magic,size;chain_config_t chain;upgrade_state_4_t state;
    uint8_t micro[4][NSTEP];uint32_t sum;
} upgrade_aux_4_t;
_Static_assert(sizeof(upgrade_aux_4_t)==2284u,"frozen four-track extension");
_Static_assert(sizeof(upgrade_aux_t)<=3840u,"project extension fits atomic storage");
#if FELUCCA_FLASH
#define UPGRADE_AUX_BUF st_buf
#else
static uint8_t upgrade_aux_buf[5632] __attribute__((aligned(4)));
#define UPGRADE_AUX_BUF upgrade_aux_buf
static uint8_t upgrade_ram_aux[5][3840];
static uint32_t upgrade_ram_len[5];
#endif
static uint8_t upgrade_aux_was_legacy;
static void upgrade_aux_expand(upgrade_aux_t *a,const upgrade_aux_4_t *input)
{
    upgrade_aux_4_t old;memcpy(&old,input,sizeof old);
    memset(a,0,sizeof *a);a->magic=UPGRADE_AUX_MAGIC;a->size=sizeof *a;a->chain=old.chain;
    memset(a->micro,7,sizeof a->micro);
    for(uint32_t k=0;k<4u;k++) {
        a->state.scene.valid[k]=old.state.scene.valid[k];a->state.scene.blend[k]=old.state.scene.blend[k];
        memcpy(a->state.scene.engine[k],old.state.scene.engine[k],sizeof a->state.scene.engine[k]);
        memcpy(a->state.scene.p[k],old.state.scene.p[k],sizeof old.state.scene.p[k]);
        memcpy(a->state.macro[k],old.state.macro[k],sizeof a->state.macro[k]);
        memcpy(a->state.macro_value[k],old.state.macro_value[k],sizeof a->state.macro_value[k]);
        memcpy(a->state.pad_chance[k],old.state.pad_chance[k],sizeof a->state.pad_chance[k]);
        memcpy(a->micro[k],old.micro[k],NSTEP);
    }
    memcpy(a->state.mute_group,old.state.mute_group,sizeof a->state.mute_group);
    a->sum=proj_hash(a,sizeof *a-4u);
}
static int upgrade_aux_valid(const upgrade_aux_t *a,uint32_t n)
{
    if(n!=sizeof *a||a->magic!=UPGRADE_AUX_MAGIC||a->size!=sizeof *a||
       !a->sum||a->sum!=proj_hash(a,n-4u)||!chain_valid(&a->chain))return 0;
    for(uint32_t k=0;k<NTRK;k++) {
        if(a->state.scene.valid[k]>3u||a->state.scene.blend[k]>100u)return 0;
        for(uint32_t s=0;s<2u;s++)if(a->state.scene.engine[k][s]>=NENGINES)return 0;
        for(uint32_t i=0;i<NSTEP;i++)if(a->micro[k][i]>14u)return 0;
        for(uint32_t i=0;i<NPAD;i++)if(a->state.pad_chance[k][i]>101u)return 0;
        for(uint32_t m=0;m<4u;m++) {
            if(a->state.macro_value[k][m]>127u)return 0;
            for(uint32_t d=0;d<4u;d++)if(a->state.macro[k][m][d].param>=P_COUNT||a->state.macro[k][m][d].amount==-128)return 0;
        }
    }
    for(uint32_t i=0;i<8u;i++)if(a->state.mute_group[i]>TRACK_MASK)return 0;
    return 1;
}
static uint32_t upgrade_aux_wire_sum(const void *b,uint32_t n)
{
    uint32_t sum=0;if(n>=4u)memcpy(&sum,(const uint8_t *)b+n-4u,4);return sum;
}
static int upgrade_aux_wire_valid(const void *b,uint32_t n)
{
    if(n==sizeof(upgrade_aux_t))return upgrade_aux_valid(b,n);
    const upgrade_aux_4_t *o=b;
    if(n!=sizeof *o||o->magic!=UPGRADE_AUX_MAGIC_4||o->size!=n||!o->sum||o->sum!=proj_hash(b,n-4u))return 0;
    upgrade_aux_t a;upgrade_aux_expand(&a,o);return upgrade_aux_valid(&a,sizeof a);
}
static void upgrade_aux_capture(upgrade_aux_t *a)
{
    uint32_t f=motion_guard();memset(a,0,sizeof *a);a->magic=UPGRADE_AUX_MAGIC;a->size=sizeof *a;
    a->chain=chain_config;a->state=upgrade;a->state.scene.gen=0;
    for(uint32_t k=0;k<NTRK;k++)for(uint32_t i=0;i<NSTEP;i++)a->micro[k][i]=(uint8_t)(step_micro(&trk[k].step[i])+7);
    motion_unguard(f);a->sum=proj_hash(a,sizeof *a-4u);
}
#if FELUCCA_FLASH
static int upgrade_aux_copy(uint32_t obj,uint32_t crc)
{
    if(!flash_ok||!crc)return -1;
    for(uint32_t copy=0;copy<2u;copy++) {
        st_hdr_t h;
        if(!st_head(obj,copy,&h)&&!st_body(obj,copy,&h)&&upgrade_aux_wire_valid(UPGRADE_AUX_BUF,h.len)&&upgrade_aux_wire_sum(UPGRADE_AUX_BUF,h.len)==crc)return (int)copy;
    }
    return -1;
}
#endif
/* Keep original wire bytes for backups: the core references their original hash. */
static const uint8_t *upgrade_aux_wire(uint32_t crc,uint32_t *len)
{
    *len=0;if(!crc)return 0;
    for(uint32_t i=0;i<5u;i++) {
#if FELUCCA_FLASH
        int copy=upgrade_aux_copy(OBJ_UPGRADE0+i,crc);
        if(copy>=0){st_hdr_t h;if(st_head(OBJ_UPGRADE0+i,(uint32_t)copy,&h))return 0;*len=h.len;return UPGRADE_AUX_BUF;}
#else
        uint32_t n=upgrade_ram_len[i];
        if(upgrade_aux_wire_valid(upgrade_ram_aux[i],n)&&upgrade_aux_wire_sum(upgrade_ram_aux[i],n)==crc){*len=n;memcpy(UPGRADE_AUX_BUF,upgrade_ram_aux[i],n);return UPGRADE_AUX_BUF;}
#endif
    }
    return 0;
}
static const upgrade_aux_t *upgrade_aux_find(uint32_t crc)
{
    uint32_t n;const uint8_t *b=upgrade_aux_wire(crc,&n);upgrade_aux_was_legacy=0;if(!b)return 0;
    if(n==sizeof(upgrade_aux_4_t)){upgrade_aux_expand((upgrade_aux_t *)UPGRADE_AUX_BUF,(const upgrade_aux_4_t *)b);upgrade_aux_was_legacy=1;}
    return (const upgrade_aux_t *)UPGRADE_AUX_BUF;
}
static uint8_t upgrade_move_mute(uint8_t mask){return (uint8_t)((mask&~8u)|((mask&8u)<<2));}
static const upgrade_aux_t *upgrade_aux_for_project(const project_t *p)
{
    upgrade_aux_t *a=(upgrade_aux_t *)upgrade_aux_find(p->aux_crc);
    if(!a||!upgrade_aux_was_legacy||!p->legacy_drum)return a;
    upgrade_state_t *s=&a->state;uint32_t d=NTRK-1u;
    s->scene.valid[d]=s->scene.valid[3];s->scene.valid[3]=0;
    s->scene.blend[d]=s->scene.blend[3];s->scene.blend[3]=0;
    memcpy(s->scene.engine[d],s->scene.engine[3],sizeof s->scene.engine[d]);memset(s->scene.engine[3],0,sizeof s->scene.engine[3]);
    memcpy(s->scene.p[d],s->scene.p[3],sizeof s->scene.p[d]);memset(s->scene.p[3],0,sizeof s->scene.p[3]);
    memcpy(s->macro[d],s->macro[3],sizeof s->macro[d]);memset(s->macro[3],0,sizeof s->macro[3]);
    memcpy(s->macro_value[d],s->macro_value[3],sizeof s->macro_value[d]);memset(s->macro_value[3],0,sizeof s->macro_value[3]);
    memcpy(s->pad_chance[d],s->pad_chance[3],sizeof s->pad_chance[d]);memset(s->pad_chance[3],0,sizeof s->pad_chance[3]);
    memcpy(a->micro[d],a->micro[3],NSTEP);memset(a->micro[3],7,NSTEP);
    for(uint32_t i=0;i<8u;i++)s->mute_group[i]=upgrade_move_mute(s->mute_group[i]);
    for(uint32_t i=0;i<CHAIN_ROWS;i++)a->chain.row[i].mute=upgrade_move_mute(a->chain.row[i].mute);
    a->sum=proj_hash(a,sizeof *a-4u);return a;
}
static int upgrade_aux_apply(project_t *p,int runtime)
{
    const upgrade_aux_t *a=upgrade_aux_for_project(p);if(p->aux_crc&&!a)return 1;
    if(a){p->chain=a->chain;for(uint32_t k=0;k<NTRK;k++)for(uint32_t i=0;i<NSTEP;i++)step_set_micro(&p->t[k].step[i],(int32_t)a->micro[k][i]-7);}
    (void)runtime;p->sum=proj_sum(p);return 0;
}
static uint32_t upgrade_core_aux(const uint8_t *b)
{
    uint32_t magic,crc=0;memcpy(&magic,b,4);
    if(magic==PROJ_MAGIC)memcpy(&crc,b+PROJ_AUX_OFF,4);
    else if(magic==PROJ_MAGIC_V10)memcpy(&crc,b+PROJ_STORE_V10-4u-PROJ_NAME_LEN-4u*FM6_PACKED-4u,4);
    return crc;
}
static int upgrade_aux_store_raw(uint32_t slot,const void *a,uint32_t n)
{
    if(slot>4u||!upgrade_aux_wire_valid(a,n))return 1;
#if FELUCCA_FLASH
    int protect=-1;if(!flash_ok)return 1;
    if(slot<4u){st_hdr_t h;if(st_current(OBJ_PROJECT0+slot,&h)>=0&&(h.len==PROJ_STORE_SIZE||h.len==PROJ_STORE_V10))protect=upgrade_aux_copy(OBJ_UPGRADE0+slot,upgrade_core_aux(UPGRADE_AUX_BUF));}
    return st_save_to(OBJ_UPGRADE0+slot,a,n,protect>=0?1-protect:-1);
#else
    memcpy(upgrade_ram_aux[slot],a,n);upgrade_ram_len[slot]=n;return 0;
#endif
}
static int upgrade_aux_store(uint32_t slot,const upgrade_aux_t *a){return upgrade_aux_store_raw(slot,a,sizeof *a);}
static int upgrade_aux_save(uint32_t slot,project_t *p)
{
    upgrade_aux_t *a=(upgrade_aux_t *)proj_wire_u.raw;upgrade_aux_capture(a);p->aux_crc=a->sum;return upgrade_aux_store(slot,a);
}
static void upgrade_aux_restore_state(const upgrade_aux_t *a)
{
    uint32_t gen=vampire_scene.gen;if(a)upgrade=a->state;else memset(&upgrade,0,sizeof upgrade);
    vampire_scene.gen=gen+1u;upgrade_gen++;
}

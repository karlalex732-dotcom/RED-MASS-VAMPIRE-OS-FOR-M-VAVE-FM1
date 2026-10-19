/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSIST_TEST_NO_MAIN 1
#include "persistence_test.c"
#include "legacy_wire.h"
static void version(uint32_t value)
{
    memset(&upgrade,0,sizeof upgrade);chain_defaults(&chain_config);
    set_engine_of(&trk[NTRK-1u],ENGI_SMPKIT);apply_preset_to(&trk[NTRK-1u],0);
    trk[0].step[0]=(step_t){{(uint8_t)value},1,ST_NOTE,0,96};
    step_set_micro(&trk[0].step[0],(int32_t)value%15-7);
    step_set_condition(&trk[0].step[0],COND_THREE4);step_set_ratchet(&trk[0].step[0],4);
    upgrade_set_pad_chance(NTRK-1u,0,value%101);upgrade.mute_group[0]=(uint8_t)(value%64);
    upgrade.scene.valid[0]=3;upgrade.scene.engine[0][0]=upgrade.scene.engine[0][1]=trk[0].eng_req;
    upgrade.scene.p[0][0][P_DIST]=(int16_t)value;upgrade.scene.p[0][1][P_DIST]=127;
    upgrade.macro[0][0][0]=(upgrade_macro_target_t){P_DIST,63,(int16_t)value};upgrade.macro_value[0][0]=(uint8_t)value;
    chain_config.count=32;chain_config.rsv[0]=3;chain_config.rsv[1]=3;chain_config.rsv[2]=2;
    for(uint32_t i=0;i<32;i++)chain_config.row[i]=(chain_row_t){(uint8_t)(i%4),64,(uint8_t)(i%64),(uint8_t)(i+1),(uint16_t)(120+i)};
    trk[NTRK-1u].step[5]=(step_t){{0},0,ST_NOTE,0,99,0x8181,0x8000,0,0,0x0181};
    song.g[G_DRLVL]=31;song.g[G_DRREV]=43;
}
static int restored(uint32_t v)
{
    return trk[0].step[0].note[0]==v&&step_micro(&trk[0].step[0])==(int32_t)v%15-7&&step_condition(&trk[0].step[0])==COND_THREE4&&step_ratchet(&trk[0].step[0])==4&&
      upgrade_pad_chance(NTRK-1u,0)==v%101&&upgrade.mute_group[0]==v%64&&upgrade.scene.valid[0]==3&&upgrade.scene.p[0][0][P_DIST]==v&&upgrade.macro[0][0][0].base==v&&upgrade.macro_value[0][0]==v&&
      chain_config.count==32&&chain_config.row[31].repeat==64&&chain_config.row[31].length==32&&chain_config.row[31].bpm==151&&chain_config.rsv[0]==3&&
      trk[NTRK-1u].step[5].ghost==0x181&&trk[NTRK-1u].step[5].hit==0x8181&&song.g[G_DRLVL]==31&&song.g[G_DRREV]==43;
}
static void cold_load(void)
{
    memset(&upgrade,0,sizeof upgrade);memset(proj_slot,0,sizeof proj_slot);host_tracks_init();flash_ok=1;
    /* Reload the simulated NOR, without probing real JEDEC / sample XIP. */
    for(uint32_t i=0;i<4;i++)proj_fetch(i);
    project_load(0);
}
static uint8_t baseline[0x100000];
static int atomic(void)
{
    int bad=0,ok=1;reset();version(60);project_save(0);memcpy(baseline,nor,sizeof nor);
    cold_load();bad+=check("FUN10 cold boot restores all 32 rows, scenes, macros, drum chances and timing",restored(60));
    uint32_t tested=0;
    for(int cut=0;cut<40;cut++) {
        memcpy(nor,baseline,sizeof nor);fail_after=-1;cold_load();version(70);fail_after=cut;
        int saved=project_save(0)==0;fail_after=-1;cold_load();
        ok &= restored(saved ? 70 : 60);tested++;
        if(saved)break;
        /* After an orphan extension, another failing attempt must still protect
         * the extension referenced by the last fully committed core. */
        version(80);fail_after=1;project_save(0);fail_after=-1;cold_load();ok &= restored(60);
        version(80);project_save(0);cold_load();ok &= restored(80);
    }
    printf("power-cut programming positions checked: %u\n",tested);
    bad+=check("Every interrupted extension/core page write and failed retry leaves a complete old/new project",ok&&tested>20);
    memcpy(nor,baseline,sizeof nor);fail_after=-1;cold_load();st_hdr_t h;int cp=st_current(OBJ_UPGRADE0,&h);
    if(cp>=0)nor[st_sector(OBJ_UPGRADE0,cp)+ST_PAYLOAD_OFF+32]^=1;
    version(90);project_load(0);bad+=check("Missing/corrupt extension is rejected without replacing current music",trk[0].step[0].note[0]==90&&str_eq(ui.msg,"PROJECT FEATURES LOST"));
    return bad;
}
static void fm1_wdt_feed(void){}
static uint8_t ed_smp_buf[512];
static uint32_t ed_unpack7(const uint8_t *a,uint32_t na,uint8_t *out,uint32_t max){uint32_t n=0;while(na&&n<max){uint32_t m=*a++,k=(--na)>7 ? 7 : na;if(!k||n+k>max||(m>>k))return 0;for(uint32_t i=0;i<k;i++,na--)out[n++]=*a++|((m>>i)&1u)<<7;}return na ? 0 : n;}
static uint32_t ed_flash_stop(void){return transport_busy() ? 3 : 0;}
enum{ED_BACKUP_LIST=65,ED_BACKUP_GET,ED_BACKUP_PUT};
#include "../firmware/src/editor_backup.c"
static void wire32(uint8_t *p,uint32_t v){for(uint32_t i=0;i<5;i++)p[i]=(uint8_t)((v>>(7*i))&127u);}
static uint32_t wire_put(uint8_t id,const uint8_t *p,uint32_t len)
{
 uint8_t a[302]={0,id};wire32(a+2,len);wire32(a+7,st_crc32(p,len));uint32_t rc=ed_bk_write(a,12);
 for(uint32_t off=0;!rc&&off<len;){
  uint32_t count=len-off>256?256:len-off,k=7;a[0]=1;wire32(a+2,off);
  for(uint32_t i=0;i<count;){uint32_t g=count-i>7?7:count-i,m=0;for(uint32_t j=0;j<g;j++)m|=(uint32_t)(p[off+i+j]>>7)<<j;a[k++]=(uint8_t)m;for(uint32_t j=0;j<g;j++)a[k++]=p[off+i+j]&127u;i+=g;}
  rc=ed_bk_write(a,k);off+=count;
 }
 a[0]=2;return rc?rc:ed_bk_write(a,2);
}
static int old_wire_backup(void)
{
 int bad=0;uint8_t core[3840],aux[2284];FILE *f=fopen("tests/fixtures/v04-drum-core.bin","rb");if(!f)exit(2);size_t cn=fread(core,1,sizeof core,f);fclose(f);
 f=fopen("tests/fixtures/v04-drum-features.bin","rb");if(!f)exit(2);size_t an=fread(aux,1,sizeof aux,f);fclose(f);
 reset();memset(&upgrade,0,sizeof upgrade);
 bad+=check("Original 0.4 backup passes BEGIN/chunk/COMMIT validation",cn==sizeof core&&an==sizeof aux&&!wire_put(11,aux,sizeof aux)&&!wire_put(2,core,sizeof core));
 project_load(0);
 bad+=check("Protocol restore migrates kit, pad 16, motion, scenes and mute mask to T6",song.sel==5&&trk[5].eng_req==ENGI_SMPKIT&&trk[5].step[63].hit==0x8000&&motion.event[0].place==383&&upgrade.scene.blend[5]==78&&upgrade.pad_chance[5][15]==37&&chain_config.row[31].mute==32);
 bad+=check("Original runtime backup restores through the same protocol",!wire_put(10,aux,sizeof aux)&&!wire_put(0,core,sizeof core)&&song.sel==5&&upgrade.scene.blend[5]==78);
 uint8_t a[12]={0,0};wire32(a+2,5633);bad+=check("Invalid backup size is rejected before staging",ed_bk_write(a,sizeof a)==1);
 return bad;
}
static int backups(void)
{
 int bad=0;uint32_t n;static project_store_t current;static upgrade_aux_t live,saved;
 reset();version(61);project_save(0);version(73);bad+=check("Backup captures a current feature hash and lists five extension objects",!ed_bk_capture()&&ED_BK_N==18);
 memcpy(&current,ED_BK_RAW,sizeof current);const uint8_t *p=ed_bk_object(10,&n);if(!p)return bad+1;memcpy(&live,p,n);
 p=ed_bk_object(11,&n);if(!p)return bad+1;memcpy(&saved,p,n);
 bad+=check("Backup distinguishes unsaved live features from saved project features",live.sum!=saved.sum&&upgrade_core_aux(current.raw)==live.sum);
 memset(&upgrade,0,sizeof upgrade);memset(proj_slot,0,sizeof proj_slot);memset(nor,255,sizeof nor);
 ed_bk_id=10;ed_bk_len=ed_bk_pos=sizeof live;memcpy(ED_BK_RAW,&live,sizeof live);ed_bk_crc=st_crc32(ED_BK_RAW,sizeof live);
 bad+=check("Backup feature restore validates and commits the runtime extension",!ed_bk_commit());
 ed_bk_id=0;ed_bk_len=ed_bk_pos=sizeof current;memcpy(ED_BK_RAW,&current,sizeof current);ed_bk_crc=st_crc32(ED_BK_RAW,sizeof current);
 bad+=check("Backup runtime restore recalls all feature state through its linked hash",!ed_bk_commit()&&restored(73));
 version(60);project_capture(&proj_scratch);proj_pack(&current,&proj_scratch);
 project_store_t old=current; legacy_wire(current.raw,PROJ_STORE_V10,PROJ_MAGIC_V9,&old,1);
 memset(&upgrade,0x01,sizeof upgrade);
 bad+=check("Real FUN9 migration retains 16-pad data, conditions, FM6 and DUST/DUCK",proj_import(&proj_scratch,&current,PROJ_STORE_V10)&&!proj_scratch.aux_crc&&project_restore_runtime(&proj_scratch)==0&&trk[NTRK-1u].step[5].ghost==0x181&&song.g[G_DRLVL]==31&&song.g[G_DRREV]==43&&upgrade_pad_chance(NTRK-1u,0)==100&&upgrade.scene.valid[0]==0);
 return bad;
}
int main(void){int bad=atomic()+backups()+old_wire_backup();printf("RED_MASS upgrade persistence: %s (%d failures)\n",bad ? "FAIL" : "PASS",bad);return !!bad;}

/* SPDX-License-Identifier: GPL-3.0-only */
#define PERSIST_TEST_NO_MAIN 1
#include "persistence_test.c"
static int bad;
static void verify(const char *name,int ok){printf("six tracks: %s %s\n",name,ok?"ok":"FAIL");bad+=!ok;}
static uint32_t fixture(const char *path,uint8_t *out,uint32_t max){FILE *f=fopen(path,"rb");if(!f)exit(2);uint32_t n=(uint32_t)fread(out,1,max,f);fclose(f);return n;}
static void migrate(const char *kind,int drums){
 uint8_t core[3840],ext[3840];char path[128];reset();memset(&upgrade,0,sizeof upgrade);
 snprintf(path,sizeof path,"tests/fixtures/v04-%s-core.bin",kind);uint32_t cn=fixture(path,core,sizeof core);
 snprintf(path,sizeof path,"tests/fixtures/v04-%s-features.bin",kind);uint32_t en=fixture(path,ext,sizeof ext);
 verify("original four-track fixtures accepted",cn==3840&&en==2284&&upgrade_aux_wire_valid(ext,en));
 verify("old atomic objects seeded",!st_save(OBJ_PROJECT0,core,cn)&&!upgrade_aux_store_raw(0,ext,en));
 memset(proj_slot,0,sizeof proj_slot);proj_fetch(0);project_load(0);
 uint32_t d=drums?5u:3u;
 verify(drums?"legacy kit and selected track migrate to T6":"legacy synth T4 stays on T4",song.sel==d&&trk[d].eng_req==(drums?ENGI_SMPKIT:0u));
 verify("notes, pad 16, ghosts, condition and microtiming retained",drums?(trk[d].step[63].hit==(1u<<15)&&trk[d].step[63].ghost==(1u<<15)&&step_condition(&trk[d].step[63])==11u&&step_micro(&trk[d].step[63])==-3):trk[d].step[63].note[0]==72u);
 verify("motion track and nine-bit step owner retained",motion.count==2u&&motion.event[0].place==d*64u+63u&&motion.on==(1u<<d));
 verify("route, scenes, macros and chance migrate together",trk[d].route==1u&&trk[d].midi_ch==10u&&trk[d].midi_send==2u&&upgrade.scene.valid[d]==3u&&upgrade.scene.blend[d]==78u&&upgrade.macro_value[d][0]==67u&&upgrade.pad_chance[d][15]==37u);
 verify("full 32-row arrangement and mute masks retained",chain_config.count==32u&&chain_config.row[31].mute==(1u<<d)&&upgrade.mute_group[0]==(1u<<d));
 verify("new tracks do not inherit old drum events",drums?trk[3].step[63].time==ST_REST:trk[5].step[63].time==ST_REST);
 verify("migrated project saves in new format",!project_save(0));
 memset(proj_slot,0,sizeof proj_slot);memset(&upgrade,0,sizeof upgrade);memset(&motion,0,sizeof motion);proj_fetch(0);project_load(0);
 verify("cold reload preserves the complete migration",song.sel==d&&motion.event[0].place==d*64u+63u&&upgrade.scene.blend[d]==78u&&step_micro(&trk[d].step[63])==-3&&chain_config.row[31].mute==(1u<<d));
}
static void roundtrip(void){
 reset();memset(&upgrade,0,sizeof upgrade);song.groove=1;set_engine_of(&trk[5],ENGI_SMPKIT);song.sel=5;
 for(uint32_t k=0;k<NTRK;k++){
  trk[k].step[63].time=ST_NOTE;trk[k].step[63].n=1;trk[k].step[63].note[0]=(uint8_t)(60u+k);trk[k].step[63].vel=90;
  step_set_micro(&trk[k].step[63],(int32_t)k-3);step_set_condition(&trk[k].step[63],k+1u);trk[k].p[P_LEVEL]=(int16_t)(87+k);
 }
 motion.count=2;motion.on=48;motion.event[0]=(motion_event_t){319,P_LEVEL,91};motion.event[1]=(motion_event_t){383,P_PAN,21};
 trk[5].step[63].hit=0x8000;trk[5].step[63].ghost=0x8000;
 verify("six-track project commits",!project_save(2));memset(proj_slot,0,sizeof proj_slot);proj_fetch(2);project_load(2);
 int ok=song.sel==5&&motion.on==48&&motion.event[0].place==319&&motion.event[1].place==383;
 for(uint32_t k=0;k<NTRK;k++)ok&=trk[k].step[63].note[0]==60+k&&step_micro(&trk[k].step[63])==(int32_t)k-3&&trk[k].p[P_LEVEL]==87+k;
 verify("all six tracks and high motion positions survive cold reload",ok);
 chain_config.count=1;chain_config.row[0]=(chain_row_t){2,1,32,64,124};chain_config.rsv[0]=CHAIN_SOUNDS;
 verify("song accepts mute bit for T6",chain_valid(&chain_config));verify("six-track song prepares",!chain_prepare());chain_start();
 verify("song cache decodes T6 pad 16 and microtiming",chain.step[5][63].hit==0x8000&&chain.step[5][63].ghost==0x8000&&step_micro(&chain.step[5][63])==2);
 verify("song applies T5/T6 sounds and mutes",trk[4].p[P_LEVEL]==91&&trk[5].p[P_LEVEL]==92&&trk[5].p[P_MUTE]);chain_stop();transport_req=0;
}
static void midi_six(void){reset();song.groove=1;song.g[G_ROUTE]=0;
 verify("groove MIDI channels 1..5 and 10",midi_track(4)==&trk[4]&&midi_track(9)==&trk[5]);
 midi_event(0x90,4,72,100);verify("channel 5 reaches T5",midi_notes[4][72]==5u);midi_event(0x80,4,72,0);
 midi_event(0x90,9,36,100);verify("channel 10 reaches drum T6",(midi_notes[9][36]&127u)==6u);midi_event(0x80,9,36,0);
 midi_event(0x90,5,74,100);verify("unused groove channel ignored",!midi_notes[5][74]);song.groove=0;verify("normal MIDI channel 6 reaches T6",midi_track(5)==&trk[5]);}
int main(void){migrate("drum",1);migrate("synth",0);roundtrip();midi_six();printf("six-track tests: %s\n",bad?"FAILED":"passed");return bad?1:0;}

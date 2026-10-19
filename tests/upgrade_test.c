/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
static void fresh(void){ui_power_on();memset(&upgrade_clip,0,sizeof upgrade_clip);memset(&ug_ui,0,sizeof ug_ui);ug_ui.page=1;ug_ui.density=100;ug_ui.octave=3;ug_ui.range=2;ug_ui.all_chance=100;ug_ui.lock_param=P_DIST;}
static void tool(uint32_t k){if(k==UG_LAUNCH)upgrade_open_toolbox();else upgrade_launch(k);ui.act=0;}
static int panel_tools(void)
{
 int bad=0,ok;uint32_t i,total;step_t copy[NSTEP];
 fresh();TSEL->p[P_SLEN]=32;
 for(i=0;i<32;i++)TSEL->step[i]=(step_t){{(uint8_t)(48+i)},1,ST_NOTE,0,96};
 step_set_micro(&TSEL->step[2],-3);motion_set_event(TSEL,2,P_DIST,67);
 tool(UG_COPY);turn(EN_K2,1);press(B_OCTUP);memcpy(copy,trk[0].step,sizeof copy);
 track_select(1);set_engine_of(TSEL,0);turn(EN_K3,1);press(B_OCTUP);
 bad+=check("COPY TRACK: notes, timing, enabled locks and microtiming on another track",!memcmp(copy,TSEL->step,sizeof copy)&&TSEL->p[P_SLEN]==32&&motion_count(TSEL)==1&&(motion.on&2)&&motion.event[motion.count-1].place==66);
 hold(B_SAVE);bad+=check("COPY TRACK: existing pattern-load undo restores destination",TSEL->step[0].time==ST_REST&&TSEL->p[P_SLEN]==16);
 track_select(0);tool(UG_COPY);turn(EN_K1,1);turn(EN_K4,1);turn(EN_K2,1);press(B_OCTUP);track_select(1);turn(EN_K4,1);turn(EN_K3,1);press(B_OCTUP);
 bad+=check("COPY PAGE: page 2 into previously empty page 3 extends LEN to 48",TSEL->p[P_SLEN]==48&&TSEL->step[32].note[0]==64&&ug_ui.page==3);
 fresh();for(i=0;i<NTRK;i++)trk[i].p[P_DIST]=(int16_t)(i*7);
 tool(UG_CTRL);song.playing=1;turn(EN_K2,23);ok=1;for(i=0;i<NTRK;i++)ok &= trk[i].p[P_DIST]==(int32_t)i*7+23;
 turn(EN_K4,1);press(B_OCTUP);for(i=0;i<NTRK;i++)ok &= trk[i].p[P_DIST]==(int32_t)i*7;
 bad+=check("CTRL ALL + RELOAD: live per-track offsets and exact restoration",ok);song.playing=0;
 fresh();tool(UG_LOCK);turn(EN_K1,2);turn(EN_K3,53-ug_ui.lock_value);press(B_OCTUP);
 bad+=check("Manual locks: real knobs PUT and enable an event on selected step",motion.count==1&&motion.on==1&&motion.event[0].place==2&&motion.event[0].param==P_DIST&&motion.event[0].value==53);
 turn(EN_K4,1);press(B_OCTUP);bad+=check("Manual locks: DELETE preserves other steps",!motion.count);
 fresh();TSEL->p[P_E4]=32;TSEL->p[P_DIST]=11;
 upgrade.macro[0][0][0]=(upgrade_macro_target_t){P_E4,63,32};upgrade.macro[0][0][1]=(upgrade_macro_target_t){P_DIST,-31,11};
 tool(UG_MACRO);turn(EN_K1,127);
 bad+=check("Macros: one knob controls multiple positive and negative targets",TSEL->p[P_E4]==95&&TSEL->p[P_DIST]==0&&upgrade.macro_value[0][0]==127);
 turn(EN_K1,-127);bad+=check("Macros: returning to zero restores captured bases",TSEL->p[P_E4]==32&&TSEL->p[P_DIST]==11);
 song.rec=1;turn(EN_K1,1);bad+=check("Macros: recording excludes ambiguous macro automation",!upgrade.macro_value[0][0]);song.rec=0;
 fresh();TSEL->p[P_ROOT]=3;TSEL->p[P_SCALE]=7;tool(UG_MELODY);press(B_OCTUP);memcpy(copy,TSEL->step,sizeof copy);ok=1;
 for(i=0;i<16;i++)ok &= TSEL->step[i].n==1 && scale_snap(TSEL,TSEL->step[i].note[0])==TSEL->step[i].note[0];
 hold(B_SAVE);press(B_OCTUP);bad+=check("Melody: deterministic seed, harmonic-minor root and reversible generation",ok&&!memcmp(copy,TSEL->step,sizeof copy));
 fresh();int16_t before[P_COUNT];memcpy(before,TSEL->p,sizeof before);tool(UG_MUTATE);turn(EN_K1,35);turn(EN_K3,1);press(B_OCTUP);
 bad+=check("Mutation: changes timbre, preserving level, transposition and tuning",memcmp(before,TSEL->p,sizeof before)&&before[P_LEVEL]==TSEL->p[P_LEVEL]&&before[P_ED_PIT]==TSEL->p[P_ED_PIT]&&before[P_LD_PIT]==TSEL->p[P_LD_PIT]&&before[P_E0]==TSEL->p[P_E0]);
 turn(EN_K4,1);press(B_OCTUP);
 bad+=check("Mutation: sound undo returns every parameter",!memcmp(before,TSEL->p,sizeof before));
 fresh();tool(UG_BANK);turn(EN_K1,1);preset_pos(&total);bad+=check("Browser: VAMP256 contains exactly 256 curated factory sounds",total==256);
 turn(EN_K1,2);preset_pos(&total);bad+=check("Browser: NEW128 contains exactly 128 new sounds",total==128);
 turn(EN_K1,4);preset_pos(&total);bad+=check("Browser: eight sampled grand piano variations",total==8);
 fresh();tool(UG_MUTES);trk[0].p[P_MUTE]=trk[2].p[P_MUTE]=1;turn(EN_K3,1);press(B_OCTUP);trk[0].p[P_MUTE]=trk[2].p[P_MUTE]=0;song.playing=1;turn(EN_K4,1);press(B_OCTUP);
 bad+=check("Mute groups: capture and recall a four-track mask while playing",upgrade.mute_group[0]==5&&trk[0].p[P_MUTE]&&trk[2].p[P_MUTE]&&!trk[1].p[P_MUTE]);song.playing=0;
 fresh();tool(UG_MICRO);turn(EN_K2,-7);bad+=check("Microtiming: real knob sets -7/32 without losing condition",step_micro(&TSEL->step[0])==-7);
 tool(UG_HISTORY);turn(EN_K1,1);press(B_OCTUP);bad+=check("Step history: microtiming undo",!step_micro(&TSEL->step[0]));turn(EN_K2,1);press(B_OCTUP);bad+=check("Step history: explicit redo",step_micro(&TSEL->step[0])==-7);
 for(i=0;i<20;i++){upgrade_note_before(TSEL,1);TSEL->step[1].vel=(uint8_t)(i+1);upgrade_note_after();}ok=upgrade_history.count==16;
 for(i=0;i<16;i++)ok &= !upgrade_history_move(0);ok &= TSEL->step[1].vel==4&&upgrade_history_move(0);
 for(i=0;i<16;i++)ok &= !upgrade_history_move(1);bad+=check("Step history: 16-entry bound, oldest eviction and full redo",ok&&TSEL->step[1].vel==20);
 upgrade_history_move(0);upgrade_note_before(TSEL,3);TSEL->step[3].vel=97;upgrade_note_after();bad+=check("Step history: a new edit removes the redo branch",upgrade_history_move(1));
 fresh();tool(UG_LAUNCH);turn(EN_K1,3);press(B_OCTUP);bad+=check("TOOLBOX: real knobs open MACROS",cur_page()->graph==GR_UPGRADE&&cur_page()->id[0]==UG_MACRO);
 return bad;
}
static int six_clipboard(void) {
 int bad=0;fresh();set_engine_of(&trk[5],ENGI_SMPKIT);set_engine_of(&trk[3],ENGI_SMPKIT);track_select(5);
 TSEL->step[63]=(step_t){{0},0,ST_NOTE,0,96,0x8000,0,0,0,0x8000};
 motion_set_event(TSEL,63,P_DIST,45);ug_ui.scope=0;upgrade_copy();
 track_select(3);step_t before=TSEL->step[63];
 bad+=check("16-pad paste refuses T4 before modifying the pattern",upgrade_paste()&&!memcmp(&before,&TSEL->step[63],sizeof before));
 track_select(5);track_defaults_steps(TSEL);motion_clear(TSEL);
 bad+=check("T6 paste retains pad 16, ghost and nine-bit automation owner",!upgrade_paste()&&TSEL->step[63].hit==0x8000&&TSEL->step[63].ghost==0x8000&&motion.count==1&&motion.event[0].place==383);
 track_select(0);set_engine_of(TSEL,0);TSEL->step[63]=(step_t){{72},1,ST_NOTE,0,96};motion_clear(TSEL);motion_set_event(TSEL,63,P_PAN,22);upgrade_copy();
 track_select(4);set_engine_of(TSEL,0);
 bad+=check("T5 paste retains nine-bit automation owner",!upgrade_paste()&&motion_count(TSEL)==1&&motion.event[motion.count-1].place==319);
 return bad;
}
static int timing(void)
{
 int bad=0,ok=1;uint32_t at[8],ns;
 for(int polarity=0;polarity<2;polarity++) {
  fresh();track_t *t=TSEL;t->p[P_SLEN]=2;t->p[P_VOICE]=V_POLY;
  for(uint32_t i=0;i<2;i++)t->step[i]=(step_t){{(uint8_t)(60+i)},1,ST_NOTE,0,96};
  step_set_micro(&t->step[0],polarity ? -7 : 7);step_set_micro(&t->step[1],polarity ? 7 : -7);
  uint32_t d=step_samples(t,div_samples(t->p[P_SDIV]),0),oldage=vage;ns=0;seq_start();
  for(uint32_t n=0;n<d*3u;n++){seq_tick(t,1);if(vage!=oldage){if(ns<8)at[ns]=n;ns++;oldage=vage;}}
  if(polarity){ok &= ns==3&&at[0]==0&&at[1]==d+d*7/32&&at[2]==2*d-d*7/32;}
  else {ok &= ns==4&&at[0]==d*7/32&&at[1]==d-d*7/32&&at[2]==2*d+d*7/32;}
  if(!ok)printf("timing polarity %d d=%u n=%u at=%u,%u,%u\n",polarity,d,ns,at[0],at[1],at[2]);seq_stop();
 }
 bad+=check("Microtiming: exact early/late offsets, first-event clipping and wrap",ok);
 fresh();track_t *t=TSEL;t->p[P_SLEN]=2;t->step[0]=(step_t){{60},1,ST_NOTE,0,96};t->step[1]=(step_t){{61},1,ST_NOTE,0,96};
 step_set_micro(&t->step[0],7);step_set_micro(&t->step[1],-7);step_set_ratchet(&t->step[0],4);uint32_t d=step_samples(t,div_samples(t->p[P_SDIV]),0),oldage=vage;ns=0;seq_start();
 for(uint32_t n=0;n<d;n++){seq_tick(t,1);if(vage!=oldage){if(ns<8)at[ns]=n;ns++;oldage=vage;}}
 bad+=check("Microtiming + ratchet: four hits fit before an early following step",ns==5&&at[3]<at[4]);seq_stop();
 fresh();track_select(NTRK-1u);set_engine_of(TSEL,ENGI_SMPKIT);TSEL->engine=TSEL->eng_req;TSEL->p[P_SLEN]=1;TSEL->step[0]=(step_t){{0},0,ST_NOTE,0,100,3,1};step_set_ratchet(&TSEL->step[0],4);upgrade_set_pad_chance(NTRK-1u,0,0);upgrade_set_pad_chance(NTRK-1u,1,100);ns=0;d=step_samples(TSEL,div_samples(TSEL->p[P_SDIV]),0);oldage=vage;seq_start();
 for(uint32_t n=0;n<d;n++){seq_tick(TSEL,1);ns+=vage-oldage;oldage=vage;}
 bad+=check("Drum pad chance: kick 0%, snare 100%, one decision across four ratchets",ns==4&&TSEL->seq_notes[0]==GROOVE_NOTE[1]);seq_stop();
 return bad;
}
static int song_and_pool(void)
{
 int bad=0,ok;uint32_t i,k;int32_t out[2*CTL];int16_t original[P_COUNT];
 fresh();
 for(k=0;k<2;k++){
  track_select(0);set_engine_of(TSEL,k ? ENGI_FM6 : 0);apply_preset_to(TSEL,0);
  for(i=0;i<NTRK;i++){trk[i].p[P_SLEN]=4;trk[i].p[P_MUTE]=(i!=0);trk[i].step[0]=(step_t){{(uint8_t)(60+5*k)},1,ST_NOTE,0,96};}
  upgrade_set_pad_chance(NTRK-1u,0,25+25*k);song.g[G_RTYPE]=(int16_t)k;project_save(k);
 }
 set_engine_of(TSEL,0);apply_preset_to(TSEL,0);TSEL->p[P_SLEN]=11;TSEL->p[P_PAN]=-15;song.g[G_BPM]=127;song.g[G_RTYPE]=0;up_store(0,"SONG ORIG");up_load(0);
 memcpy(original,TSEL->p,sizeof original);chain_defaults(&chain_config);chain_config.count=2;
 chain_config.rsv[0]=CHAIN_SOUNDS;chain_config.row[0]=(chain_row_t){0,1,0,1,140};chain_config.row[1]=(chain_row_t){1,1,0,1,85};song.rec=3;
 ok=chain_prepare()==0;mix_block(out,CTL);ok &= chain.running&&chain.row==0&&trk[0].eng_req==0&&song.g[G_BPM]==140&&upgrade_pad_chance(NTRK-1u,0)==25;
 for(i=0;i<FS*2u/CTL && chain.running&&chain.row==0;i++)mix_block(out,CTL);
 ok &= chain.running&&chain.row==1&&trk[0].eng_req==ENGI_FM6&&song.g[G_BPM]==85&&upgrade_pad_chance(NTRK-1u,0)==50&&song.g[G_RTYPE]==1&&!trk[0].user;
 for(i=0;i<16;i++)mix_block(out,CTL);
 uint32_t sounding=0;for(i=0;i<NVOICE;i++)sounding|=trk[0].v[i].active&&trk[0].v[i].note==65;
 bad+=check("Song PROJECT sounds: row engine, first note, tempo and pad chances switch",ok&&trk[0].engine==ENGI_FM6&&sounding);
 for(i=0;i<FS*3u/CTL && chain.running;i++)mix_block(out,CTL);
 bad+=check("Song STOP: full starting sound, user identity, timing, tempo and REC arms return",!chain.running&&!song.playing&&!memcmp(original,TSEL->p,sizeof original)&&song.g[G_BPM]==127&&song.rec==3&&song.g[G_RTYPE]==0&&TSEL->user==1);
 chain_defaults(&chain_config);chain_config.count=32;chain_config.rsv[0]=CHAIN_LOOP;chain_config.rsv[1]=30;chain_config.rsv[2]=31;
 for(i=0;i<32;i++)chain_config.row[i]=(chain_row_t){(uint8_t)(i%2),1,0,1,0};
 chain_config.row[30].repeat=64;chain_config.row[31].mute=1;ok=chain_prepare()==0;events_block(1);ok &= chain.row==30&&chain.remaining==64;
 uint32_t d=step_samples(TSEL,div_samples(TSEL->p[P_SDIV]),0);
 for(i=0;i<64;i++)events_block(d);
 ok &= chain.row==31&&TSEL->p[P_MUTE]==1;
 events_block(d);ok &= chain.row==31&&chain.remaining==1;
 bad+=check("Song 32 rows: START 31, 64 repeats, row mutes and LOOP FROM 32",ok);seq_stop();
 fresh();memset(phys_owner,0,sizeof phys_owner);memset(phys_map,0,sizeof phys_map);
 for(k=0;k<NTRK;k++){set_engine_of(&trk[k],ENGI_PHYS);apply_preset_to(&trk[k],1);trk[k].engine=ENGI_PHYS;}
 for(k=0;k<NTRK;k++)for(i=0;i<3;i++)trk_note_on(&trk[k],48+3*k+i,100);
 uint32_t voices=0,seen=0;ok=1;
 for(k=0;k<NTRK;k++)for(i=0;i<PHYS_POLY;i++)if(trk[k].v[i].active&&trk[k].v[i].stage!=4u){uint32_t slot=phys_map[k][i];voices++;ok &= slot<NVOICE&&phys_owner[slot]==1+k*PHYS_POLY+i&&!(seen&(1u<<slot));seen|=1u<<slot;}
 for(i=0;i<128;i++)mix_block(out,CTL);
 for(k=0;k<NTRK;k++)trk_note_on(&trk[k],72+k,100);
 for(i=0;i<128;i++)mix_block(out,CTL);
 for(k=0;k<NTRK;k++){uint32_t heard=0;for(i=0;i<PHYS_POLY;i++)heard |= trk[k].v[i].active&&trk[k].v[i].note==72+k;ok &= heard;}
 bad+=check("PHYS pool: four tracks claim eight distinct states, steal and retrigger",ok&&voices==8);
 fresh();TSEL->step[0]=(step_t){{60},1,ST_NOTE,0,90};motion_set_event(TSEL,0,P_DIST,55);tool(UG_COPY);turn(EN_K2,1);press(B_OCTUP);track_select(1);
 for(i=0;i<MOTION_MAX-1;i++)motion_set_event(&trk[2],i,P_DIST,(int16_t)i);
 motion_set_event(&trk[0],1,P_REV,4);step_t unchanged=TSEL->step[0];uint32_t cnt=motion.count;
 turn(EN_K3,1);press(B_OCTUP);
 bad+=check("COPY: full automation pool refuses before changing destination",motion.count==cnt&&!memcmp(&unchanged,&TSEL->step[0],sizeof unchanged)&&msg_is("AUTOMATION FULL"));
 return bad;
}
int main(void){int bad=panel_tools()+six_clipboard()+timing()+song_and_pool();printf("RED_MASS upgrade UI + timing: %s (%d failures)\n",bad ? "FAIL" : "PASS",bad);return !!bad;}

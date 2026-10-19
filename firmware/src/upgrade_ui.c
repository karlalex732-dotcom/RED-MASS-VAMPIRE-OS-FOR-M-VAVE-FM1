/* SPDX-License-Identifier: GPL-3.0-only
 * RED_MASS 0.4 panel tools. Four labelled knobs, OCT+ applies actions. */
#include "vampire256_browser.h"
static uint8_t upgrade_category;
static int upgrade_bank_accept(uint32_t e, uint32_t p)
{
    if (!upgrade_category) return 1;
    uint32_t tag=e<NENGINES && p<128u ? BANK_CATEGORY[e][p] : 0u;
    return upgrade_category==1u ? tag!=0u : upgrade_category==2u ? (tag&16u)!=0u :
           upgrade_category==3u ? (tag&32u)!=0u : (tag&15u)==upgrade_category;
}
static const char *const UG_CATEGORIES[]={"ALL","VAMP256","OLD128","NEW128","PLUCK","PAD","LEAD","PIANO","BASS","KEYS","BELL","CHOIR","STRING","ORGAN","PERC","FX"};
static const char *const UG_SCOPE[]={"TRACK","PAGE"};
static const char *const UG_END[]={"STOP","LOOP"};
static const char *const UG_SOUND[]={"KEEP","PROJ"};
static const char *const UG_LOCK_OP[]={"PUT","DELETE"};
static const char *const UG_FEATURES[]={"COPY","CTRLALL","LOCKS","MACROS","MACSET","MELODY","MUTATE","BANK256","MUTES","DRUMS","MICRO","HISTORY","SONGROW","SONGSET","SCENES"};
static const uint8_t UG_CTRL_IDS[]={P_DIST,P_CHOR,P_DLY,P_REV,P_ATK,P_DEC,P_SUS,P_REL,P_PAN,P_LEVEL};
static const char *const UG_LABEL[UG_N][4]={
    {"SCOPE","COPY","PASTE","PAGE"},{"PARAM","AMT","CAP","RELOAD"},
    {"STEP","PARAM","VALUE","OP"},{"MAC 1","MAC 2","MAC 3","MAC 4"},
    {"MAC","TARGET","PARAM","AMT"},{"DENS","OCT","RANGE","SEED"},
    {"AMT","SEED","APPLY","UNDO"},{"CAT","SOUND","FAV","FAVS"},
    {"GROUP","MUTES","CAP","RECALL"},{"PAD","CHANCE","ALL","APPLY"},
    {"STEP","SHIFT","ZERO","HUMAN"},{"UNDO","REDO","DEPTH","CLEAR"},
    {"ROW","LEN","BPM","MUTES"},{"SOUNDS","START","LOOP","END"},
    {"TOOL","OPEN","",""}
};
static struct {
    int16_t scope, ctrl, amount, lock_param, lock_value, lock_op, mac, target;
    int16_t density, octave, range, seed, mutate, group, all_chance, human, tool, page;
} ug_ui={0,0,0,P_DIST,0,0,0,0,55,3,2,0,24,0,100,2,0,1};
static int16_t ug_view[4];
static param_desc_t ug_desc;
static uint8_t upgrade_ctrl_valid;
static uint16_t upgrade_ctrl_touched;
static int16_t upgrade_ctrl_base[NTRK][P_COUNT] __attribute__((section(".auxpool")));
static struct {
    step_t step[NSTEP]; motion_store_t motion;
    int16_t timing[4];
    uint8_t valid, engine, start, count;
} upgrade_clip;

static const param_desc_t *upgrade_page_desc(const page_t *pg, uint32_t c, int16_t **vp)
{
    uint32_t kind=pg->id[0], row=ui.song_row<CHAIN_ROWS ? ui.song_row : CHAIN_ROWS-1u;
    int32_t value=0, lo=0, hi=127; uint32_t fmt=F_INT; const char *const *names=0;
    if (c>3u || kind>=UG_N) { *vp=0; return 0; }
    switch(kind) {
    case UG_COPY: if(c==1u||c==2u) { *vp=0;return 0; } value=c ? ug_ui.page : ug_ui.scope; hi=c ? 4 : 1; lo=c ? 1 : 0; names=c ? 0 : UG_SCOPE; break;
    case UG_CTRL: if(c>1u) { *vp=0;return 0; } value=c ? ug_ui.amount : ug_ui.ctrl; lo=c ? -63 : 0; hi=c ? 63 : (int32_t)NELEM(UG_CTRL_IDS)-1; break;
    case UG_LOCK: value=c==0u ? ui.cursor+1u : c==1u ? ug_ui.lock_param : c==2u ? ug_ui.lock_value : ug_ui.lock_op;
        if(!c){lo=1;hi=TSEL->p[P_SLEN];} else if(c==1u){hi=P_COUNT-1;} else if(c==2u){ug_desc=*track_desc(TSEL,(uint32_t)ug_ui.lock_param);ug_desc.label="VALUE";ug_view[c]=(int16_t)value;*vp=&ug_view[c];return &ug_desc;} else{hi=1;names=UG_LOCK_OP;} break;
    case UG_MACRO: value=upgrade.macro_value[song.sel][c]; break;
    case UG_MACSET: { const upgrade_macro_target_t *m=&upgrade.macro[song.sel][ug_ui.mac][ug_ui.target];
        value=c==0u ? ug_ui.mac+1 : c==1u ? ug_ui.target+1 : c==2u ? m->param : m->amount;
        if(c<2u){lo=1;hi=4;} else if(c==2u)hi=P_COUNT-1; else lo=-127; break; }
    case UG_MELODY: value=c==0u ? ug_ui.density : c==1u ? ug_ui.octave : c==2u ? ug_ui.range : ug_ui.seed;
        hi=c==0u ? 100 : c==1u ? 7 : c==2u ? 3 : 127; lo=c==2u ? 1 : 0; break;
    case UG_MUTATE: if(c>1u){*vp=0;return 0;} value=c ? ug_ui.seed : ug_ui.mutate; hi=c ? 127 : 64; break;
    case UG_BANK: if(!c){value=upgrade_category;hi=15;names=UG_CATEGORIES;} else if(c==1u){uint32_t total;value=(int32_t)preset_pos(&total)+1;lo=1;hi=total ? (int32_t)total : 1;} else{value=c==2u ? preset_favorite() : favorites.filter;hi=1;names=N_ONOFF;} break;
    case UG_MUTES: if(c>1u){*vp=0;return 0;} value=c ? upgrade.mute_group[ug_ui.group] : ug_ui.group+1;lo=c ? 0 : 1;hi=c ? (1u << NTRK)-1u : 8;break;
    case UG_DRUM: value=c==0u ? ui.lane+1u : c==1u ? upgrade_pad_chance(song.sel,ui.lane) : ug_ui.all_chance;
        if(c==3u){*vp=0;return 0;}lo=c ? 0 : 1;hi=c ? 100 : 16;break;
    case UG_MICRO: if(c==2u){*vp=0;return 0;}value=c==0u ? ui.cursor+1u : c==1u ? step_micro(&TSEL->step[ui.cursor]) : ug_ui.human;
        lo=c==0u ? 1 : c==1u ? -7 : 0;hi=c==0u ? TSEL->p[P_SLEN] : 7;break;
    case UG_HISTORY: if(c!=2u){*vp=0;return 0;}value=upgrade_history.pos;hi=UPGRADE_HISTORY;break;
    case UG_SONGROW: value=c==0u ? row+1u : c==1u ? chain_config.row[row].length : c==2u ? chain_config.row[row].bpm : chain_config.row[row].mute;
        lo=c==0u ? 1 : 0;hi=c==0u ? CHAIN_ROWS : c==1u ? NSTEP : c==2u ? 300 : (1u << NTRK)-1u;break;
    case UG_SONGSET: value=c==0u ? !!(chain_config.rsv[0]&CHAIN_SOUNDS) : c==1u ? chain_config.rsv[1]+1u : c==2u ? chain_config.rsv[2]+1u : !!(chain_config.rsv[0]&CHAIN_LOOP);
        lo=c==1u||c==2u ? 1 : 0;hi=lo ? CHAIN_ROWS : 1; names=c==0u ? UG_SOUND : c==3u ? UG_END : 0;break;
    case UG_LAUNCH: if(c){*vp=0;return 0;}value=ug_ui.tool;hi=14;names=UG_FEATURES;break;
    }
    ug_desc=(param_desc_t){UG_LABEL[kind][c],names ? F_ENUM : (uint8_t)fmt,(int16_t)lo,(int16_t)hi,0,names,0};
    ug_view[c]=(int16_t)value;*vp=&ug_view[c];return &ug_desc;
}
static uint32_t upgrade_action_mask(const page_t *pg)
{
    switch(pg->id[0]){
    case UG_COPY:return 6u;case UG_CTRL:return 12u;case UG_LOCK:case UG_MELODY:case UG_DRUM:return 8u;
    case UG_MUTATE:case UG_MUTES:return 12u;case UG_MICRO:return 12u;case UG_HISTORY:return 11u;
    case UG_SONGROW:case UG_SONGSET:return 1u;case UG_LAUNCH:return 2u;default:return 0;
    }
}
static uint32_t upgrade_action_col(const page_t *pg)
{
    if(pg->id[0]==UG_LOCK||pg->id[0]==UG_MELODY||pg->id[0]==UG_DRUM)return 4u;
    if(pg->id[0]==UG_SONGROW||pg->id[0]==UG_SONGSET)return 1u;
    if(pg->id[0]==UG_LAUNCH)return 2u;
    return ui.act;
}
static const char *upgrade_action_name(const page_t *pg,uint32_t c)
{
    uint32_t k=pg->id[0];
    if(k==UG_LOCK)return ug_ui.lock_op ? "DELETE" : "PUT";
    if(k==UG_MELODY||k==UG_DRUM||(k==UG_MUTATE&&c==2u))return "APPLY";
    if(k==UG_MICRO)return c==2u ? "ZERO" : "HUMAN";
    if(k==UG_HISTORY)return c==0u ? "UNDO" : c==1u ? "REDO" : "CLEAR";
    if(k==UG_SONGROW||k==UG_SONGSET)return song.playing||chain_busy() ? "STOP" : "PLAY";
    return UG_LABEL[k][c&3u];
}
static int upgrade_action_ready(const page_t *pg,uint32_t c)
{
    uint32_t k=pg->id[0];
    if(k==UG_LAUNCH)return 1;
    if(k==UG_SONGROW||k==UG_SONGSET)return chain_config.count||song.playing||chain_busy();
    if(k==UG_MUTES)return 1;
    if(k==UG_CTRL)return !chain_busy()&&!song.rec&&(c==2u||upgrade_ctrl_valid);
    if(song.playing||chain_busy()||song.rec)return 0;
    if(k==UG_COPY)return c==1u||upgrade_clip.valid;
    if(k==UG_HISTORY)return c==0u ? upgrade_history.pos!=0u : c==1u ? upgrade_history.pos<upgrade_history.count : upgrade_history.count!=0u;
    return 1;
}
static void upgrade_lock_read(void)
{
    ug_ui.lock_value=motion_base_value(TSEL,(uint32_t)ug_ui.lock_param);
    for(uint32_t i=0;i<motion.count;i++)if(motion.event[i].place==((song.sel<<6)|ui.cursor)&&motion.event[i].param==ug_ui.lock_param){ug_ui.lock_value=motion.event[i].value;break;}
}
static uint32_t upgrade_param_next(track_t *t,uint32_t id,int32_t step,int locks)
{
    for(uint32_t n=0;n<P_COUNT;n++){
        id=(id+(step>0 ? 1u : P_COUNT-1u))%P_COUNT;
        if(locks ? motion_param(id) : upgrade_sound_param(t,id))return id;
    }
    return P_ATK;
}
static void upgrade_ctrl_capture(void)
{
    uint32_t f=motion_guard();
    for(uint32_t k=0;k<NTRK;k++)for(uint32_t i=0;i<P_COUNT;i++)upgrade_ctrl_base[k][i]=motion_base_value(&trk[k],i);
    upgrade_ctrl_valid=1;upgrade_ctrl_touched=0;ug_ui.amount=0;motion_unguard(f);
}
static void upgrade_edit(const page_t *pg,uint32_t c,int32_t steps)
{
    int16_t *vp;const param_desc_t *d=upgrade_page_desc(pg,c,&vp);uint32_t kind=pg->id[0];
    if(!d||!vp){ui.act=steps>0 ? (uint8_t)(c+1u) : 0u;return;}
    int32_t v=param_turn(d,*vp,steps);
    if(chain_busy() && kind!=UG_BANK && kind!=UG_MUTES && kind!=UG_LAUNCH){ui_message("STOP TO EDIT");return;}
    switch(kind){
    case UG_COPY:if(!c)ug_ui.scope=(int16_t)v;else ug_ui.page=(int16_t)v;break;
    case UG_CTRL:if(!c){ug_ui.ctrl=(int16_t)v;ug_ui.amount=0;}else{
        for(uint32_t k=0;k<NTRK;k++)if(motion_enabled(&trk[k])&&motion_count(&trk[k])){ui_message("TURN AUTOMATION OFF");return;}
        if(song.rec){ui_message("DISARM REC FIRST");return;}if(!upgrade_ctrl_valid)upgrade_ctrl_capture();
        ug_ui.amount=(int16_t)v;uint32_t id=UG_CTRL_IDS[ug_ui.ctrl],f=motion_guard();
        for(uint32_t k=0;k<NTRK;k++)trk[k].p[id]=(int16_t)param_fit(&TP[id],upgrade_ctrl_base[k][id]+v);
        upgrade_ctrl_touched|=(uint16_t)(1u<<ug_ui.ctrl);motion_unguard(f);
        }break;
    case UG_LOCK:if(!c){cursor_set(v-1);upgrade_lock_read();}else if(c==1u){ug_ui.lock_param=(int16_t)upgrade_param_next(TSEL,ug_ui.lock_param,steps,1);upgrade_lock_read();}else if(c==2u)ug_ui.lock_value=(int16_t)v;else ug_ui.lock_op=(int16_t)v;break;
    case UG_MACRO:if(upgrade_macro_set(TSEL,c,(uint32_t)v))ui_message("DISARM / MOTION OFF");break;
    case UG_MACSET:if(song.playing||song.rec){ui_message("STOP / DISARM FIRST");return;}if(c==0u)ug_ui.mac=(int16_t)(v-1);else if(c==1u)ug_ui.target=(int16_t)(v-1);else{
        upgrade_macro_rebase(TSEL);upgrade_macro_target_t *m=&upgrade.macro[song.sel][ug_ui.mac][ug_ui.target];
        if(c==2u)m->param=(uint8_t)upgrade_param_next(TSEL,m->param,steps,0);else m->amount=(int8_t)v;
        m->base=motion_base_value(TSEL,m->param);
        }break;
    case UG_MELODY:if(!c)ug_ui.density=(int16_t)v;else if(c==1u)ug_ui.octave=(int16_t)v;else if(c==2u)ug_ui.range=(int16_t)v;else ug_ui.seed=(int16_t)v;break;
    case UG_MUTATE:if(!c)ug_ui.mutate=(int16_t)v;else ug_ui.seed=(int16_t)v;break;
    case UG_BANK:if(!c)upgrade_category=(uint8_t)v;else if(c==1u)preset_step(steps>0 ? 1 : -1);else if(c==2u)preset_mark(v!=0);else{favorites.filter=(uint32_t)v;settings_save();}break;
    case UG_MUTES:if(!c)ug_ui.group=(int16_t)(v-1);else upgrade.mute_group[ug_ui.group]=(uint8_t)v;break;
    case UG_DRUM:if(!c){ui.lane=(uint8_t)(v-1);groove_grid_bank=ui.lane&8u;}else if(c==1u)upgrade_set_pad_chance(song.sel,ui.lane,(uint32_t)v);else ug_ui.all_chance=(int16_t)v;break;
    case UG_MICRO:if(!c)cursor_set(v-1);else if(c==1u){if(song.playing){ui_message("STOP TO EDIT");return;}upgrade_note_before(TSEL,ui.cursor);step_set_micro(&TSEL->step[ui.cursor],v);upgrade_note_after();TSEL->micro_mode=0;}else{ug_ui.human=(int16_t)v;ui.act=4;}break;
    case UG_HISTORY:break;
    case UG_SONGROW:if(!c)ui.song_row=(uint8_t)(v-1);else{uint32_t r=ui.song_row;chain_row_t *row=&chain_config.row[r];if(r>=chain_config.count)chain_config.count=(uint8_t)(r+1u);if(c==1u)row->length=(uint8_t)v;else if(c==2u)row->bpm=(uint16_t)(v==0 ? 0 : v<20 ? steps>0 ? 20 : 0 : v);else row->mute=(uint8_t)v;}break;
    case UG_SONGSET:if(c==0u)chain_config.rsv[0]=(uint8_t)((chain_config.rsv[0]&~CHAIN_SOUNDS)|(v ? CHAIN_SOUNDS : 0));else if(c==1u||c==2u)chain_config.rsv[c]=(uint8_t)(v-1);else chain_config.rsv[0]=(uint8_t)((chain_config.rsv[0]&~CHAIN_LOOP)|(v ? CHAIN_LOOP : 0));break;
    case UG_LAUNCH:ug_ui.tool=(int16_t)v;break;
    }
    upgrade_gen++;ui.force=1;
}
static int upgrade_copy(void)
{
    uint32_t begin=ug_ui.scope ? (uint32_t)(ug_ui.page-1)*16u : 0u,count=ug_ui.scope ? 16u : NSTEP,n=0;
    motion_snapshot_track(TSEL,&upgrade_clip.motion);
    for(uint32_t i=0;i<upgrade_clip.motion.count;i++){
        motion_event_t e=upgrade_clip.motion.event[i];uint32_t step=e.place&63u;
        if(step>=begin&&step<begin+count){e.place=(uint8_t)(step-begin);upgrade_clip.motion.event[n++]=e;}
    }
    upgrade_clip.motion.count=(uint8_t)n;
    for(uint32_t i=0;i<count;i++){upgrade_clip.step[i]=TSEL->step[begin+i];if(ug_ui.scope&&begin+i>=(uint32_t)TSEL->p[P_SLEN]){memset(&upgrade_clip.step[i],0,sizeof(step_t));upgrade_clip.step[i].time=ST_REST;}}
    memcpy(upgrade_clip.timing,&TSEL->p[P_SLEN],sizeof upgrade_clip.timing);
    upgrade_clip.engine=TSEL->eng_req;upgrade_clip.start=(uint8_t)begin;upgrade_clip.count=(uint8_t)count;upgrade_clip.valid=1;return 0;
}
static int upgrade_paste(void)
{
    if(!upgrade_clip.valid)return 1;
    uint32_t begin=upgrade_clip.count==NSTEP ? 0u : (uint32_t)(ug_ui.page-1)*16u,n=0,k=song.sel;
    motion_store_t keep;
    motion_snapshot_track(TSEL,&keep);
    for(uint32_t i=0;i<keep.count;i++)if((keep.event[i].place&63u)<begin||(keep.event[i].place&63u)>=begin+upgrade_clip.count)keep.event[n++]=keep.event[i];
    keep.count=(uint8_t)n;
    if(motion.count-motion_count(TSEL)+n+upgrade_clip.motion.count>MOTION_MAX){ui_message("AUTOMATION FULL");return 1;}
    for(uint32_t i=0;i<upgrade_clip.motion.count;i++)if(upgrade_clip.engine!=TSEL->eng_req&&upgrade_clip.motion.event[i].param>=P_FM1_ATK){ui_message("LOCKS NEED SAME ENG");return 1;}
    if(k!=NTRK-1u)for(uint32_t i=0;i<upgrade_clip.count;i++)if((upgrade_clip.step[i].hit|upgrade_clip.step[i].acc)>255u||upgrade_clip.step[i].ghost){ui_message("16 PADS NEED TRACK 6");return 1;}
    undo.keep=0;load_begin(TSEL,UNDO_PAT);
    memcpy(TSEL->step+begin,upgrade_clip.step,upgrade_clip.count*sizeof(step_t));
    if(upgrade_clip.count==NSTEP)memcpy(&TSEL->p[P_SLEN],upgrade_clip.timing,sizeof upgrade_clip.timing);
    else if(TSEL->p[P_SLEN]<(int32_t)(begin+upgrade_clip.count))TSEL->p[P_SLEN]=(int16_t)(begin+upgrade_clip.count);
    for(uint32_t i=0;i<upgrade_clip.motion.count;i++){motion_event_t e=upgrade_clip.motion.event[i];e.place=(uint16_t)((k<<6)|((e.place&63u)+begin));keep.event[keep.count++]=e;}
    keep.on=(uint8_t)((upgrade_clip.count==NSTEP ? !!upgrade_clip.motion.on : !!keep.on || !!upgrade_clip.motion.on) ? 1u<<k : 0u);
    (void)motion_replace_track(TSEL,&keep);load_end(TSEL);TSEL->micro_mode=0;return 0;
}
static int upgrade_melody(void)
{
    if(drum_track(TSEL)){ui_message("SYNTH TRACK ONLY");return 1;}
    uint8_t notes[48];uint32_t count=0,mask=scale_mask(TSEL),seed=0x4B415253u^((uint32_t)ug_ui.seed+1u)*2654435761u;
    int32_t root=12*(ug_ui.octave+1)+TSEL->p[P_ROOT];
    for(uint32_t i=0;i<(uint32_t)ug_ui.range*12u;i++)if((mask>>(i%12u))&1u)if(root+(int32_t)i<=127)notes[count++]=(uint8_t)(root+i);
    if(!count){ui_message("LOWER OCTAVE");return 1;}
    undo.keep=0;load_begin(TSEL,UNDO_PAT);memset(TSEL->step,0,sizeof TSEL->step);
    for(uint32_t i=0;i<NSTEP;i++){
        step_t *s=&TSEL->step[i];s->time=ST_REST;
        if(i<(uint32_t)TSEL->p[P_SLEN]&&vampire_rand(&seed)%100u<(uint32_t)ug_ui.density){s->time=ST_NOTE;s->n=1;s->note[0]=notes[vampire_rand(&seed)%count];s->vel=(uint8_t)(80u+vampire_rand(&seed)%33u);}
    }
    load_end(TSEL);TSEL->micro_mode=0;return 0;
}
static int upgrade_mutate(void)
{
    uint32_t seed=0x564D5554u^((uint32_t)ug_ui.seed+1u)*2654435761u;
    undo.keep=0;load_begin(TSEL,UNDO_SOUND);
    for(uint32_t i=0;i<P_COUNT;i++){
        const param_desc_t *d=track_desc(TSEL,i);
        if(!upgrade_sound_param(TSEL,i)||param_kept(i)||d->fmt==F_SEMI||d->fmt==F_OCT||i==P_ED_PIT||i==P_LD_PIT||i==P_TRANS)continue;
        if(i>=P_E0&&(str_eq(d->label,"TUNE")||str_eq(d->label,"PIT")||str_eq(d->label,"INT")||str_eq(d->label,"SLOT")))continue;
        int32_t depth=(d->max-d->min)*ug_ui.mutate/127;
        int32_t delta=(int32_t)(vampire_rand(&seed)%(uint32_t)(2*depth+1))-depth;
        TSEL->p[i]=(int16_t)param_fit(d,TSEL->p[i]+delta);
    }
    load_end(TSEL);return 0;
}
static void upgrade_launch(uint32_t kind)
{
    for(uint32_t i=0;i<NPAGES;i++)if((kind<UG_LAUNCH&&PAGES[i].graph==GR_UPGRADE&&PAGES[i].id[0]==kind)||(kind==UG_LAUNCH&&PAGES[i].graph==GR_VSCENE)){
        ui.page=(uint8_t)i;ui.home=0;ui.menu=0;page_entered();return;
    }
}
static void upgrade_action(const page_t *pg,uint32_t c)
{
    uint32_t kind=pg->id[0],f;int rc=0;
    if(!upgrade_action_ready(pg,c)){ui_message("STOP / DISARM FIRST");return;}
    switch(kind){
    case UG_COPY:rc=c==1u ? upgrade_copy() : upgrade_paste();if(!rc)ui_message(c==1u ? "PATTERN COPIED" : "PATTERN PASTED");break;
    case UG_CTRL:if(c==2u){upgrade_ctrl_capture();ui_message("CTRL ALL CAPTURED");}else{f=motion_guard();for(uint32_t k=0;k<NTRK;k++)for(uint32_t i=0;i<NELEM(UG_CTRL_IDS);i++)if((upgrade_ctrl_touched>>i)&1u)trk[k].p[UG_CTRL_IDS[i]]=upgrade_ctrl_base[k][UG_CTRL_IDS[i]];motion_unguard(f);upgrade_ctrl_valid=0;ug_ui.amount=0;ui_message("CTRL ALL RELOADED");}break;
    case UG_LOCK:if(!ug_ui.lock_op){rc=(int)motion_set_event(TSEL,ui.cursor,(uint32_t)ug_ui.lock_param,ug_ui.lock_value);ui_message(rc ? "AUTOMATION FULL" : "STEP LOCK STORED");}else{motion_delete_event(TSEL,ui.cursor,(uint32_t)ug_ui.lock_param);ui_message("STEP LOCK DELETED");}break;
    case UG_MELODY:if(!upgrade_melody())ui_message("MELODY GENERATED");break;
    case UG_MUTATE:if(c==2u){if(!upgrade_mutate())ui_message("SOUND MUTATED");}else undo_swap();break;
    case UG_MUTES:if(c==2u){uint32_t mask=0;for(uint32_t k=0;k<NTRK;k++)mask|=(uint32_t)!!trk[k].p[P_MUTE]<<k;upgrade.mute_group[ug_ui.group]=(uint8_t)mask;ui_message("MUTE GROUP CAPTURED");}else{f=motion_guard();for(uint32_t k=0;k<NTRK;k++)trk[k].p[P_MUTE]=(int16_t)((upgrade.mute_group[ug_ui.group]>>k)&1u);motion_unguard(f);ui_message("MUTE GROUP RECALLED");}break;
    case UG_DRUM:for(uint32_t i=0;i<NPAD;i++)upgrade_set_pad_chance(song.sel,i,(uint32_t)ug_ui.all_chance);ui_message("DRUM CHANCE SET");break;
    case UG_MICRO:if(c==2u){upgrade_note_before(TSEL,ui.cursor);step_set_micro(&TSEL->step[ui.cursor],0);upgrade_note_after();ui_message("STEP ON GRID");}else{uint32_t seed=0x4D494352u^((uint32_t)ug_ui.seed+1u)*2654435761u;undo.keep=0;load_begin(TSEL,UNDO_PAT);for(uint32_t i=0;i<(uint32_t)TSEL->p[P_SLEN];i++)step_set_micro(&TSEL->step[i],(int32_t)(vampire_rand(&seed)%(uint32_t)(ug_ui.human*2+1))-ug_ui.human);load_end(TSEL);TSEL->micro_mode=0;ui_message("TIMING HUMANIZED");}break;
    case UG_HISTORY:if(c==3u){upgrade_history_clear();ui_message("HISTORY CLEARED");}else{rc=upgrade_undo_ui(c==1u);}break;
    case UG_SONGROW:case UG_SONGSET:if(song.playing||chain_busy())transport_req=2;else chain_play_ui();break;
    case UG_LAUNCH:upgrade_launch((uint32_t)ug_ui.tool);break;
    }
    upgrade_gen++;ui.force=1;
}

static int upgrade_undo_ui(int redo)
{
    if(upgrade_history_move(redo)){ui_message("NOTHING TO CHANGE");return 1;}
    const upgrade_edit_t *v=&upgrade_history.edit[redo ? upgrade_history.pos-1u : upgrade_history.pos];
    track_select(v->track);cursor_set(v->step);ui.force=1;
    ui_message(redo ? "STEP EDIT REDONE" : "STEP EDIT UNDONE");return 0;
}
static void upgrade_open_toolbox(void)
{
    for(uint32_t i=0;i<NPAGES;i++)if(PAGES[i].graph==GR_UPGRADE&&PAGES[i].id[0]==UG_LAUNCH){ui.page=(uint8_t)i;ui.home=0;ui.menu=0;page_entered();return;}
}

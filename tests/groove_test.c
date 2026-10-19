/* SPDX-License-Identifier: GPL-3.0-only
 * Integrated host checks for the actual RED_MASS groove firmware paths. */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
#include "legacy_wire.h"
static int failures;
static void gcheck(const char *name, int ok)
{ printf("groove: %-66s %s\n",name,ok ? "ok" : "FAIL"); failures += !ok; }
static void setup(void)
{
    ui_power_on(); song.master_q12=4096; song.groove=1;
    groove_fill_held=0;
    track_select(NTRK-1u); set_engine_of(TSEL,ENGI_SMPKIT); TSEL->engine=TSEL->eng_req; panic_req=0;
    mo_r=mo_w=0;
    usb.config=1;
}
static int32_t render_peak(uint32_t blocks)
{
    int32_t peak=0, out[CTL*2];
    for (uint32_t b=0;b<blocks;b++) {
        mix_block(out,CTL);
        for (uint32_t i=0;i<CTL*2u;i++) { int32_t a=out[i]<0 ? -out[i] : out[i]; if (a>peak) peak=a; }
    }
    return peak;
}
static void kits(void)
{
    uint32_t mask=0; int32_t max=0;
    for (uint32_t p=0;p<NPAD;p++) {
        setup(); trk_note_on(TSEL,GROOVE_NOTE[p],100);
        int32_t peak=render_peak(FS/CTL);
        mask |= (uint32_t)(peak>100) << p;
        if (peak>max) max=peak;
        uint32_t active=0; for (uint32_t v=0;v<NVOICE;v++) active+=TSEL->v[v].active;
        gcheck(GROOVE_PAD[p],peak>100 && peak<=32767 && !active);
    }
    gcheck("16 original PCM pads sound and finish without key-up",mask==65535u && max<32767);
    setup(); TSEL->p[P_E0]=1; trk_note_on(TSEL,36,100);
    int32_t dry[CTL];track_render(TSEL,dry,CTL);uint32_t silent=1;
    for(uint32_t i=0;i<CTL;i++)silent &= dry[i]==0;
    gcheck("empty user drum kit is silent (no sine fallback)",silent);
    setup(); TSEL->p[P_SCALE]=7; song.octave=3; TSEL->p[P_CHRD]=3;
    uint32_t pads=0;
    for (uint32_t k=0;k<27u;k++) if (!key_black(k)) pads |= 1u << (uint32_t)groove_pad(kb_map(TSEL,k));
    gcheck("16 white keys map all pads independently of scale/chord/octave",pads==65535u && chord_build(TSEL,36,kb_chord[0])==1u);
}
static void patterns(void)
{
    setup(); go_page(GR_ROLL);
    for (uint32_t p=0;p<NPAD;p++) grid_hit(TSEL,0,p,1);
    grid_acc(TSEL,0,15,1); grid_acc(TSEL,0,15,0);
    TSEL->step[0].ghost=1u<<8; TSEL->step[0].acc=1u<<15;
    gcheck("grid edits preserve all 16 hit bits",TSEL->step[0].hit==65535u && TSEL->step[0].acc==(1u<<15));
    TSEL->seq_idx=0; TSEL->p[P_SLEN]=16;
    seq_step(TSEL,&TSEL->step[0],div_samples(2),0);
    uint32_t seen=0; for (uint32_t i=0;i<TSEL->seq_n;i++) { int32_t p=groove_pad(TSEL->seq_notes[i]); if(p>=0)seen |= 1u << (uint32_t)p; }
    gcheck("sequencer emits all 16 pad notes with bounded arrays",TSEL->seq_n==NPAD && seen==65535u);
    seq_release(TSEL); step_set_ratchet(&TSEL->step[0],4);
    seq_step(TSEL,&TSEL->step[0],div_samples(2),1u<<23);
    seen=0; for(uint32_t i=0;i<TSEL->seq_n;i++) if(TSEL->seq_notes[i]==GROOVE_NOTE[15])seen++;
    gcheck("live skip on pad 16 does not collide with ratchet flag",TSEL->rat_left==3u && !seen);
    uint32_t ok=1;
    /* A twelve-pass score, independent of the dispatcher's arithmetic. */
    const uint16_t score[COND_COUNT]={0xFFF,1,0xFFE,0x555,0xAAA,0x111,0x888,0,0xFFF,0x249,0x492,0x924,0x222,0x444};
    for(uint32_t fill=0;fill<2;fill++) for(uint32_t c=0;c<COND_COUNT;c++) for(uint32_t cycle=0;cycle<12;cycle++) {
        TSEL->seq_cycle=cycle; step_set_condition(&TSEL->step[0],c); song.fill=fill;
        int want=((score[c]>>cycle)&1u)!=0;
        if(c==COND_FILL)want=fill;
        if(c==COND_NOTFILL)want=!fill;
        seq_step(TSEL,&TSEL->step[0],div_samples(2),0); ok &= (TSEL->seq_n!=0)==want;
        seq_release(TSEL);
    }
    gcheck("all cycle conditions gate the whole step and its ratchets",ok);
    song.fill=1; step_set_condition(&TSEL->step[0],COND_FILL); seq_step(TSEL,&TSEL->step[0],div_samples(2),0);
    gcheck("FILL condition responds to the live fill switch",TSEL->seq_n==NPAD && TSEL->rat_left==3u);
    setup(); TSEL->step[0].hit=1; TSEL->step[0].time=ST_NOTE;
    step_set_condition(&TSEL->step[0],COND_FILL); kb_mask=perf_mask=1u; fm1_in.buttons=1u; kb_prev=0;
    fm1_in.notes=1u<<26; keyboard_block();
    gcheck("FX plus G5 holds FILL without playing or recording a note",groove_fill_held && condition_plays(TSEL,&TSEL->step[0]) && kb_note[26]==KB_SILENT);
    fm1_in.notes=0; keyboard_block();
    gcheck("letting go of G5 ends the transient FILL",!groove_fill_held && !condition_plays(TSEL,&TSEL->step[0]));
    setup(); TSEL->step[0].ghost=0xFFFF; TSEL->step[0].condition_ext=8;
    load_pat16(TSEL,PATTERNS[11].note,PATTERNS[11].flags);
    gcheck("loading a pattern clears old ghost hits and extended conditions",!TSEL->step[0].ghost && !TSEL->step[0].condition_ext);
    setup(); trk[0].p[P_SLEN]=3; trk[1].p[P_SLEN]=5; seq_start();
    seq_tick(&trk[0],0); seq_tick(&trk[1],0);
    for(uint32_t i=0;i<15;i++) { seq_tick(&trk[0],div_samples(trk[0].p[P_SDIV])); seq_tick(&trk[1],div_samples(trk[1].p[P_SDIV])); }
    gcheck("independent pattern lengths maintain independent cycles",trk[0].seq_cycle==5u && trk[1].seq_cycle==3u);
}
static void recording(void)
{
    setup(); groove_rec_start=1; rec_tap();
    gcheck("NOTE recording waits while stopped",groove_rec_wait && !song.playing && !transport_req);
    input_on(TSEL,GROOVE_NOTE[15],96);
    gcheck("first note starts and is recorded at step zero",song.playing && !groove_rec_wait && TSEL->seq_idx==0 && (TSEL->step[0].hit & 0x8000u));
    setup(); groove_rec_start=2; rec_tap(); events_block(CTL);
    gcheck("1 BAR count-in delays the sequencer",song.playing && groove_count && TSEL->seq_idx==15u && !rec_on(TSEL));
    input_on(TSEL,36,100); gcheck("audition during count-in writes no early hit",!TSEL->step[0].hit);
    uint32_t remain=groove_count; events_block(remain);
    input_on(TSEL,38,100);
    gcheck("recording starts exactly after count-in",!groove_count && TSEL->seq_idx==0u && (TSEL->step[0].hit & 2u));
}
static void midi(void)
{
    setup(); gcheck("groove MIDI IN maps CH1..3 / CH10 / other to selected",midi_track(0)==&trk[0] && midi_track(2)==&trk[2] && midi_track(9)==&trk[NTRK-1u] && midi_track(15)==TSEL);
    midi_event(0x90,9,36,100); gcheck("CH10 is accepted by the real MIDI dispatcher",midi_note_held(&trk[NTRK-1u],36));
    track_select(0); midi_event(0x80,9,36,0); gcheck("MIDI note-off retains its track after selection changes",!midi_note_held(&trk[NTRK-1u],36));
    setup(); TSEL->route=1; TSEL->midi_send=2; TSEL->midi_ch=16;
    TSEL->step[0]=(step_t){{36},1,ST_NOTE,0,100}; TSEL->seq_idx=0;
    seq_step(TSEL,&TSEL->step[0],div_samples(2),0);
    uint32_t first=midi_out_q[mo_r%MQ];
    uint32_t voices=0; for(uint32_t i=0;i<NVOICE;i++)voices += TSEL->v[i].active;
    gcheck("MIDI-only sequencer emits USB note-on without an audio voice",mo_w>mo_r && ((first>>8)&255u)==0x9Fu && !voices);
    TSEL->midi_ch=2; seq_release(TSEL);
    uint32_t last=midi_out_q[(mo_w-1u)%MQ];
    gcheck("sequenced note-off uses its original channel",((last>>8)&255u)==0x8Fu);
    setup(); kb_note[0]=36; key_on(0,TSEL); TSEL->midi_ch=8; key_off(0,TSEL);
    last=midi_out_q[(mo_w-1u)%MQ]; gcheck("physical key-up retains its original CH10",((last>>8)&255u)==0x89u);
}
static void persistence(void)
{
    project_t p,q; project_store_t b; uint32_t ok=1;
    setup(); song.g[G_DRREV]=65; song.g[G_DRLVL]=40; TSEL->route=2; TSEL->midi_ch=10; TSEL->midi_send=2;
    for(uint32_t i=0;i<NSTEP;i++) {
        TSEL->step[i].hit=(uint16_t)(1u << (i%16u)); TSEL->step[i].time=ST_NOTE;
        if(i&1u)TSEL->step[i].ghost=TSEL->step[i].hit;
        step_set_condition(&trk[i%4u].step[i],i%COND_COUNT); step_set_ratchet(&TSEL->step[i],1u+i%4u);
    }
    project_capture(&p); ok &= proj_pack(&b,&p) && proj_import(&q,&b,sizeof b) && !memcmp(&p,&q,sizeof p);
    gcheck("FUN9 round-trip: all pads, ghosts, conditions, FM6, routes, FX",ok);
    uint8_t old[PROJ_STORE_V8]; memset(old,0,sizeof old);
    setup(); project_capture(&p); proj_pack(&b,&p); legacy_wire(old,PROJ_STORE_V8,PROJ_MAGIC_V8,&b,1);
    uint32_t magic=PROJ_MAGIC_V8,size=PROJ_STORE_V8; memcpy(old,&magic,4);memcpy(old+4,&size,4);old[65]=0;
    memcpy(old+3056,b.raw+PROJ_FM6_OFF,512); memcpy(old+3568,b.raw+PROJ_NAME_OFF,12);
    int16_t inert=100;memcpy(old+8+G_DRREV*2u,&inert,2);
    uint32_t sum=proj_hash(old,sizeof old-4u);memcpy(old+sizeof old-4u,&sum,4);
    ok=proj_import(&q,old,sizeof old);
    gcheck("original FUN8 layout imports with audio/routes and master FX bypass",ok && !q.rsv && !q.g[G_DRREV] && !q.t[0].route && !memcmp(q.fm6[0],p.fm6[0],FM6_PACKED) && !memcmp(q.fm6[5],p.fm6[5],FM6_PACKED));
    old[100]^=1;gcheck("corrupted legacy payload is rejected before use",!proj_import(&q,old,sizeof old));
    setup(); TSEL->step[0].hit=0x8000; project_capture(&p);proj_pack(&b,&p);
    b.raw[3043]=255;sum=proj_hash(b.raw,sizeof b-4u);memcpy(b.raw+sizeof b-4u,&sum,4);
    gcheck("invalid ghost bits are rejected even with a valid hash",!proj_import(&q,&b,sizeof b));
    setup(); project_capture(&p);proj_pack(&b,&p);
    /* First step note bytes encode the condition; 14 and 15 are unassigned. */
    b.raw[161]&=127; b.raw[162]|=128; b.raw[163]|=128; b.raw[165]|=128;
    sum=proj_hash(b.raw,sizeof b-4u);memcpy(b.raw+sizeof b-4u,&sum,4);
    gcheck("unassigned condition rejected despite a valid project hash",!proj_import(&q,&b,sizeof b));
    gcheck("flash sector and retained cache sizes retain guard space",sizeof b==5632u && sizeof(proj_slot)+0xC8u<=0x5D50u);
    gcheck("extended conditions retain the 16-byte step allocation",sizeof(step_t)==16u);
}
static void userkit(void)
{
    const char *path=getenv("GROOVE_USERKIT");
    if (!path) return;
    memset(host_slots,0,sizeof host_slots);
    FILE *f=fopen(path,"rb");
    if (!f) {gcheck("user kit file opens",0);return;}
    uint32_t n=(uint32_t)fread(host_slots,1,SMP_USER_SIZE,f);fclose(f);
    smp_user_scan(0);uint32_t ok=n>512u && usr_nz[0]==16u, seen=0;
    for(uint32_t p=0;p<NPAD;p++) {
        setup();TSEL->p[P_E0]=1;trk_note_on(TSEL,GROOVE_NOTE[p],100);
        if(render_peak(FS/CTL/2u)>100)seen |= 1u<<p;
    }
    gcheck("kit CLI output plays all 16 GM-mapped zones in native SMPKIT",ok && seen==65535u);
    memset(host_slots,0,sizeof host_slots);smp_user_scan(0);
}
#ifndef GROOVE_TEST_NO_MAIN
int main(void)
{
    kits();patterns();recording();midi();persistence();userkit();
    printf("groove: %d failures\n",failures);return failures ? 1 : 0;
}
#endif

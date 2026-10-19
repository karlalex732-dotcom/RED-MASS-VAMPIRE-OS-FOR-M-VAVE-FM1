/* SPDX-License-Identifier: GPL-3.0-only
 * Shift note/lock events, leaving the transport clock and song row length intact. */
static uint32_t upgrade_seq_skip(track_t *t, const step_t *s, uint32_t idx)
{
    uint32_t skip=0, i, k;
    if (t->rskip_n && t->rskip_idx == idx) {
        for (k=0;k<t->rskip_n;k++) {
            for (i=0;i<s->n && i<4u;i++) if (s->note[i]==t->rskip[k]) skip |= 1u<<i;
            for (i=0;i<drum_pads(t);i++) if (((s->hit>>i)&1u) && pattern_note(t,i)==t->rskip[k]) skip |= 1u<<(8u+i);
        }
        t->rskip_n=0;
    }
    return skip;
}
static void upgrade_micro_fire(track_t *t, uint32_t idx, uint32_t cycle, uint32_t period, uint32_t repeat, uint32_t part, uint32_t late)
{
    uint16_t clock_idx=t->seq_idx;
    uint32_t clock_cycle=t->seq_cycle;
    const step_t *s=&seq_steps(t)[idx];
    t->seq_idx=(uint16_t)idx; t->seq_cycle=cycle;
    if (!repeat) {
        motion_step(t,idx,chain.running ? &chain.source[chain.slot].motion : &motion);
        rec_hold(t,idx,(uint32_t)t->p[P_SLEN]);
    }
    seq_step(t,s,period,repeat ? SEQ_REP : upgrade_seq_skip(t,s,idx));
    if(part && t->seq_n)t->seq_off=part*(uint32_t)t->p[P_SGATE]/128u;
    if (t->seq_n && !t->seq_hold) t->seq_off=t->seq_off>late ? t->seq_off-late : 0u;
    t->seq_idx=clock_idx; t->seq_cycle=clock_cycle;
}
static void upgrade_micro_tick(track_t *t, uint32_t n, uint32_t period, uint32_t len)
{
    uint32_t initial=t->seq_pos>=0x7FFFFFFFu;
    const step_t *steps=seq_steps(t);
    if (!len) len=1;
    if (initial) {
        t->seq_idx=0; t->seq_pos=chain.running ? chain.carry : 0u;
        t->micro_next=0; t->micro_play=0; t->micro_wrap=1;
        t->micro_cycle=t->seq_cycle;
        int32_t offset=(int32_t)step_samples(t,period,0)*step_micro(&steps[0])/32;
        t->micro_due=(offset>0 ? offset : 0)-(int32_t)t->seq_pos;
        t->micro_rat=0;
    } else {
        t->seq_pos+=n;
        while (t->seq_pos>=step_samples(t,period,t->seq_idx)) {
            t->seq_pos-=step_samples(t,period,t->seq_idx);
            t->seq_idx=(uint16_t)((t->seq_idx+1u)%len);
            if (!t->seq_idx) t->seq_cycle++;
        }
        t->micro_due-=(int32_t)n;
        if (t->rat_left) t->micro_rat-=(int32_t)n;
    }
    /* At the supported clocks a block crosses at most a few events. */
    for (uint32_t guard=0;t->micro_due<=0 && guard<8u;guard++) {
        uint32_t idx=t->micro_next%len, next=(idx+1u)%len;
        uint32_t duration=step_samples(t,period,idx);
        int32_t here=(int32_t)duration*step_micro(&steps[idx])/32;
        int32_t there=(int32_t)step_samples(t,period,next)*step_micro(&steps[next])/32;
        if (t->micro_wrap && here<0) here=0; /* first event cannot precede PLAY */
        uint32_t gap=(uint32_t)((int32_t)duration+there-here),hits=step_ratchet(&steps[idx]);
        t->micro_part=(duration<gap ? duration : gap)/hits;
        upgrade_micro_fire(t,idx,t->micro_cycle,period,0,hits>1u ? t->micro_part : 0u,(uint32_t)-t->micro_due);
        t->micro_play=(uint8_t)idx;
        t->micro_rat=(int32_t)t->micro_part+t->micro_due;
        t->micro_due+=(int32_t)duration+there-here;
        t->micro_next=(uint8_t)next; t->micro_wrap=0;
        if (!next) t->micro_cycle++;
    }
    for (uint32_t guard=0;t->rat_left && t->micro_rat<=0 && guard<4u;guard++) {
        uint32_t idx=t->micro_play%len, hits=step_ratchet(&steps[idx]);
        if (t->rat_left>=hits) { t->rat_left=0; break; }
        t->rat_left--;
        upgrade_micro_fire(t,idx,t->micro_cycle-(t->micro_next==0u),period,1,t->micro_part,(uint32_t)-t->micro_rat);
        t->micro_rat+=(int32_t)t->micro_part;
    }
}

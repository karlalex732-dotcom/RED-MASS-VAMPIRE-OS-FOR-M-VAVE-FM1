/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_TEST_NO_MAIN 1
#include "ui_test.c"
int main(int argc,char **argv)
{
    if(argc!=2)return 1;
    ui_power_on();palette_set(UI_REDMASS_INDEX);settings.palette=UI_REDMASS_INDEX;upgrade_category=1;
    set_engine_of(TSEL,CLUB_ENGINE[0]);apply_preset_to(TSEL,CLUB_FACTORY[0]);
    for(uint32_t i=0;i<16;i++)TSEL->step[i]=(step_t){{(uint8_t)(60+i%7)},1,ST_NOTE,0,96};
    step_set_micro(&TSEL->step[3],-3);motion_set_event(TSEL,3,P_DIST,35);
    upgrade.macro[0][0][0]=(upgrade_macro_target_t){P_E2,63,TSEL->p[P_E2]};
    upgrade.macro[0][0][1]=(upgrade_macro_target_t){P_REV,45,TSEL->p[P_REV]};
    for(uint32_t i=0;i<4;i++)project_save(i);
    chain_defaults(&chain_config);chain_config.count=4;
    for(uint32_t i=0;i<4;i++)chain_config.row[i]=(chain_row_t){(uint8_t)i,(uint8_t)(i+1),(uint8_t)(1u<<i),16,(uint16_t)(110+5*i)};
    chain_config.rsv[0]=CHAIN_SOUNDS|CHAIN_LOOP;
    vampire_capture(0);TSEL->p[P_REV]=90;vampire_capture(1);
    for(uint32_t i=0;i<UG_N;i++) {
        song.sel=i==UG_DRUM ? NTRK-1u : 0u;
        if(i==UG_DRUM){set_engine_of(TSEL,ENGI_SMPKIT);apply_preset_to(TSEL,0);}
        upgrade_launch(i);
        char name[40];snprintf(name,sizeof name,"tool-%02u",i+1);
        screen_save(argv[1],name);
    }
    upgrade_open_toolbox();screen_save(argv[1],"toolbox");
    go_page(GR_SONG);screen_save(argv[1],"song-main");
    return 0;
}

# SPDX-License-Identifier: GPL-3.0-only
"""RED MASS 0.5.2: deliberate replacement voicings for the curated library.

Sub/808, all bass families and percussion are returned byte-for-byte unchanged.
Engine numbers, factory slots, names and voice caps are never moved. Sound
changes are local to the patch: no DSP, shared samples or global FX change.
"""
from copy import deepcopy
from gen_fm6_patches import op, voice, pack

PROTECTED = {'sub', 'bass', 'reese', 'talkbass', 'perc'}

# Four articulations per role: hollow, rusted, spectral and drowned. These are
# envelopes in the instrument's native time units, not linear milliseconds.
ENVELOPES = {
 'bell':[(1,112,0,99),(0,91,0,83),(3,118,0,104),(1,104,0,94)],
 'pluck':[(0,75,0,49),(0,57,0,35),(2,93,5,69),(1,84,0,58)],
 'keys':[(1,103,21,72),(0,86,11,53),(2,112,39,85),(3,98,17,66)],
 'lead':[(1,81,103,46),(0,68,91,35),(8,101,109,61),(4,91,99,52)],
 'choir':[(82,105,111,105),(57,91,107,91),(101,113,117,112),(69,99,109,101)],
 'pad':[(96,109,112,109),(72,98,119,101),(108,115,107,114),(88,104,115,106)],
 'string':[(51,94,119,89),(31,83,111,77),(77,103,116,102),(62,98,113,94)],
 'organ':[(9,104,127,79),(2,93,119,61),(23,111,123,95),(12,102,125,87)],
 'stab':[(0,57,0,39),(0,45,0,28),(3,74,0,64),(1,66,0,51)],
 'pulse':[(2,97,101,65),(0,84,119,49),(17,109,109,81),(8,101,111,73)],
 'fx':[(15,112,37,106),(54,103,99,101),(89,111,107,112),(32,119,51,109)],
 'piano':[(0,111,21,81),(0,101,11,69),(2,117,34,94),(1,107,17,77)],
}

def fm_voice(p, v, r):
    """A different six-operator instrument, including its own operator EGs.

    Three pairs keep a stable root; detuned low carriers and inharmonic
    modulators provide the tension. High partials decay before the body.
    """
    role=p['role']; sustained=role in ('pad','choir','string','organ')
    carrier_ratios = {
      'bell':[(1,0),(1,1),(2,0)], 'pluck':[(1,0),(0,0),(2,0)],
      'keys':[(1,0),(0,0),(1,1)], 'pad':[(0,0),(1,0),(1,1)],
      'choir':[(1,0),(0,0),(1,2)], 'organ':[(0,0),(1,0),(2,0)],
      'lead':[(1,0),(0,0),(2,0)], 'fx':[(0,0),(1,1),(1,3)],
    }.get(role,[(1,0),(0,0),(2,0)])
    mods = {
      'bell':[(1,41),(2,71),(5,18)], 'pluck':[(1,0),(2,31),(4,7)],
      'keys':[(1,0),(2,0),(3,13)], 'pad':[(1,0),(1,2),(3,11)],
      'choir':[(1,19),(2,37),(3,61)], 'organ':[(1,0),(2,0),(3,0)],
      'lead':[(2,13),(1,37),(3,5)], 'fx':[(1,41),(4,13),(5,18)],
    }.get(role,[(1,0),(2,17),(3,31)])
    ops=[]
    for pair,((fc,ff),(mc,mf)) in enumerate(zip(carrier_ratios,mods)):
        if sustained:
            cr=(34+v*4+pair*2,61+pair*3,43,49+v*2)
            cl=(99,92-pair*3,85+v*2,0)
            mr=(40+v*3+pair*3,68+pair*4,49,58+v*2)
            ml=(99,73-pair*4,59+v*3,0)
            index=42+v*4+pair*3
        elif role in ('bell','pluck','keys'):
            cr=(99,49+v*4+pair*2,35+pair*3,54+v*3)
            cl=(99,74-pair*5,0,0)
            mr=(99,74+v*2+pair*4,56+pair*4,76)
            ml=(99,39+v*5,0,0)
            index=61+v*3-pair*3
        else:
            cr=(94-v*2,68+pair*3,45,73-v*2)
            cl=(99,87-pair*3,74+v*3,0)
            mr=(96,78+pair*3,54,79)
            ml=(99,59+v*4,43+pair*5,0)
            index=57+v*4+pair*2
        # Fine ratios vary gently within the recipe, rather than randomising
        # every operator and losing the playable fundamental.
        mf=(mf+(r%3)*2) % 100
        ops.append(op(r=cr,l=cl,ol=96-pair*8,fc=fc,ff=ff,
                      det=6+(v+pair)%3,kvs=2,rs=1))
        ops.append(op(r=mr,l=ml,ol=index,fc=mc,ff=mf,det=7,
                      kvs=4+pair%2,rs=1))
    return pack(voice(p['name'][4:14],5,ops,fb=1+v%2,lfs=13+v*5,
                      lfd=65 if sustained else 37,lpmd=0,lamd=0,lpms=0))

def revoice(p):
    if p['role'] in PROTECTED:
        return p
    p=deepcopy(p);n=p['number'];v=(n-1)%4;r=p.get('recipe',(n-1)//8)
    role=p['role'];ei=p['engine_id'];old=p['engine_values'];e=old.copy()
    sustained=role in ('choir','pad','string','organ')
    short=role in ('bell','pluck','keys','stab','piano')
    if ei==0:
        if sustained:
            e=[[2,4,0,2][v],18+(r*3+v*5)%29,43+v*9,(r+v)%3,
               [43,51,37,47][v]+r%5,[9,23,13,19][v],9+(r+v*3)%19,35+v*3]
        elif short:
            e=[[2,3,4,2][v],3+(r+v*3)%13,[28,0,63,39][v],0,
               [49,57,43,53][v]+r%5,[31,39,23,35][v],14+(r*2+v*5)%25,39+v*5]
        else:
            e=[[4,1,0,4][v],7+(r*2+v*3)%18,47+v*13,(r+v)%3,
               [61,69,53,64][v]+r%5,[49,65,39,57][v],27+(r+v*7)%31,43+v*3]
    elif ei==2:
        e=[[3,1,4,0][v] if sustained else [3,6,4,1][v],
           [4,0,1,4][v],[29,47,37,41][v]+r%9,
           [31,49,23,41][v],3+(r+v*3)%13,0,
           [33,17,43,27][v] if sustained else [17,0,25,11][v],0]
    elif ei==3:
        # WRAM: SINE, HOLW, VOXO and ORGN; the TONE filter removes the
        # brittle top without forcing every chip sound onto one waveform.
        e=[[1,0,2,1][v],4,[24,80,96,72][v],8+(r+v*7)%23,
           0 if sustained else [2,9,0,4][v],[2,0,4,1][v],0,
           [47,61,41,53][v]+r%7]
    elif ei==5:
        e=[[86,105,73,96][v],[104,81,117,67][v],
           [67,37,96,81][v] if sustained else [15,9,27,19][v],
           [-7,-4,-9,-6][v],[55,73,43,63][v],
           [7,13,4,9][v],[73,89,65,81][v],[9,17,5,13][v]]
    elif ei==6:
        e=[[4,12,13,14][v] if short else [0,4,6,13][v],
           [0,7,0,-12][v],[ -12,-12,6,0][v],11+(r+v*3)%19,
           0,[45,57,39,51][v]+r%7,[23,43,17,31][v],
           [39,71,53,89][v]]
    elif ei==7:
        e=[old[0],[3,4,2,5][v],[1,2,3,1][v],[-6,-4,-7,-5][v],
           0 if sustained else [1,2,0,1][v],4+(r+v)%11,
           [17,29,9,23][v],1 if v!=1 else 2]
    elif ei==9:
        e=[old[0],max(9,min(118,old[1]+[-9,13,-17,7][v])),
           [43,61,35,51][v]+r%7,[63,79,55,71][v],
           [23,39,57,31][v],83+(r+v*5)%27,
           [68,87,57,77][v] if role=='string' else 0,
           [7,23,13,31][v]]
    elif ei==11:
        e=[old[0],[111,93,123,103][v],
           [53,67,47,59][v]+r%7,[37,63,29,49][v],
           [31,17,39,23][v],old[5],[41,57,29,47][v],
           [7,17,3,11][v]]
        if old[0]>=2:
            e[7]=max(99,old[7]) # balanced LFSR excitation even on low notes
    elif ei==12:
        e=[0,0,[-14,-7,-19,-11][v],0,[-9,-3,-13,-7][v],0,
           [7,3,11,5][v],old[7]]
        p['fm6']=fm_voice(p,v,r)
    elif ei==14:
        mode={'choir':0,'pad':4,'string':6,'organ':7,'bell':5,
              'pluck':5,'lead':1,'stab':1,'pulse':4}.get(role,old[0])
        if role in ('pad','choir','stab','pulse'):
            mode=old[0] # choir, bowed and rain sources remain distinct families
        e=[mode,[43,57,37,49][v]+r%7,[31,55,19,43][v],
           [71,87,59,79][v] if role!='fx' else [41,65,29,53][v],
           [19,31,13,25][v],[4,9,2,6][v] if role!='fx' else [13,23,9,17][v],
           21+(r*7+v*19)%79,[-12,0,0,-12][v] if role=='fx' else 0]
    elif ei==4:
        # Keep the recorded grand piano's attack and tuning. Damp the body,
        # vary sustain/room and use modest saturation, not a synthetic piano.
        e=[old[0],0,0,1,[69,83,61,76][v]+(n-249)//4*7,0,
           [5,11,2,7][v],0]
    else:
        raise ValueError(f'No dark recipe for {p["name"]}: engine {ei}')
    p['engine_values']=e
    p['envelope']=list(ENVELOPES[role][v])
    if short:
        fx=[[9,23,5,17][v],[13,7,29,19][v],[27,17,43,31][v],[67,49,89,77][v]]
    elif sustained:
        fx=[[5,13,3,9][v],[37,23,57,43][v],[17,9,29,21][v],[89,71,104,97][v]]
    elif role=='fx':
        fx=[[13,29,7,19][v],[31,47,19,39][v],[43,29,59,47][v],[97,79,111,103][v]]
    else:
        fx=[[27,43,19,35][v],[11,5,23,17][v],[31,19,43,37][v],[43,29,61,53][v]]
    p['effects']=fx
    c=p['sound_controls']={str(k):val for k,val in p['sound_controls'].items()}
    c.update({'5':[-13,-21,-7,-17][v] if short else [-9,-17,5,-13][v],
       '6':0,'7':[-5,5,-9,9][v], '9':12+(r*3+v*5)%43,
       '10':v%2,'11':(n*7)%128,'12':67+v*7 if sustained else 17+v*5,
       '13':1 if role in ('lead','string','fx') and v==2 else 0,
       '14':[9,17,7,13][v], '15':[0,3,1,5][v],
       '16':49+v*9 if role=='pulse' else (4+v*2 if role in ('choir','fx') else 0),
       '37':0,'38':0})
    matrix=[[3,4,19+v*3],[1,2,[-7,11,-11,7][v]],
            [6,2,23+v*3],[7,3,13+v*4]]
    if ei==4: # recorded pianos retain a stable pitch and attack
        c['14']=0;c['15']=0;c['9']=19+(n-249)*3
        matrix=[[3,4,19],[1,5,3+v],[6,2,13+v*2],[7,3,9+v*2]]
    for i,row in enumerate(matrix):
        for k,val in enumerate(row):c[str(49+i*3+k)]=val
    p['filter_env']=c['5']
    p['sound_controls']={int(k):value for k,value in c.items()}
    p['articulation']=['Hollow','Rusted','Spectral','Drowned'][v]
    p['sound_design']='Dark 0.5.2: new source/partial balance, damping, native envelopes and expression.'
    return p

def voice_dark_bank(bank):
    return [revoice(p) for p in bank]

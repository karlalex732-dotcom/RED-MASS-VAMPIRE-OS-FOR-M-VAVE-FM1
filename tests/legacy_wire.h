/* SPDX-License-Identifier: GPL-3.0-only
 * Frozen four-track fixture writer for historical packed formats. */
static void legacy_wire(uint8_t *out,uint32_t size,uint32_t magic,const project_store_t *native,int drums)
{
 const uint8_t *b=native->raw;uint32_t np=b[66],stride=np+2u+64u*9u,pos=68u,src[4]={0,1,2,drums?5u:3u};
 uint32_t v9=magic==PROJ_MAGIC_V9||magic==PROJ_MAGIC_V10;
 memset(out,0,size);memcpy(out,b,68);memcpy(out,&magic,4);memcpy(out+4,&size,4);out[63]=4;out[67]=0;
 if(drums&&out[62]==5u)out[62]=3u;if(!v9)out[65]=0;
 for(uint32_t k=0;k<4;k++){
  memcpy(out+pos,b+68u+src[k]*stride,stride);
  if(!v9)for(uint32_t i=0;i<64;i++){
   uint8_t *s=out+pos+np+2u+i*9u;s[0]&=127;s[1]&=127;s[2]&=127;s[4]&=127;
  }
  pos+=stride;
 }
 uint32_t input=68u+6u*stride;memcpy(out+pos,b+input,36);pos+=36;input+=36;
 uint32_t mpos=pos,count=0;pos+=260;
 uint32_t on=b[input+1];out[mpos+1]=(uint8_t)(drums?((on&7u)|((on&32u)>>2)):(on&15u));
 for(uint32_t i=0;i<b[input];i++){
  const uint8_t *e=b+input+4u+i*4u;uint32_t place=e[0]|(e[1]&1u)<<8,owner=place>>6;
  if(drums){if(owner==5)place=3u*64u+(place&63u);else if(owner>=3u)continue;}else if(owner>=4u)continue;
  uint8_t *v=out+mpos+4u+count++*4u;v[0]=(uint8_t)place;v[1]=e[1]>>1;v[2]=e[2];v[3]=e[3];
 }
 out[mpos]=(uint8_t)count;input+=260;
 if(v9){memcpy(out+pos,b+input,256);pos+=256;input+=256;for(uint32_t k=0;k<4;k++){memcpy(out+pos,b+input+src[k]*3u,3);pos+=3;}}
 uint32_t name=size-16u,patch=name-512u;
 if(magic!=PROJ_MAGIC_V7)for(uint32_t k=0;k<4;k++)memcpy(out+patch+k*128u,b+PROJ_FM6_OFF+src[k]*128u,128);
 memcpy(out+name,b+PROJ_NAME_OFF,12);
 if(magic==PROJ_MAGIC_V10)memcpy(out+patch-4u,b+PROJ_AUX_OFF,4);
 uint32_t sum=proj_hash(out,size-4u);memcpy(out+size-4u,&sum,4);
}

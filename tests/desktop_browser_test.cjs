/* SPDX-License-Identifier: GPL-3.0-only; Copyright (C) 2026 Karsei.
 * Fresh headless browser QA. No hardware, MIDI writes or signed-in session. */
const fs = require('node:fs/promises'), path = require('node:path'), http = require('node:http');
const assert = require('node:assert/strict');
const {chromium} = require(require.resolve('playwright', {paths:[process.env.CODEX_PRIMARY_RUNTIME_NODE_MODULES || process.cwd()]}));
const bundle=path.resolve(__dirname,'../..'), output=path.resolve(__dirname,'../build/desktop-qa');
const mimes={'.html':'text/html','.js':'text/javascript','.json':'application/json','.ttf':'font/ttf','.mp3':'audio/mpeg','.png':'image/png','.wav':'audio/wav'};
(async()=>{
 await fs.mkdir(output,{recursive:true});
 const server=http.createServer(async(req,res)=>{
  try{
   let name=decodeURIComponent(new URL(req.url,'http://localhost').pathname);
   if(name.endsWith('/'))name+='index.html';
   const file=path.resolve(bundle,'.'+name);assert(file.startsWith(bundle+path.sep));
   res.setHeader('Content-Type',mimes[path.extname(file)] || 'text/plain');
   res.end(await fs.readFile(file));
  }catch(e){res.statusCode=404;res.end('Not found');}
 });
 await new Promise(r=>server.listen(0,'127.0.0.1',r));
 const base=`http://127.0.0.1:${server.address().port}/`;
 const browser=await chromium.launch({headless:true,channel:'chromium'});
 const page=await browser.newPage({viewport:{width:1366,height:1050}}),errors=[],missing=[];
 page.on('pageerror',e=>errors.push(e.message));
 page.on('response',r=>{if(r.status()>=400&&!r.url().endsWith('favicon.ico'))missing.push(`${r.status()} ${r.url()}`);});
 try{
  await page.goto(base+'web-tools/webapp/app/',{waitUntil:'networkidle'});
  assert.equal(await page.title(),'RED MASS VAMPIRE OS');
  await page.screenshot({path:path.join(output,'home.png'),fullPage:true});
  await page.locator('nav [data-view="editor"]').click();
  const editor=page.frameLocator('#view');
  await editor.locator('#connect').waitFor();
  const frame=page.frames().find(f=>f.url().includes('/editor/'));
  await frame.goto(base+'web-tools/webapp/editor/?mock=1');
  await editor.locator('#connect').click();
  await editor.locator('#p-sound').waitFor({state:'visible',timeout:60000});
  await page.waitForFunction(()=>!document.querySelector('nav [data-view="home"]').disabled);
  assert.match(await editor.locator('#version').textContent(),/RED MASS VAMPIRE OS/);
  const tabs=['sound','fm6','sequencer','tracks','library','samples','projects','settings'];
  for(const tab of tabs){
   await editor.locator(`[role="tab"][data-tab="${tab}"]`).click();
   await editor.locator('#p-'+tab).waitFor({state:'visible'});
   console.log(`App editor tab ${tab}: visible`);
  }
  await editor.locator('[role="tab"][data-tab="tracks"]').click();
  assert.equal(await editor.locator('#mixer > *').count(),6);
  assert.match(await editor.locator('#mixer').textContent(),/6/);
  await page.screenshot({path:path.join(output,'mixer-mock.png'),fullPage:true});
  await editor.locator('[role="tab"][data-tab="sound"]').click();
  await editor.locator('#engine').selectOption({label:'SMPKIT'});
  await editor.locator('#preset option').filter({hasText:'RAGE TRAP'}).waitFor({state:'attached'});
  const names=await editor.locator('#preset option').allTextContents();
  assert(names.some(n=>n.includes('NIGHT TRAP'))&&names.some(n=>n.includes('RAGE TRAP')));
  console.log('Native kit descriptors exposed by editor:',names.join(', '));
  await editor.locator('#preset').selectOption('4');
  await page.waitForFunction(()=>!document.querySelector('nav [data-view="home"]').disabled);
  await page.locator('nav [data-view="home"]').click();
  await page.locator('#home').waitFor({state:'visible'});
  await page.locator('.actions [data-tab="projects"]').click();
  await editor.locator('[role="tab"][data-tab="projects"][aria-selected="true"]').waitFor();
  await page.locator('nav [data-view="installer"]').click();
  const install=page.frameLocator('#view');
  await install.locator('#go').waitFor();
  await page.waitForTimeout(800);
  console.log('Installer package status:',await install.locator('#status').textContent());
  // Never click Install or request real MIDI permission in this test.
  const sourceLink=await install.locator('a').filter({hasText:'Source included'}).getAttribute('href');
  assert.equal((await page.request.get(new URL(sourceLink,page.frames().find(f=>f.url().includes('/installer/')).url()).href)).status(),200);
  for(const view of ['sounds','guide']){
   await page.locator(`nav [data-view="${view}"]`).click();
   await page.frameLocator('#view').locator('body').waitFor();
   await page.waitForTimeout(150);
  }
  assert.deepEqual(errors,[]);assert.deepEqual(missing,[]);
  console.log('Desktop browser checks: passed; eight editor tabs, six tracks, both kits, backup route, installer/source/guide/previews; no browser errors or missing assets.');
 }finally{await browser.close();await new Promise(r=>server.close(r));}
})().catch(e=>{console.error(e);process.exit(1);});

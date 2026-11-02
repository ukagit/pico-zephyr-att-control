'use strict';
const fs=require('fs'),vm=require('vm'),assert=require('assert');
const html=fs.readFileSync(__dirname+'/../web/att.html','utf8');
const js=html.split('<script>')[1].split('</script>')[0];
const nodes=new Map(), posts=[];let active=0,maxActive=0,fail=false;
function element(id){if(!nodes.has(id))nodes.set(id,{value:'0',style:{},listeners:{},setAttribute(){},addEventListener(t,f){this.listeners[t]=f;}});return nodes.get(id);}
const wait=ms=>new Promise(r=>setTimeout(r,ms));
const context={document:{getElementById:element},setTimeout,clearTimeout,AbortController,console,
 fetch:async(path,options)=>{active++;maxActive=Math.max(maxActive,active);await wait(20);active--;
 if(fail)throw Error('offline');const write=options.method==='POST';if(write)posts.push(Number(path.split('/').pop()));
 return {ok:true,json:async()=>({attenuation_db:write?posts.at(-1):20,mask:0,ready:true,gpio_error:0})};}};
(async()=>{
 vm.runInNewContext(js,context);await wait(60);assert.equal(element('db').value,'20');
 let prevented=false;const wheel=delta=>element('db').listeners.wheel({deltaY:delta,preventDefault(){prevented=true;}});
 wheel(-1);wheel(-1);wheel(-1);await wait(280);assert(prevented);assert.deepEqual(posts,[23]);
 element('db').value='127';wheel(-1);await wait(280);assert.equal(posts.at(-1),127);
 element('db').value='0';wheel(1);await wait(280);assert.equal(posts.at(-1),0);
 element('db').value='128';element('set').onclick();await wait(60);assert.equal(posts.at(-1),0);
 element('db').value='42';element('db').listeners.keydown({key:'Enter',preventDefault(){}});await wait(60);assert.equal(posts.at(-1),42);
 element('slider').value='64';element('slider').listeners.input();await wait(280);assert.equal(posts.at(-1),64);
 wheel(-1);await wait(205);wheel(-1);await wait(500);assert.equal(posts.at(-1),66);assert.equal(maxActive,1);
 fail=true;element('db').value='30';element('set').onclick();await wait(70);assert.match(element('status').textContent,/fehlgeschlagen/);
 console.log('PASS: wheel debounce, limits, Enter, slider, serialized requests, queued edits, errors');
})().catch(e=>{console.error(e);process.exitCode=1;});

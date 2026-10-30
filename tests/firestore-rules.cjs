// Offline Firestore Rules regression suite. Node 18+, no npm dependencies.
// Run only via Firebase emulators:exec with project demo-wiiu-rules.
const fs = require('node:fs');
const path = require('node:path');
const assert = require('node:assert/strict');
const crypto = require('node:crypto');
const host = process.env.FIRESTORE_EMULATOR_HOST;
const project = process.env.GCLOUD_PROJECT || 'demo-wiiu-rules';
if (!/^127\.0\.0\.1:\d+$/.test(host || '') || !project.startsWith('demo-'))
  throw new Error('Refusing tests outside a loopback demo-project emulator.');
const root = path.resolve(__dirname, '..');
const ruleFile = process.env.FIRESTORE_RULES_FILE || path.join(root, 'firestore.rules');
const rules = fs.readFileSync(ruleFile, 'utf8');
const ids = ['achievements.xml','achievements-story.xml'].flatMap(file =>
  Array.from(fs.readFileSync(path.join(root,file),'utf8').matchAll(/<achievement\b[^>]*\bid="([^"]+)"/g), x=>x[1]));
const list = rules.match(/return id in \[([\s\S]+?)\];/)[1];
assert.deepEqual(Array.from(list.matchAll(/'([^']+)'/g),x=>x[1]).sort(), ids.sort(), 'Server whitelist must equal the executable catalog.');
let checks = 0;
function pass(name) { console.log('PASS '+(++checks)+' '+name); }
pass('Server whitelist exactly covers the 70 executable IDs');
const base = `http://${host}/v1/projects/${project}/databases/(default)/documents`;
const hash = crypto.createHash('sha256').update('synthetic-profile|game|revision').digest('hex');
const collection = `users/alice/profiles/${hash}/unlocks`;
const document = `${collection}/wallet-10`;
const past = new Date(Date.now()-3600000).toISOString();
const valid = {utc:{timestampValue:past}};
function token(uid) {
 const now=Math.floor(Date.now()/1000);
 const encode=x=>Buffer.from(JSON.stringify(x)).toString('base64url');
 return encode({alg:'none',typ:'JWT'})+'.'+encode({sub:uid,user_id:uid,iat:now,exp:now+3600,aud:project,iss:'https://securetoken.google.com/'+project,firebase:{sign_in_provider:'password',identities:{}}})+'.';
}
async function request(method, target, fields, uid='alice') {
 const headers={}; if(uid) headers.Authorization='Bearer '+token(uid);
 if(fields!==undefined) headers['Content-Type']='application/json';
 const response=await fetch(base+(target.startsWith(':')?target:'/'+target), {method, headers,
  body: fields===undefined?undefined:JSON.stringify(target.startsWith(':')?fields:{fields}),signal:AbortSignal.timeout(15000)});
 const text=await response.text(); let body;try{body=JSON.parse(text)}catch{body=text}
 return {status:response.status,body};
}
async function denied(name, method, target, fields, uid) {
 const r=await request(method,target,fields,uid);
 assert.equal(r.status,403, name+': '+JSON.stringify(r.body)); pass(name);
}
async function main() {
 let r=await request('PATCH',document+'?currentDocument.exists=false',valid);
 assert.equal(r.status,200,JSON.stringify(r.body)); pass('Owner can create a real timestamp with the desktop PATCH precondition');
 r=await request('GET',document);assert.equal(r.status,200);assert.equal(Date.parse(r.body.fields.utc.timestampValue),Date.parse(past));pass('Owner reads its original UTC timestamp');
 r=await request('GET',collection+'?pageSize=100');assert.equal(r.status,200);assert.equal(r.body.documents.length,1);pass('Exact desktop paginated list query is allowed');
 await denied('Anonymous get denied','GET',document,undefined,null);
 await denied('Anonymous list denied','GET',collection+'?pageSize=100',undefined,null);
 await denied('Anonymous create denied','PATCH',collection+'/wallet-100?currentDocument.exists=false',valid,null);
 await denied('Other account cannot read owner data','GET',document,undefined,'bob');
 await denied('Other account cannot list owner data','GET',collection+'?pageSize=100',undefined,'bob');
 await denied('Other account cannot write under owner UID','PATCH',collection+'/wallet-100?currentDocument.exists=false',valid,'bob');
 const bobDoc=`users/bob/profiles/${hash}/unlocks/wallet-10`;
 r=await request('PATCH',bobDoc+'?currentDocument.exists=false',valid,'bob');assert.equal(r.status,200);pass('Second account can independently use the same profile hash and achievement ID');
 await denied('First account cannot read the second account','GET',bobDoc);
 await denied('User parent metadata is not readable','GET','users/alice');
 await denied('Parent role escalation denied','PATCH','users/alice',{role:{stringValue:'admin'}});
 await denied('Listing all users denied','GET','users?pageSize=100');
 await denied('Listing profile metadata denied','GET',`users/alice/profiles?pageSize=100`);
 await denied('Existing timestamps cannot be changed','PATCH',document,{utc:{timestampValue:new Date().toISOString()}});
 await denied('Existing records cannot be stripped','PATCH',document,{});
 await denied('Existing achievements cannot be deleted','DELETE',document);
 const target=collection+'/wallet-100?currentDocument.exists=false';
 await denied('Required utc field cannot be omitted','PATCH',target,{});
 await denied('String cannot replace native timestamp','PATCH',target,{utc:{stringValue:past}});
 await denied('Numeric type cannot replace timestamp','PATCH',target,{utc:{integerValue:'42'}});
 await denied('Spoofed ownership field rejected','PATCH',target,{...valid,ownerId:{stringValue:'bob'}});
 await denied('Extra schema and admin flags rejected','PATCH',target,{...valid,isAdmin:{booleanValue:true}});
 await denied('Oversized extra field rejected by rules','PATCH',target,{...valid,padding:{stringValue:'x'.repeat(900000)}});
 await denied('Future timestamp rejected','PATCH',target,{utc:{timestampValue:new Date(Date.now()+86400000).toISOString()}});
 await denied('Pre-epoch timestamp rejected','PATCH',target,{utc:{timestampValue:'1969-12-31T23:59:59Z'}});
 await denied('Unknown achievement ID rejected','PATCH',collection+'/injected-achievement?currentDocument.exists=false',valid);
 const planned = Array.from(fs.readFileSync(path.join(root,'achievements-planned.xml'),'utf8').matchAll(/<planned\b[^>]*\bid="([^"]+)"/g),x=>x[1])[0];
 assert.ok(planned);await denied('Unimplemented planned achievement rejected','PATCH',collection+'/'+planned+'?currentDocument.exists=false',valid);
 await denied('Malformed profile key rejected','PATCH','users/alice/profiles/short/unlocks/wallet-10?currentDocument.exists=false',valid);
 await denied('Nested unrecognized subcollection rejected','PATCH',document+'/private/nested?currentDocument.exists=false',valid);
 await denied('Global collection-group query rejected', 'POST',':runQuery',{structuredQuery:{from:[{collectionId:'unlocks',allDescendants:true}]}});
 r=await request('PATCH',document+'?currentDocument.exists=false',valid);assert.ok([400,403,409,412].includes(r.status));pass('Create-only retries cannot overwrite an existing document');
 r=await request('GET',document);assert.equal(Date.parse(r.body.fields.utc.timestampValue),Date.parse(past));pass('All mutation attacks leave the earned timestamp intact');
 console.log('SUCCESS '+checks+' offline Firestore authorization checks');
}
main().catch(e=>{console.error(e.message);process.exitCode=1});

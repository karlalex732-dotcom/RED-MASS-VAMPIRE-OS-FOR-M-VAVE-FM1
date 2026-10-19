# SPDX-License-Identifier: GPL-3.0-only
import html,re

def inline(s):
 s=html.escape(s)
 s=re.sub(chr(96)+'([^'+chr(96)+']+)'+chr(96),r'<code>\1</code>',s)
 return re.sub(r'\*\*([^*]+)\*\*',r'<strong>\1</strong>',s)

def markdown(s):
 ls=s.splitlines();out=[];i=0
 while i<len(ls):
  l=ls[i];i+=1
  if not l.strip():continue
  if l.startswith(chr(96)*3):
   a=[]
   while i<len(ls) and not ls[i].startswith(chr(96)*3):a.append(ls[i]);i+=1
   i+=1;out.append('<pre>'+html.escape('\n'.join(a))+'</pre>');continue
  if l.startswith('#'):
   n=len(l)-len(l.lstrip('#'));out.append(f'<h{n}>'+inline(l[n:].strip())+f'</h{n}>');continue
  if l.startswith('|'):
   rows=[l]
   while i<len(ls) and ls[i].startswith('|'):rows.append(ls[i]);i+=1
   a=[]
   for j,row in enumerate(rows):
    if j==1 and re.match(r'^\|[\s:|\-]+$',row):continue
    tag='th' if j==0 else 'td'
    a.append('<tr>'+''.join(f'<{tag}>'+inline(x.strip())+f'</{tag}>' for x in row.strip('|').split('|'))+'</tr>')
   out.append('<div class="table"><table>'+''.join(a)+'</table></div>');continue
  if re.match(r'^(?:- |\d+\. )',l):
   tag='ol' if re.match(r'^\d+\.',l) else 'ul';a=[re.sub(r'^(?:- |\d+\. )','',l)]
   while i<len(ls) and re.match(r'^(?:- |\d+\. )',ls[i]):a.append(re.sub(r'^(?:- |\d+\. )','',ls[i]));i+=1
   out.append(f'<{tag}>'+''.join('<li>'+inline(x)+'</li>' for x in a)+f'</{tag}>');continue
  if l.startswith('> '):out.append('<aside>'+inline(l[2:])+'</aside>');continue
  a=[l]
  while i<len(ls) and ls[i] and not re.match(r'^(?:#|\||>|- |\d+\. )',ls[i]) and not ls[i].startswith(chr(96)*3):a.append(ls[i]);i+=1
  out.append('<p>'+inline(' '.join(a))+'</p>')
 return '\n'.join(out)

import re, sys
src = open(sys.argv[1]).read().split('\n')
out = ['\t.text']
glob = set()
for l in src:
    m = re.match(r'^(_[A-Za-z0-9_]*):', l)
    if m: glob.add(m.group(1))
for g in sorted(glob): out.append('\t.globl\t' + g)
for l in src:
    s = l.rstrip()
    if s.startswith('!'): out.append('| ' + s[1:]); continue
    m = re.match(r'^\s*\.sect\s+\.(\w+)', s)
    if m: out.append('\t.' + ('text' if m.group(1) in ('text', 'rom') else m.group(1))); continue
    if re.match(r'^\s*\.extern', s): continue
    s = re.sub(r'^(\s*)\.data1\b', r'\1.byte', s)
    s = re.sub(r'^(\s*)\.data2\b', r'\1.word', s)
    s = re.sub(r'^(\s*)\.data4\b', r'\1.long', s)
    s = re.sub(r'^(\s*)\.align\s+2', r'\1.balign 2', s)
    s = s.replace('.Xlmul', '_X_lmul').replace('.Xulmul', '_X_ulmul').replace('.Xldiv', '_X_ldiv').replace('.Xuldiv', '_X_uldiv').replace('.Xasuldiv','_X_asuldiv').replace('.Xasldiv','_X_asldiv').replace('.Xasulmul','_X_asulmul').replace('.Xaslmul','_X_aslmul').replace('.Xasulrem','_X_asulrem').replace('.Xaslrem','_X_aslrem').replace('.Xlrem', '_X_lrem').replace('.Xulrem', '_X_ulrem')
    out.append(s)
open(sys.argv[2], 'w').write('\n'.join(out) + '\n')

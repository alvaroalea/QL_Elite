#!/usr/bin/env python3
"""Mutaciones de src/ql_ellrows.asm: cada una debe hacer fallar tools/test_ellrowsasm.py (si no, la prueba no cubre esa instruccion).
Dos son equivalentes y no se detectan: `cmp.w #254,d2` (con x1 = 255 el recorte da el mismo valor) y `cmp.w d4,d3` en vez de `cmp.w d7,d3` (solo
difiere con el mismo hi, lo viejo < hi y lo nuevo = hi, que en el contorno de una elipse no ocurre: ver CLAUDE.md)."""
import subprocess, os, tempfile
src = open('src/ql_ellrows.asm').read()
muts = [('\tbpl\tg1', '\tbmi\tg1'), ('\tbgt\tgnone', '\tbge\tgnone'), ('\tbeq\tnextrow\t\t\t; mismo tramo', '\tbne\tnextrow'), ('lsl.w\t#7,d1', 'lsl.w\t#6,d1'),
        ('\ttst.w\td3\n\tbne\ts1', '\ttst.w\td3\n\tbeq\ts1'), ('add.w\t#6,d0', 'add.w\t#5,d0'), ('\tcmp.w\t#255,d2\n\tble\tr2', '\tcmp.w\t#254,d2\n\tble\tr2'),
        ('\tbne\tr_hrun', '\tbeq\tr_hrun'), ('move.w\td2,-(sp)\t\t; la nueva', 'move.w\td3,-(sp)\t\t; la nueva'), ('\tbmi\tnextrow\t\t\t; ninguna', '\tbpl\tnextrow'),
        ('cmp.w\td7,d3\n\tbne\tdraw', 'cmp.w\td4,d3\n\tbne\tdraw'), ('move.w\td0,-(sp)\n\tmove.w\td4,-(sp)', 'move.w\td4,-(sp)\n\tmove.w\td0,-(sp)'),
        ('\tmove.w\t26(a6),d1\n\tbsr\tspans', '\tmove.w\t10(a6),d1\n\tbsr\tspans'), ('\tsub.w\td3,d0\t\t\t; x1 del tramo izquierdo', '\tadd.w\td3,d0'),
        ('\tbne\ts2\n\tmove.w\td1,d4\t\t\t; lo = hi', '\tbeq\ts2\n\tmove.w\td1,d4\t\t\t; lo = hi'), ('\tsub.w\td2,d4\n\tbmi\ts2', '\tsub.w\td2,d4\n\tbpl\ts2'),
        ('\tbgt\ts2b', '\tble\ts2b'), ('sub.w\td2,d1\t\t\t; (se deshace', 'add.w\td2,d1\t\t\t; (se deshace'),
        ('\tadd.w\td3,d3\n\tmoveq\t#0,d2\n\tmove.b\t1(a0,d3.w),d2', '\tadd.w\td3,d3\n\tmoveq\t#0,d2\n\tmove.b\t0(a0,d3.w),d2'),
        ('\tmove.w\td4,d2\n\tmove.w\td7,d3\n\tmove.w\t10(a6),d1', '\tmove.w\td4,d2\n\tmove.w\td7,d3\n\tmove.w\t26(a6),d1')]
bad = 0
for a, b in muts:
    if a not in src: print('NO ENCONTRADA', repr(a)[:45]); bad += 1; continue
    with tempfile.NamedTemporaryFile('w', suffix='.asm', delete=False) as f: f.write(src.replace(a, b, 1)); name = f.name
    det = False
    for seed in ('3', '4'):
        r = subprocess.run(['python3', 'tools/test_ellrowsasm.py', '30', seed], env=dict(os.environ, ELLROWSASM=name), capture_output=True, text=True)
        if r.returncode != 0: det = True; break
    os.unlink(name)
    print('detectada' if det else 'NO DETECTADA', repr(a)[:50], flush=True)
    bad += (not det)
print('no detectadas:', bad)

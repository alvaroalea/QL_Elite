#!/usr/bin/env python3
"""Inventario de rutinas de la fuente de referencia y su estado de port.
Lee las cabeceras ';       Name: X' / ';   Category: Y' / ';    Summary: Z' de elite-source.asm y la lista
tools/ported.txt (una linea por rutina portada: NOMBRE  funcion_C  estado, estado = verificada | sin-verificar | propia).
Genera INVENTARIO.md. La fuente de referencia no se incluye en el proyecto (ver tools/README_ref.txt)."""
import os, re, sys
src = os.environ.get("ELITE_SRC", "/tmp/moxon/elite-source-code-apple-ii-main/1-source-files/main-sources")
text = open(os.path.join(src, "elite-source.asm"), encoding="latin-1").read().split("\n")
ported = {}
for ln in open("tools/ported.txt"):
    ln = ln.split("#")[0].strip()
    if not ln: continue
    p = ln.split()
    ported[p[0]] = (p[1] if len(p) > 1 else "", p[2] if len(p) > 2 else "sin-verificar")
rows = []
i = 0
while i < len(text):
    m = re.match(r"^;\s+Name: (\S+)(.*)$", text[i])
    if m:
        name, extra = m.group(1), m.group(2).strip()
        cat = summ = ""
        for j in range(i, min(i + 8, len(text))):
            c = re.match(r"^;\s+Category: (.*)$", text[j])
            s = re.match(r"^;\s+Summary: (.*)$", text[j])
            if c: cat = c.group(1).strip()
            if s: summ = s.group(1).strip()
        rows.append((name, extra, cat, summ))
    i += 1
seen = {}
out = []
for name, extra, cat, summ in rows:
    if name in seen: continue          # rutinas en varias partes: una sola fila
    seen[name] = 1
    out.append((name, cat, summ))
tot = len(out)
done = sum(1 for n, c, s in out if n in ported)
ver = sum(1 for n, c, s in out if n in ported and ported[n][1] == "verificada")
bycat = {}
for n, c, s in out:
    bycat.setdefault(c or "(sin categoria)", []).append((n, s))
md = ["# Inventario de rutinas (Elite Apple II -> Elite QL)", "",
      "Generado por `tools/inventory.py` a partir de la fuente de referencia y de `tools/ported.txt`.", "",
      "Rutinas: %d. Portadas: %d (verificadas con el emulador: %d)" % (tot, done, ver), "",
      "Estados: **verificada** = comparada con el 6502 original (emulador); **transcripcion** = comparada con una transcripcion literal propia del 6502 (falta el emulador); **sin-verificar** = portada, en PENDIENTES.md; **propia** = sustituto no basado en la rutina; vacio = sin portar.", ""]
for cat in sorted(bycat, key=lambda k: k):
    items = bycat[cat]
    d = sum(1 for n, s in items if n in ported)
    md.append("## %s (%d/%d)" % (cat, d, len(items)))
    md.append("")
    md.append("| Rutina | Resumen | Estado | C |")
    md.append("|---|---|---|---|")
    for n, s in items:
        if n in ported:
            md.append("| %s | %s | %s | `%s` |" % (n, s, ported[n][1], ported[n][0]))
        else:
            md.append("| %s | %s |  |  |" % (n, s))
    md.append("")
open("INVENTARIO.md", "w").write("\n".join(md))
print("rutinas %d, portadas %d, verificadas %d" % (tot, done, ver))

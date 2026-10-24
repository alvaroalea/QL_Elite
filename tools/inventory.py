#!/usr/bin/env python3
"""Inventario de rutinas de la fuente de referencia y su estado de port.
Lee las cabeceras ';       Name: X' / ';   Category: Y' / ';    Summary: Z' de elite-source.asm y la lista
tools/ported.txt (una linea por rutina portada: NOMBRE  funcion_C  estado, estado = verificada | sin-verificar | propia).
Genera INVENTARIO.md. La fuente de referencia no se incluye en el proyecto (ver tools/README_ref.txt)."""
import os, re, sys
src = os.environ.get("ELITE_SRC", os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "ref", "moxon-appleII", "1-source-files", "main-sources"))
text = open(os.path.join(src, "elite-source.asm"), encoding="latin-1").read().split("\n")
ported = {}
for ln in open("tools/ported.txt"):
    ln = ln.split("#")[0].strip()
    if not ln: continue
    p = ln.split()
    ported[p[0]] = (p[1] if len(p) > 1 else "", p[2] if len(p) > 2 else "sin-verificar", " ".join(p[3:]))   # funcion C, estado, nota
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
def st(n): return ported[n][1] if n in ported else ""
VERIF = ("verificada",)
PEND_V = ("transcripcion", "sin-verificar", "propia")
def count(items):
    d = dict(total=len(items), ver=0, pv=0, nn=0, pi=0, nv=0)
    for n in items:
        s = st(n)
        if s in VERIF: d["ver"] += 1
        elif s in PEND_V: d["pv"] += 1
        elif s == "no-verificable": d["nv"] += 1
        elif s == "no-necesaria": d["nn"] += 1
        else: d["pi"] += 1                       # vacio o parcial
    d["port"] = d["ver"] + d["pv"] + d["nv"]
    return d
bycat = {}
for n, c, s in out:
    bycat.setdefault(c or "(sin categoria)", []).append((n, s))
allc = count([n for n, c, s in out])
md = ["# Inventario de rutinas (Elite Apple II -> Elite QL)", "",
      "Generado por `tools/inventory.py` a partir de la fuente de referencia (Mark Moxon, Elite Apple II) y de `tools/ported.txt`. No se edita a mano: se cambia `tools/ported.txt` y se ejecuta `python3 tools/inventory.py`.", "",
      "## Resumen", "",
      "- Rutinas de la fuente: **%d**" % allc["total"],
      "- **Portadas** (con la funcion C que las cubre): **%d**, de ellas verificadas contra el 6502: %d, **no verificables** (dependen del hardware del Apple II o de pantallas propias; no se vuelven a pedir): %d y pendientes de verificar: %d" % (allc["port"], allc["ver"], allc["nv"], allc["pv"]),
      "- **No necesarias** en la version QL (cargador, proteccion anticopia, RWTS del DOS, tablas de HGR, utilidades que da el compilador de C, etc.): **%d**" % allc["nn"],
      "- **Sin portar** (pendientes de implementar, incluidas las parciales): **%d**" % allc["pi"],
      "", "Comprobacion: total = portadas + no necesarias + sin portar (%d = %d + %d + %d)." % (allc["total"], allc["port"], allc["nn"], allc["pi"]), "",
      "| Area | Total | Portadas | De ellas verificadas | No verificables | No necesarias | Pendientes verificar | Pendientes implementar |",
      "|---|---:|---:|---:|---:|---:|---:|---:|"]
for cat in sorted(bycat):
    c = count([n for n, s in bycat[cat]])
    md.append("| %s | %d | %d | %d | %d | %d | %d | %d |" % (cat, c["total"], c["port"], c["ver"], c["nv"], c["nn"], c["pv"], c["pi"]))
md.append("| **Total** | **%d** | **%d** | **%d** | **%d** | **%d** | **%d** | **%d** |" % (allc["total"], allc["port"], allc["ver"], allc["nv"], allc["nn"], allc["pv"], allc["pi"]))
md += ["", "Columnas: *Portadas* = verificadas + no verificables + pendientes de verificar (incluye los equivalentes propios); *No verificables* = portadas que no tienen equivalente comparable en el original (estado no-verificable: no se piden en las rondas de verificacion); *Pendientes verificar* = estados transcripcion, sin-verificar y propia; *Pendientes implementar* = sin portar o parciales.", "",
       "## Rutinas sin portar, por area", ""]
for cat in sorted(bycat):
    pend = [n + (" (parcial)" if st(n) == "parcial" else "") for n, s in bycat[cat] if st(n) in ("", "parcial")]
    if pend: md.append("- **%s** (%d): %s" % (cat, len(pend), ", ".join("`%s`" % p if "(" not in p else "`%s` (parcial)" % p.split(" ")[0] for p in pend)))
md += ["", "## Estados", "",
       "**verificada** = comparada con el 6502 original (emulador); **transcripcion** = comparada con una transcripcion literal propia del 6502 (falta el emulador); **sin-verificar** = portada, anotada en PENDIENTES.md; **propia** = sustituto o equivalente propio (la funcion C cubre lo que hacia la rutina, con otra forma); **parcial** = existe una version reducida y falta parte (cuenta como pendiente de implementar); **no-verificable** = portada, pero no se puede comparar con el 6502 (depende del hardware del Apple II, como el modo texto, o de pantallas propias del port): queda fuera de los pendientes de verificar; **no-necesaria** = no hace falta en la version QL (la nota dice por que); vacio = sin portar.", ""]
for cat in sorted(bycat):
    items = bycat[cat]
    c = count([n for n, s in items])
    md.append("## %s (%d portadas de %d%s%s)" % (cat, c["port"], c["total"], (", %d no necesarias" % c["nn"]) if c["nn"] else "", (", %d sin portar" % c["pi"]) if c["pi"] else ""))
    md.append("")
    md.append("| Rutina | Resumen | Estado | C | Nota |")
    md.append("|---|---|---|---|---|")
    for n, s in items:
        if n in ported:
            md.append("| %s | %s | %s | %s | %s |" % (n, s, ported[n][1], (", ".join("`%s`" % f for f in ported[n][0].split(","))) if ported[n][0] != "-" else "", ported[n][2]))
        else:
            md.append("| %s | %s |  |  |  |" % (n, s))
    md.append("")
open("INVENTARIO.md", "w").write("\n".join(md))
print("rutinas %d, portadas %d (verificadas %d), no necesarias %d, sin portar %d" % (allc["total"], allc["port"], allc["ver"], allc["nn"], allc["pi"]))

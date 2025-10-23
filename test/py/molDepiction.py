post.summarySection("Hydrogen moving and charges")
for b in ["", "=", "#"]:
    for c in ["", "+", ".", "+."]:
        for l in [6, 8, 10, 12]:
            for h in ["", "2"]:
                unit = "[CH%s%s]" % (h, c)
                s = [unit + "1"]
                for i in range(l - 1):
                    s.append(unit)
                s.append("1")
                Graph.fromSMILES(b.join(s), name="b%s-c%s-l%d-h%s" % (b, c, l, h))
p = GraphPrinter()
p.setMolDefault()
p.simpleCarbons = False
p.withIndex = True
for a in inputGraphs: a.print(p)
inputGraphs[:] = []

post.summarySection("Double/Triple/Lablled bond")
Graph.fromSMILES("O=C=C")
Graph.fromSMILES("CCCC#N")
Graph.fromSMILES("CCC(O)=O")
Graph.fromSMILES("CC(=O)C")
Graph.fromDFS("C{a}C")
for a in inputGraphs: a.print()
inputGraphs[:] = []
post.summarySection("middle C -> simple C + hidden H")
Graph.fromSMILES("CCC")
Graph.fromSMILES("CC(O)C")
Graph.fromSMILES("N=CC")
for a in inputGraphs: a.print()
inputGraphs[:] = []
post.summarySection("collapse H")
Graph.fromSMILES("[H][H]")
Graph.fromDFS("[Q][H][R]")
Graph.fromDFS("[H]=[R]")
Graph.fromSMILES("O")
Graph.fromSMILES("CNC")
for a in inputGraphs: a.print()
inputGraphs[:] = []
post.summarySection("Aromatic")
Graph.fromSMILES("c1ccccc1")
Graph.fromSMILES("c1[nH]c2c(ncnc2n1)N")
for a in inputGraphs: a.print()
inputGraphs[:] = []
post.summarySection("Charges")
Graph.fromSMILES("[OH-]")
Graph.fromDFS("[O2-]")
Graph.fromSMILES("[H+]")
Graph.fromSMILES("[NH4+]")
Graph.fromDFS("O[N2+](O)(O)O")
Graph.fromDFS("[Q]C[Hest]C[Q+]")
Graph.fromSMILES("O[C-]C(=O)COP([O-])([O-])=O")
Graph.fromSMILES("O[CH-]C(=O)COP([O-])([O-])=O")
for a in inputGraphs: a.print()

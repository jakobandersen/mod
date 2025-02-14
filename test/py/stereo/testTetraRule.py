r = ruleGMLString("""rule [
    left [
        node [ id 10 label "H" ]
    ]
    context [
        node [ id 0 label "C" ]
        edge [ source 0 target 10 label "-" ]
    ]
    right [
        node [ id 10 label "C" stereo "[0, 11, 12, 13]!" ]
        node [ id 11 label "H" ]
        node [ id 12 label "H" ]
        node [ id 13 label "H" ]
        edge [ source 10 target 11 label "-" ]
        edge [ source 10 target 12 label "-" ]
        edge [ source 10 target 13 label "-" ]
    ]
]""")
g = smiles("[O][C@]([C@H3])([C@H2][C@H3])([C@H2][C@H3])")

post.summaryChapter("DG")
ls = LabelSettings(LabelType.String, LabelRelation.Isomorphism, LabelRelation.Specialisation)
dg = DG(graphDatabase=inputGraphs, labelSettings=ls)
dg.build().execute(addSubset(g) >> r, verbosity=100)
dg.print()
for a in dg.graphDatabase:
	a.print()

post.summaryChapter("RC")
rc = RCEvaluator([r], labelSettings=ls)
res = rc.eval(rcBind(g) *rcSuper* r)
print("Res:", len(res))
for a in res: a.print()

post.summaryChapter("Iso")
g1 = smiles("[O][C@]([C@H3])([C@H2][C@H3])([C@H2][C@H]([C@H3])([C@H3]))")
g2 = smiles("[O][C@@]([C@H3])([C@H2][C@H3])([C@H2][C@H]([C@H3])([C@H3]))")
p = GraphPrinter()
p.withPrettyStereo = True
g1.print(p)
g2.print(p)
ls = LabelSettings(LabelType.String, LabelRelation.Isomorphism, LabelRelation.Isomorphism)
assert not g1.isomorphism(g2, labelSettings=ls)

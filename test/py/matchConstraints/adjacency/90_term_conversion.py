ls = LabelSettings(LabelType.Term, LabelRelation.Specialisation)
rId = Rule.fromGMLString("""rule [
    context [
        node [ id 0 label "C" ]
    ]
    right [
        node [ id 1 label "R" ]
        edge [ source 0 target 1 label "-" ]
    ]
]""")
rIdInv = rId.makeInverse()
rStr = """rule [
    context [
        node [ id 0 label "C" ]
        node [ id 1 label "Q" ]
        edge [ source 0 target 1 label "-" ]
    ]
    constrainAdj [
        id 1
        nodeLabels [ label "a" label "_x" label "b(c)" label "d(_y)" ]
        edgeLabels [ label "g" label "_p" label "h(i)" label "j(_q)" ]
		op "=" count 1
    ]
]"""
a = Rule.fromGMLString(rStr)
post.summaryChapter("TermState")
a.printTermState()
#b = a.makeInverse()

post.summaryChapter("Compose first")
rc = RCEvaluator([], ls)
res = rc.eval(a *rcSuper* rId)
for b in res:
    b.print()
    b.printTermState()

post.summaryChapter("Compose second")
rc = RCEvaluator([], ls)
res = rc.eval(rIdInv *rcSub* a)
for b in res:
    b.print()
    b.printTermState()

post.summaryChapter("DGRuleComp")
Graph.fromDFS("C[Q]")
dg = DG(labelSettings=ls)
dg.build().execute(addSubset(inputGraphs) >> a)
dg.print()

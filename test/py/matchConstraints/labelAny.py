smiles("[C]")
smiles("[O]")
smiles("[N]")
graphDFS("[x(_a)]")

a = ruleGMLString("""rule [
    left [
        node [ id 0 label "_x" ]
    ]
    right [
        node [ id 0 label "a(_x)" ]
    ]
    constrainLabelAny [
        label "q(_x)"
        labels [ label "q(C)" label "q(N)" ]
    ]
]""")
a.print()
a.printTermState()
print(a.getGMLString())
dg = DG(graphDatabase=inputGraphs, labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation))
dg.build().execute(addSubset(inputGraphs) >> inputRules)
dg.print()

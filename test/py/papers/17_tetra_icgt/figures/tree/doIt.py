include("../stereoIsoCommon.py")

#g = smiles("O[C@H]([C@H]([C@H3])([C@H2][C@H3]))([C@H]([C@H3])([C@H2][C@H3]))")
#g = smiles("N[C@](O)([C@](S)(P)(O))([C@](S)(P)(O))")
g = smiles("[N][C@]([O])([C@]([S])([P])([O]))([C@]([S])([P])([O]))")

dg = DG(graphDatabase=inputGraphs,
	labelSettings=LabelSettings(LabelType.Term, LabelRelation.Specialisation, LabelRelation.Specialisation))
dg.build().execute(addSubset(g) >> repeat(change))
p = DGPrinter()
p.withRuleName = True
p.withRuleId = False
dg.print(p)

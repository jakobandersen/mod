include("2xx_morphisms_helpers.py")

lsIso = LabelSettings(LabelType.Term, LabelRelation.Isomorphism)
lsSpec = LabelSettings(LabelType.Term, LabelRelation.Specialisation)
lsUni = LabelSettings(LabelType.Term, LabelRelation.Unification)

ab = Graph.fromDFS("[t(_A)][t(_B)]")
cd = Graph.fromDFS("[t(_C)][t(_D)]")
cdq = Graph.fromDFS("[t(_C)][t(_D)][Q]")
xx = Graph.fromDFS("[t(_X)][t(_X)]")
xxq = Graph.fromDFS("[t(_X)][t(_X)][Q]")
fab = Graph.fromDFS("[t(f(_A))][t(_B)]")
gab = Graph.fromDFS("[t(g(_A))][t(_B)]")
gabq = Graph.fromDFS("[t(g(_A))][t(_B)][Q]")

assert ab.isomorphism(cd, labelSettings=lsIso) > 0
assert ab.isomorphism(cd, labelSettings=lsSpec) > 0
assert ab.isomorphism(cd, labelSettings=lsUni) > 0

assert ab.isomorphism(xx, labelSettings=lsIso) == 0
assert ab.isomorphism(xx, labelSettings=lsSpec) > 0
assert ab.isomorphism(xx, labelSettings=lsUni) > 0

assert fab.isomorphism(gab, labelSettings=lsIso) == 0
assert fab.isomorphism(gab, labelSettings=lsSpec) == 0
assert fab.isomorphism(gab, labelSettings=lsUni) > 0

res = [
	[(0, 0), (1, 1)],
	[(0, 1), (1, 0)],
]
check(ab.enumerateIsomorphisms, cd, res, ls=lsIso)
check(ab.enumerateIsomorphisms, cd, res, ls=lsSpec)
check(ab.enumerateIsomorphisms, cd, res, ls=lsUni)

check(ab.enumerateIsomorphisms, xx, [], ls=lsIso)
check(ab.enumerateIsomorphisms, xx, res, ls=lsSpec)
check(ab.enumerateIsomorphisms, xx, res, ls=lsUni)

check(fab.enumerateIsomorphisms, gab, [], ls=lsIso)
check(fab.enumerateIsomorphisms, gab, [], ls=lsSpec)
check(fab.enumerateIsomorphisms, gab, [[(0, 1), (1, 0)]], ls=lsUni)


assert ab.monomorphism(cdq, labelSettings=lsIso) > 0
assert ab.monomorphism(cdq, labelSettings=lsSpec) > 0
assert ab.monomorphism(cdq, labelSettings=lsUni) > 0

assert ab.monomorphism(xxq, labelSettings=lsIso) == 0
assert ab.monomorphism(xxq, labelSettings=lsSpec) > 0
assert ab.monomorphism(xxq, labelSettings=lsUni) > 0

assert fab.monomorphism(gabq, labelSettings=lsIso) == 0
assert fab.monomorphism(gabq, labelSettings=lsSpec) == 0
assert fab.monomorphism(gabq, labelSettings=lsUni) > 0

res = [
	[(0, 0), (1, 1)],
	[(0, 1), (1, 0)],
]
check(ab.enumerateMonomorphisms, cdq, res, ls=lsIso)
check(ab.enumerateMonomorphisms, cdq, res, ls=lsSpec)
check(ab.enumerateMonomorphisms, cdq, res, ls=lsUni)

check(ab.enumerateMonomorphisms, xxq, [], ls=lsIso)
check(ab.enumerateMonomorphisms, xxq, res, ls=lsSpec)
check(ab.enumerateMonomorphisms, xxq, res, ls=lsUni)

check(fab.enumerateMonomorphisms, gabq, [], ls=lsIso)
check(fab.enumerateMonomorphisms, gabq, [], ls=lsSpec)
check(fab.enumerateMonomorphisms, gabq, [[(0, 1), (1, 0)]], ls=lsUni)

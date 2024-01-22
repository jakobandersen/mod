include("2xx_morphisms_helpers.py")

co2 = smiles("O=C=O", "CO2")
co = smiles("[C]=O", name="CO")
co2_2 = smiles("O=C=O", "CO2 2")

assert co2.isomorphism(co2_2) > 0
assert co2.isomorphism(co) == 0

check(co2.enumerateIsomorphisms, co2_2, [
	[(0, 0), (1, 1), (2, 2)],
	[(0, 0), (1, 2), (1, 2)],
])
check(co2.enumerateIsomorphisms, co2_2, [], numMaps=0)
check(co2.enumerateIsomorphisms, co, [])
check(co.enumerateIsomorphisms, co2, [])
check(co.enumerateIsomorphisms, co2, [], numMaps=0)
check(co2.enumerateIsomorphisms, co, [])


assert co.monomorphism(co2) > 0
assert co2.monomorphism(co2_2) > 0
assert co2.monomorphism(co) == 0

check(co2.enumerateMonomorphisms, co2_2, [
	[(0, 0), (1, 1), (2, 2)],
	[(0, 0), (1, 2), (1, 2)],
])
check(co2.enumerateMonomorphisms, co2_2, [], numMaps=0)
check(co2.enumerateMonomorphisms, co, [])
check(co.enumerateMonomorphisms, co2, [
	[(0, 0), (1, 1)],
	[(0, 0), (1, 2)],
])
check(co.enumerateMonomorphisms, co2, [], numMaps=0)
check(co2.enumerateMonomorphisms, co, [])

# check that the graphs and maps are still there
res = []
def c(m):
	res.append(m)
smiles('[C]', 'C').enumerateMonomorphisms(smiles('[C]', 'C 2'),
	callback=c)
assert res[0].domain.name == 'C'
assert res[0].codomain.name == 'C 2'
assert res[0][next(iter(res[0].domain.vertices))] == \
              next(iter(res[0].codomain.vertices))
assert res[0].inverse(next(iter(res[0].codomain.vertices))) == \
                      next(iter(res[0].domain.vertices))

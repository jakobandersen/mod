include("../xxx_helpers.py")

g = smiles("O")
fail(lambda: g.isomorphism(None), "codomain is null.")
fail(lambda: g.monomorphism(None), "codomain is null.")

fail(lambda: g.enumerateMonomorphisms(None, callback=False), 
	"codomain is null.")


co2 = smiles("O=C=O", "CO2")
co = smiles("[C]=O", name="CO")
co2_2 = smiles("O=C=O", "CO2 2")

assert(co2.isomorphism(co2_2) > 0)
assert(co2.isomorphism(co) == 0)
assert(co.monomorphism(co2) > 0)
assert(co2.monomorphism(co2_2) > 0)
assert(co2.monomorphism(co) == 0)

def check(f, codom, maps, numMaps=2**30):
	exp = list(sorted(maps))
	res = []
	def c(m):
		if len(res) >= numMaps:
			return False
		m_ = []
		for v in m.domain.vertices:
			assert m[v]
			assert m.inverse(m[v]) == v
			m_.append((v.id, m[v].id))
		res.append(m_)
		if len(res) >= numMaps:
			return False
		return True
	f(codom, callback=c)
	res = list(sorted(res))
	if len(res) != len(exp):
		print("res:", res)
		print("exp:", exp)


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

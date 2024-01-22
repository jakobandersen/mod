include("../xxx_helpers.py")

lsString = LabelSettings(LabelType.String, LabelRelation.Isomorphism)

def check(f, codom, maps, numMaps=2**30, ls=lsString):
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
	f(codom, callback=c, labelSettings=ls)
	res = list(sorted(res))
	if len(res) != len(exp):
		print("res:", res)
		print("exp:", exp)

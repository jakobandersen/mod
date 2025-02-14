def printMap(m, name, compact):
	dom = [
		"{}:{:2s}".format(v.id, v.stringLabel)
		for v in m.domain.vertices
	]
	codom = [
		"{}:{:2s}".format(
			m[v].id if m[v] else ".",
			m[v].stringLabel if m[v] else ".")
		for v in m.domain.vertices
	]

	domInv = [
		"{}:{:2s}".format(v.id, v.stringLabel)
		for v in m.codomain.vertices
	]
	codomInv = [
		"{}:{:2s}".format(
			m.inverse(v).id if m.inverse(v) else ".",
			m.inverse(v).stringLabel if m.inverse(v) else ".")
		for v in m.codomain.vertices
	]

	if not compact:
		print(f"{name}: {dom}")
		print(" "*(len(name) - 2), f"-> {codom}")
		print(f"{name} inverse: {domInv}")
		print(" "*(len(name) - 2 + 8), f"-> {codomInv}")
	else:
		print(f"{name}: {dom} -> {codom}, {domInv} -> {codomInv}", end=" | ")


def checkMapInvertible(m):
	for v in m.domain.vertices:
		if m[v]:
			assert m.inverse(m[v]) == v, (m, m[v], m.inverse(m[v]))

def checkMaps(maps, silent: bool):
	for res in maps:
		if not silent:
			compact = False
			printMap(res.map, "map", compact)
			printMap(res.match, "match", compact)
			printMap(res.comatch, "comatch", compact)
			if compact:
				print()
		checkMapInvertible(res.map)
		checkMapInvertible(res.match)
		checkMapInvertible(res.comatch)

		for vL in res.rule.left.vertices:
			vR = vL.core.right
			vG = res.match[vL]
			assert vG
			vH = res.map[vG]
			if vR:
				assert res.comatch[vR] == vH
			else:
				assert not vH
		for vR in res.rule.right.vertices:
			vL = vR.core.left
			vH = res.comatch[vR]
			assert vH
			vG = res.map.inverse(vH)
			if vL:
				assert res.match[vL] == vG
			else:
				assert not vG
		for vG in maps.left.vertices:
			vH = res.map[vG]
			vL = res.match.inverse(vG)
			if vL:
				continue
			else:
				assert vH
				assert not res.comatch.inverse(vH)
		for vH in maps.right.vertices:
			vG = res.map.inverse(vH)
			vR = res.comatch.inverse(vH)
			if vR:
				continue
			else:
				assert vG
				assert not res.match.inverse(vG)

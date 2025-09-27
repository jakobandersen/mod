include("xxx_helpers.py")

def checkVertexMap(dom, codom, m):
	assert m.domain == dom
	assert m.codomain == codom
	fail(lambda: m[dom.Vertex()], "Can not map null vertex.")
	fail(lambda: m.inverse(codom.Vertex()), "Can not map null vertex.")

	for vDom in m.domain.vertices:
		vCodom = m[vDom]
		if not vCodom:
			continue
		assert vCodom.graph == codom
		vDomInv = m.inverse(vCodom)
		assert vDomInv.graph == dom, f"{vDomInv.graph}, {dom}"
		assert vDom == vDomInv, f"{vDom}, {vDomInv}"

	for vCodom in m.codomain.vertices:
		vDom = m.inverse(vCodom)
		if not vDom:
			continue
		assert vDom.graph == dom
		vCodomInv = m[vDom]
		assert vCodomInv.graph == codom
		assert vCodom == vCodomInv

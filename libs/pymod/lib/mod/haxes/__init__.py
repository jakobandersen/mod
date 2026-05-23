import mod

from typing import Union

def ruleFromReactionSmiles(line: str, name=None, allowAbstract=False, *, invert: bool=False, add: bool=True) -> mod.Rule:
	sLeft, sRight = line.split(">>")
	ssLeft = sLeft.split(".")
	ssRight = sRight.split(".")
	mLeft = [mod.Graph.fromSMILES(s, allowAbstract=allowAbstract, add=False) for s in ssLeft]
	mRight = [mod.Graph.fromSMILES(s, allowAbstract=allowAbstract, add=False) for s in ssRight]
	def printGraph(g):
		extFromInt = {}
		for iExt in range(g.minExternalId, g.maxExternalId + 1):
			v = g.getVertexFromExternalId(iExt)
			if not v.isNull():
				extFromInt[v] = iExt
		s = ""
		for v in g.vertices:
			assert v in extFromInt
			s += '\t\tnode [ id %d label "%s" ]\n' % (extFromInt[v], v.stringLabel)
		for e in g.edges:
			s += '\t\tedge [ source %d target %d label "%s" ]\n' % (extFromInt[e.source], extFromInt[e.target], e.stringLabel)
		return s
	s = "rule [\n\tleft [\n"
	for m in mLeft:
		s += printGraph(m)
	s += "\t]\n\tright [\n"
	for m in mRight:
		s += printGraph(m)
	s += "\t]\n]\n"
	return mod.Rule.fromGMLString(s, name=name, invert=invert, add=add)


def reactionSmilesFromRule(r: mod.Rule) -> str:
	def replaceIds(s: str, g: mod.Graph) -> str:
		import re
		# put an 'o' in front of each ID to distinguish it as original
		s = re.sub(":([0-9]+)]", ":o\\1]", s)
		# now replace the original IDs that are the IDs from loaded graphs
		# replace with the original IDs in the rule
		for i in range(g.minExternalId, g.maxExternalId + 1):
			v = g.getVertexFromExternalId(i)
			if not v:
				continue
			s = s.replace(f":o{v.id}]", f":{i}]")
		return s

	def makeSideGraphs(sg: Union[mod.Rule.LeftGraph, mod.Rule.RightGraph]) -> str:
		s = "graph [\n"
		for v in sg.vertices:
			s += f'	node [ id {v.id} label "{v.stringLabel}" ]\n'
		for e in sg.edges:
			s += f'	edge [ source {e.source.id} target {e.target.id} label "{e.stringLabel}" ]\n'
		s += "]\n"
		gs = mod.Graph.fromGMLStringMulti(s, add=False)
		for g in gs:
			s = g.smilesWithIds
			s = replaceIds(s, g)

		return ".".join(replaceIds(g.smilesWithIds, g) for g in gs)
	return f"{makeSideGraphs(r.left)}>>{makeSideGraphs(r.right)}"

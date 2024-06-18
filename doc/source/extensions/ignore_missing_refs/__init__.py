
from sphinx.errors import NoUri

exacts = {
	'cpp': [
		"AtomIds",
		"graph",
		"rule", "rule::RCExp", "RCExp",
		"dg",
		"post",
		"lib", "boost",
	],
	'py': [
		"DG.RuleRange", "DG.SourceRange", "DG.TargetRange", "DG.OutEdgeRange", "DG.InEdgeRange", "DG.EdgeRange", "DG.VertexRange",
	],
}
prefixes = {
	'cpp': [
		"Function<", "lib::", "boost::",
	]
}

def missing_reference(app, env, node, contnode):
	target = node['reftarget']
	typ = node['reftype']
	domain = node.get('refdomain')
	if domain in exacts and target in exacts[domain]:
		raise NoUri()
	if domain in prefixes and any(target.startswith(a) for a in prefixes[domain]):
		raise NoUri()


def setup(app):
	app.connect('missing-reference', missing_reference)
	return {
		'version': '0.1',
		'parallel_read_safe': True,
		'parallel_write_safe': True,
	}

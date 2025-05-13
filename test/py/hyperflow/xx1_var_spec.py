post.disableInvokeMake()

varRegistry = {}

def registerVar(module, typ, name, alsoRelaxed, solCheck):
	assert typ in ("vertex", "edge", "custom")
	if module not in varRegistry:
		varRegistry[module] = []
	if all(name != t[1] for t in varRegistry[module]):
		varRegistry[module].append((typ, name, alsoRelaxed, solCheck))

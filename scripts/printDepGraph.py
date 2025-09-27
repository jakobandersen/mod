import os
import sys

def clusterComponentFromFilename(f):
	cs = f.split("/")
	if "." in cs[-1]:
		cs.pop()
	assert cs[0] == 'mod'
	if len(cs) == 1:
		return 'interface', cs[0]
	cs = cs[1:]
	if len(cs) == 1:
		if cs[0] == 'lib':
			return 'lib', 'lib'
		if cs[0] == 'internal':
			return 'internal', 'internal'
		return 'interface', cs[0]
	if cs[1] == 'internal':
		return 'internal', cs[0]
	return cs[0], cs[1]
	
mods = set()
deps = {}

try:
	os.chdir("libs/libmod/src")
except FileNotFoundError as e:
	print(e)
	print("Run from the root of the repository.")
	sys.exit(1)
for root, dirs, files in os.walk("mod"):
	mod = clusterComponentFromFilename(root)
	mods.add(mod)
	if mod not in deps:
		deps[mod] = {}
	for file_ in files:
		fName = os.path.join(root, file_)
		with open(fName) as f:
			for line in f:
				if not line.startswith("#include <mod"):
					continue
				first = line.find("<") + 1
				last = line.find(">")
				line = line[first:last]
				depMod = clusterComponentFromFilename(line)	
				mods.add(depMod)
				if depMod not in deps[mod]:
					deps[mod][depMod] = 0
				deps[mod][depMod] += 1

utilMods = (
	('interface', 'mod'),
	('lib', 'lib'),
	('lib', 'Chem'),
)
clusterColour = {
	'interface': 'green',
	'internal': 'orange',
	'lib':	'blue',
}

def printNode(mod):
	print('\t"{}/{}" [ label="{}" ];'.format(mod[0], mod[1], mod[1]))
def ignoreEdge(mod, depMod):
	if mod in utilMods:
		return 'util'
	if depMod in utilMods:
		return 'util'
	if (mod, depMod) == (('interface', 'flow'), ('interface', 'causality')):
		return 'deprecated'
	if mod == ('lib', 'Flow') or depMod == ('lib', 'Flow'):
		return 'deprecated'
	return 'dep'
def printEdge(mod, depMod, count, indent=False):
	if depMod == mod:
		return
	options = ""
	ignore = ignoreEdge(mod, depMod)
	if ignore == 'util':
		options += " constraint=false color=gray style=solid"
		return
	if ignore == 'deprecated':
		options += " constraint=false color=gray style=dashed"
	print('{}"{}/{}" -> "{}/{}" [ label="{}"{} ];'.format(
		"\t" if indent else "",
		mod[0], mod[1], depMod[0], depMod[1], count, options))
def makeCluster(cluster):
	print("subgraph cluster_{} {{\n\tcolor={};".format(
		cluster, clusterColour[cluster]))
	print("\tlabel = \"{}\";".format(cluster))
	for mod in mods:
		if mod[0] == cluster:
			printNode(mod)
	for mod, data in deps.items():
		if mod[0] != cluster:
			continue
		for depMod, count in data.items():
			if depMod[0] != cluster:
				continue
			printEdge(mod, depMod, count, True)
	print("}")

print('''digraph g {
rankdir="LR"
node [ shape=box ];
''')
makeCluster("lib")
makeCluster("interface" )
makeCluster("internal")
for mod, data in deps.items():
	for depMod, count in data.items():
		if mod[0] == depMod[0]:
			continue
		printEdge(mod, depMod, count)
print("}")

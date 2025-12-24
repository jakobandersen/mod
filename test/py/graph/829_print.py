strs = [
	"[C@]([H])(O)(S)(P)",
	"[C@]([H])(O)(S)(P=C)",
	"[C@]([H])(O)(S)(P#C)",
]

for s in strs:
	g = Graph.fromSMILES(s)
	g.print()

def p(dfs, name):
	g = Graph.fromDFS(dfs, name=name)
	g.print()

p("[U][C]",                "UC")
p("[U][C]([H])",           "UCH")
p("[U][C]([H])([H])",      "UCH2")
p("[U][C]([H])([H])([H])", "UCH3")

p("[U][C][V]",           "UCV")
p("[U][C]([H])[V]",      "UCHV")
p("[U][C]([H])([H])[V]", "UCH2V")

for a in "-=:#pq":
	for b in "-=:#pq":
		p(f"[U]{{{a}}}[C]{{{b}}}[V]",      f"UCV{a}{b}")
		p(f"[U]{{{a}}}[C]([H]){{{b}}}[V]", f"UCHV{a}{b}")

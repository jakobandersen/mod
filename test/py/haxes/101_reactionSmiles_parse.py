strs = [
	"[C:8]([C:7]1[C:0]([H:11])=[C:1]([C:2]([H:13])=[C:3]([C:4]=1[O:5][C:6]([H:17])([H:16])[H:15])[H:14])[H:12])([O-:10])=[O:9]>>[C:0]1([C-:7]=[C:4]([C:3]([H:14])=[C:2]([C:1]=1[H:12])[H:13])[O:5][C:6]([H:15])([H:16])[H:17])[H:11].[C:8](=[O:10])=[O:9]",
	"[C:0]1([C-:7]=[C:4]([C:3]([H:11])=[C:2]([C:1]=1[H:9])[H:10])[O:5][C:6]([H:14])([H:13])[H:12])[H:8]>>[C:1]1([C:2]([H:10])=[C:3]([C:4]([O:5][C-:6]([H:12])[H:13])=[C:7]([C:0]=1[H:8])[H:14])[H:11])[H:9]",
	"[C:1]1([C:2]([H:10])=[C:3]([C:4]([O:5][C-:6]([H:14])[H:13])=[C:7]([C:0]=1[H:8])[H:12])[H:11])[H:9]>>[C:3]1([C-:4]=[C:7]([C:0]([H:8])=[C:1]([C:2]=1[H:10])[H:9])[H:12])[H:11].[C:6]([H:13])([H:14])=[O:5]",
	("[a:1]>>[b:1]", True),
]
for s in strs:
	if isinstance(s, tuple):
		s, allowAbstract = s
	else:
		allowAbstract = False
	r = haxes.ruleFromReactionSmiles(s, allowAbstract=allowAbstract)
	p = GraphPrinter()
	p.setReactionDefault()
	p.withIndex = True
	r.print(p)

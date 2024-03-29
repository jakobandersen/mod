def p(dfs):
	print(dfs)
	post.summarySection(dfs)
	r = Rule.fromDFS(dfs)
	p = GraphPrinter()
	r.print(p, printCombined=True)
	p.withGraphvizCoords = True
	r.print(p)
	r.printTermState()

print("=" * 80)
print("K")
print("-" * 80)
p("[x]1{e}[y]2>>[a]1{e}[b]2")
p("[x]1{q}[y]2>>[a]1{r}[b]2")
p("[C]1-[S]2>>[N]1-[O]2")
p("[C]1=[S]2>>[N]1#[O]2")

print("=" * 80)
print("L")
print("-" * 80)
p("[x]{q}[y]>>")
p("[C]=[H]>>")

print("=" * 80)
print("R")
print("-" * 80)
p(">>[x]{q}[y]")
p(">>[C]=[H]")

print("=" * 80)
print("Combinations K")
print("-" * 80)
# offset a side to provoke index out of bounds problems
p("[ox]{oq}[oy].[x]1{e}[y]2>>[a]1{e}[b]2")
p("[ox]{oq}[oy].[x]1{q}[y]2>>[a]1{r}[b]2")
p("[ox]{oq}[oy].[C]1-[S]2>>[N]1-[O]2")
p("[ox]{oq}[oy].[C]1=[S]2>>[N]1#[O]2")
p("[x]1{e}[y]2>>[ox]{oq}[oy].[a]1{e}[b]2")
p("[x]1{q}[y]2>>[ox]{oq}[oy].[a]1{r}[b]2")
p("[C]1-[S]2>>[ox]{oq}[oy].[N]1-[O]2")
p("[C]1=[S]2>>[ox]{oq}[oy].[N]1#[O]2")

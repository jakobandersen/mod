include("formoseCommon/grammar.py")

dg = DG(graphDatabase=inputGraphs)
dg.build().execute(addSubset(inputGraphs) >> inputRules[0] >> inputRules[2])

printer = DGPrinter()
post.summarySection("Const False")
printer.pushVertexVisible(False)
dg.print(printer)
printer.popVertexVisible()
post.summarySection("Const True")
printer.pushVertexVisible(True)
dg.print(printer)
printer.popVertexVisible()

post.summarySection("Func")
def f(v): return all(v.graph != a for a in inputGraphs)
printer.pushVertexVisible(f)
dg.print(printer)
printer.popVertexVisible()

printer.pushVertexVisible(True)
dg.print(printer)
printer.popVertexVisible()

post.summarySection("Lambda")
printer.pushVertexVisible(lambda v: all(v.graph != a for a in inputGraphs))
dg.print(printer)
printer.popVertexVisible()

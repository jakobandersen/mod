include("eventTrace_05x_print_common.py")

dg = DG()
res = dg.build().addAbstract("A -> B")
net = causality.Net(dg)
marking = causality.Marking(net)
marking.add(res.getGraph("A"), 6)
trace = causality.EventTrace(marking)
doPrint(trace)

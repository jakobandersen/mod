include("xxx_common.py")

fail(lambda: hyperflow.ModelImplementationView(None), "Model is null.")

makeFlow("A -> B")
unlocked(lambda: hyperflow.ModelImplementationView(flow))
unlocked(lambda: flow.implementationView)

flow.findSolutions(maxNumSolutions=0)

hyperflow.ModelImplementationView(flow)
view = flow.implementationView

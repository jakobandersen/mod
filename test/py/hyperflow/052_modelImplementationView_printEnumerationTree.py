include("xxx_common.py")

makeFlow("A -> B")

flow.findSolutions(maxNumSolutions=0)
view = flow.implementationView

post.enableInvokeMake()

view.printEnumerationTree()
flow.findSolutions()
view.printEnumerationTree()

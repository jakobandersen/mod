def doPrint(t, *, caption=None, tp=None):
	if caption is not None:
		post.summaryChapter(caption)
	if tp is None:
		tp = causality.EventTracePrinter()
	t.print(tp)
	tp.logTime = True
	t.print(tp)

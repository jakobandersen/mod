include("xxx_helpers.py")

assert type(makeUniqueFilePrefix()) == str

post.command("echo hello")

post.flushCommands()
checkDeprecated(lambda: postFlush())

post.disableCommands()
checkDeprecated(lambda: postDisable())

post.enableCommands()
checkDeprecated(lambda: postEnable())

post.reopenCommandFile()
checkDeprecated(lambda: postReset())


post.enableCommands()

post.summaryChapter("Some chapter")
checkDeprecated(lambda: postChapter("Some other chapter"))

post.summarySection("Some section")
checkDeprecated(lambda: postSection("Some other section"))

post.summaryRaw(r"$\frac{a}{b}$")
post.summaryRaw(r"$\frac{c}{d}$", "secondRaw.whatever")

with open("out/input.txt", "w") as f:
	f.write(r"$\frac{e}{f}$")
post.summaryInput("out/input.txt")

post.disableCompileSummary()
post.enableCompileSummary()
post.disableInvokeMake()
post.enableInvokeMake()

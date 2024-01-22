#include "Post.hpp"

#include <mod/Error.hpp>
#include <mod/lib/IO/IO.hpp>

#include <iomanip>

namespace mod::post {

FileHandle::FileHandle(const std::string &name) : name(name) {
	stream.open(name.data());
	if(!stream) throw LogicError("Can not open file '" + this->name + "'.");
}

std::string makeUniqueFilePrefix() {
	return lib::IO::makeUniqueFilePrefix();
}

void command(const std::string &line) {
	lib::IO::post() << line << '\n';
}

void flushCommands() {
	lib::IO::post() << std::flush;
}

void disableCommands() {
	lib::IO::postDisable();
}

void enableCommands() {
	lib::IO::postEnable();
}

void reopenCommandFile() {
	lib::IO::postReopenCommandFile();
}

void summaryChapter(const std::string &heading) {
	lib::IO::post() << "summaryChapter \"" << heading << "\"\n";
}

void summarySection(const std::string &heading) {
	lib::IO::post() << "summarySection \"" << heading << "\"\n";
}

void summaryRaw(const std::string &latexCode) {
	summaryRaw(latexCode, "raw.tex");
}

void summaryRaw(const std::string &latexCode, const std::string &file) {
	FileHandle s(lib::IO::makeUniqueFilePrefix() + file);
	s << latexCode;
	lib::IO::post() << "summaryInput '" << std::string(s) << "'\n";
}

void summaryInput(const std::string &filename) {
	lib::IO::post() << "summaryInput '" << filename << "'\n";
}

void disableInvokeMake() {
	lib::IO::post() << "disableInvokeMake\n";
}

void enableInvokeMake() {
	lib::IO::post() << "enableInvokeMake\n";
}

void disableCompileSummary() {
	lib::IO::post() << "disableCompileSummary\n";
}

void enableCompileSummary() {
	lib::IO::post() << "enableCompileSummary\n";
}

} // namespace mod::post
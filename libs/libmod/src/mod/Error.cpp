#include "Error.hpp"

#include <boost/lexical_cast.hpp>

#include <cxxabi.h>
#include <execinfo.h>

#include <cstdlib>
#include <ostream>
#include <vector>

namespace mod {

Stacktrace::Stacktrace(unsigned int frameLimit, unsigned int numSkip) {
	numSkip++; // skip this
	frameLimit += numSkip;
	std::vector<void*> frames(frameLimit, nullptr);
	int numFrames = backtrace(&frames[0], frames.size());
	char **symbols = backtrace_symbols(&frames[0], numFrames);
	if(!symbols) {
		raw.push_back("Could not capture stack trace.");
		return;
	}
	for(int i = numSkip; i < numFrames; i++)
		raw.emplace_back(symbols[i]);
	std::free(symbols);
	for(const auto &p : raw) {
		unsigned int fStart = 0;
		unsigned int fEnd = 0;
		for(fStart = 0; fStart < p.size() && p[fStart] != '('; fStart++);
		fStart++;
		for(fEnd = fStart + 1; fEnd < p.size() && p[fEnd] != '+'; fEnd++);
		if(fStart >= p.size() || fEnd >= p.size() || fStart == fEnd) {
			demangled.emplace_back(p);
			continue;
		}
		std::string sym(p, 0, fStart);
		std::string mangled(p, fStart, fEnd - fStart);
		int status;
		char *d = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
		switch(status) {
		case 0: break;
		default:
			sym += "demangling error(";
			sym += boost::lexical_cast<std::string>(status);
			sym += ")(";
			sym += mangled;
			sym += ')';
			break;
		}
		if(d) sym += std::string(d);
		std::free(d);
		sym += std::string(p, fEnd);
		demangled.push_back(std::move(sym));
	}
}

void Stacktrace::print(unsigned int frameLimit, std::ostream &s) const {
	if(frameLimit == 0) frameLimit = raw.size();
	for(unsigned int i = 0; i < frameLimit; i++) {
		s << i << ": " << demangled[i] << std::endl;
		if(demangled[i].size() > 200)
			s << std::string(80, '-') << std::endl;
	}
}

const char *Exception::what() const noexcept {
	return text.c_str();
}

void Exception::printStacktrace(unsigned int frameLimit, std::ostream &s) const {
	s << getName() << ", stacktrace:" << std::endl;
	stacktrace.print(0, s);
	s << "(end of stacktrace)" << std::endl;
}

void Exception::append(const std::string &text) {
	this->text += text;
}

const char *FatalError::what() const noexcept {
	std::stringstream ss;
	Exception::printStacktrace(0, ss);
	ss << getName() << ": " << text;
	ss << "\n(Do _not_ try to recover from this exception!)\n";
	whatString = ss.str();
	return whatString.c_str();
}

void fatal(std::string function, std::string file, std::size_t line) {
	std::string text;
	text += "Aborting from " + function + "\n\t" + file + ":" + boost::lexical_cast<std::string>(line) + "\n";
	text += "Fatal error, please report it including this complete error message and stacktrace information.\n";
	throw FatalError(std::move(text), 1);
}

} // namespace mod
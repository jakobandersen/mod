#include "IO.hpp"

#include <boost/lexical_cast.hpp>

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>

namespace mod::lib::IO {
namespace {

const std::string prefix = "out/";

struct PostStream {
	PostStream() {
		std::string postFile = prefix + "post.sh";
		s.open(postFile.c_str());
		enabled = s.is_open();
	}

	~PostStream() {}

	void resetStream() {
		std::string postFile = prefix + "post.sh";
		s.close();
		s.open(postFile.c_str());
		enabled = s.is_open();
	}

	std::ofstream &getStream() {
		if(enabled) return s;
		std::cerr << "ERROR: can not write to '" << prefix + "post.sh'" << std::endl;
		std::cerr << "Does '" << prefix << "' exist?" << std::endl;
		std::exit(1);
	}

private:
	bool enabled;
	std::ofstream s;
public:
	bool dynamicallyEnabled = true;
};

PostStream postStream;

} // namespace

std::string makeUniqueFilePrefix() {
	static int count = 0;

	std::string strCount;
	int k = 4;
	int diff = std::max(0., k - log10(count + 1) - 1);
	if(count == 0) diff--;
	for(int i = 0; i < diff; i++) strCount += '0';
	strCount += boost::lexical_cast<std::string>(count);
	strCount += '_';
	count++;
	return prefix + strCount;
}

std::string escapeForLatex(const std::string &str) {
	std::string res;
	res.reserve(str.size());
	for(char c: str) {
		switch(c) {
		case '#':
		case '_':
		case '{':
		case '}':
			res += "\\";
			res += c;
			break;
		case '*':
			res += "\\ensuremath{*}";
			break;
		default:
			res += c;
		}
	}
	return res;
}

std::string asLatexMath(const std::string &str) {
	std::string res = "$\\mathrm{";
	for(char c: str) {
		switch(c) {
		case ' ':
		case '#':
			res += "\\";
			res += c;
			break;
		default:
			res += c;
		}
	}
	res += "}$";
	return res;
}

std::ostream &nullStream() {
	// https://stackoverflow.com/questions/11826554/standard-no-op-output-stream/11826787
	struct NullBuffer : public std::streambuf {
		int overflow(int c) { return c; }
	};
	struct NullStream : public std::ostream {
		NullStream() : std::ostream(&buffer) {}

	private:
		NullBuffer buffer;
	};
	static NullStream s;
	return s;
}

std::ostream &post() {
	if(!postStream.dynamicallyEnabled) return nullStream();
	return postStream.getStream();
}

void postDisable() {
	postStream.dynamicallyEnabled = false;
}

void postEnable() {
	postStream.dynamicallyEnabled = true;
}

void postReopenCommandFile() {
	postStream.resetStream();
}

std::ostream &Logger::indent(int offset) const {
	assert(indentLevel + offset >= 0);
	return s << std::string((indentLevel + offset) * 2, ' ');
}

std::ostream &Logger::sep(char c) const {
	assert(indentLevel >= 0);
	return indent() << std::string(std::max(10, 80 - indentLevel * 2), c) << '\n';
}

} // namespace mod::lib::IO

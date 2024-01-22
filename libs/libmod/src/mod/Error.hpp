#ifndef MOD_ERROR_HPP
#define MOD_ERROR_HPP

#include <mod/BuildConfig.hpp>

#include <iosfwd>
#include <string>
#include <vector>

// rst: This file contains the functionality used for reporting errors.
// rst: In the description of the exceptions we use the standard terminology (see e.g., `Exception Safety <http://en.wikipedia.org/wiki/Exception_safety>`_).
// rst: If nothing else is specified a thrown exception from MØD provides no exception safety.
// rst: Any exception thrown from MØD or any dependencies are intended to derive from `std::exception`,
// rst: and any exception from MØD derives from :class:`Exception`.
// rst: The exceptions are in general only thrown from the outermost interface, and not from within the ``mod::lib`` namespace.

namespace mod {

// rst-class: Stacktrace
// rst:
// rst:		A class encapsulating the current stacktrace at construction time.
// rst:		Stacktraces may not be supported on all platforms.
// rst-class-start:
struct MOD_DECL Stacktrace {
	// rst: .. function:: Stacktrace(unsigned int frameLimit, unsigned int numSkip)
	// rst:
	// rst:		Capture a stacktrace with at most `frameLimit` frames and without the first `numSkip` frames.
	// rst:		The frame from the constructor itself is always skipped.
	Stacktrace(unsigned int frameLimit, unsigned int numSkip);
	// rst: .. function:: void print(unsigned int frameLimit, std::ostream &s) const
	// rst: 
	// rst:		Print at most the first `frameLimit` frames from the capture stacktrace. Use 0 to print all available frames.
	// rst:		Symbol names will be demangled if possible.
	void print(unsigned int frameLimit, std::ostream &s) const;
private:
	std::vector<std::string> raw;
	std::vector<std::string> demangled;
};
// rst-class-end:


// rst-class: Exception : public std::exception
// rst:
// rst:		The base class of all MØD exceptions.
// rst-class-start:
struct BOOST_SYMBOL_VISIBLE Exception : public std::exception {
protected:
	Exception(std::string &&text, unsigned int numSkip, unsigned int frameLimit)
			: text(text), stacktrace(frameLimit, numSkip) {}
	Exception(std::string &&text, unsigned int numSkip) : Exception(std::move(text), numSkip, 20) {}
	Exception(std::string &&text) : Exception(std::move(text), 0) {}
public:
	// rst: .. function:: virtual std::string getName() const = 0
	// rst:
	// rst:		:returns: the name of the exception.
	virtual std::string getName() const = 0;
	// rst: .. function:: virtual const char *what() const noexcept
	// rst: 
	// rst:		:returns: the description for the exception.
	virtual const char *what() const noexcept;
	// rst: .. function:: void printStacktrace(unsigned int frameLimit, std::ostream &s) const
	// rst:
	// rst:		Print the stacktrace captured when the exception was constructed. See :func:`Stacktrace::print`.
	void printStacktrace(unsigned int frameLimit, std::ostream &s) const;
	// rst: .. function:: void append(const std::string &text)
	// rst:
	// rst:		Append text to the exception message.
	void append(const std::string &text);
protected:
	std::string text;
	Stacktrace stacktrace;
	mutable std::string whatString; // is set by the what function
};
// rst-class-end:

// rst-class: FatalError : public Exception
// rst:
// rst:		When thrown there is no exception safety. It is not safe to continue after catching it.
// rst:
// rst-class-start:
struct BOOST_SYMBOL_VISIBLE FatalError : public Exception {
	FatalError(std::string &&text) : Exception(std::move(text)) {}
	FatalError(std::string &&text, unsigned int numSkip) : Exception(std::move(text), numSkip) {}

	std::string getName() const {
		return "MØD FatalError";
	}
	const char *what() const noexcept;
};
// rst-class-end:

// rst-class: InputError : public Exception
// rst:
// rst:		When thrown there is strong exception safety.
// rst:		This exception is thrown when bad data has been provided to a loading function.
// rst:
// rst-class-start:
struct BOOST_SYMBOL_VISIBLE InputError : public Exception {
	InputError(std::string &&text) : Exception(std::move(text)) {}

	std::string getName() const {
		return "MØD InputError";
	}
};
// rst-class-end:

// rst-class: LogicError : public Exception
// rst:
// rst:		When thrown there is strong exception safety.
// rst:		This exception is thrown when a pre-condition of a function is violated.
// rst:
// rst-class-start:
struct BOOST_SYMBOL_VISIBLE LogicError : public Exception {
	LogicError(std::string &&text) : Exception(std::move(text)) {}

	std::string getName() const {
		return "MØD LogicError";
	}
};
// rst-class-end:

// rst-class: TermParsingError : public Exception
// rst:
// rst:		When thrown there is at least basic exception safety.
// rst:		This exception is thrown if :cpp:any:`LabelType::Term` is used and
// rst:		parsing of a string into a first-order term fails.
// rst:
// rst-class-start:
struct BOOST_SYMBOL_VISIBLE TermParsingError : public Exception {
	TermParsingError(std::string &&text) : Exception(std::move(text)) {}

	std::string getName() const {
		return "MØD TermParsingError";
	}
};
// rst-class-end:

// rst-class: StereoDeductionError : public Exception
// rst:
// rst:		When thrown there is at least basic exception safety.
// rst:		This exception is thrown if stereo data is requested and
// rst:		deduction failed.
// rst:
// rst-class-start:
struct BOOST_SYMBOL_VISIBLE StereoDeductionError : public Exception {
	StereoDeductionError(std::string &&text) : Exception(std::move(text)) {}

	std::string getName() const {
		return "MØD StereoDeductionError";
	}
};
// rst-class-end:

MOD_DECL
void fatal(std::string function, std::string file,
           std::size_t line) __attribute__((__noreturn__)); // TODO: change to C++11 syntax at some point
#define MOD_ABORT mod::fatal(__func__, __FILE__, __LINE__)

} // namespace mod

#endif // MOD_ERROR_HPP
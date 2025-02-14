#ifndef MOD_LIB_GRAPH_PROP_TERM_HPP
#define MOD_LIB_GRAPH_PROP_TERM_HPP

#include <mod/lib/Graph/Properties/Property.hpp>
#include <mod/lib/Term/WAM.hpp>

#include <optional>
#include <string>

namespace mod::lib::graph {
struct PropString;

struct PropTerm : Prop<PropTerm, std::size_t, std::size_t> {
	using Base = Prop<PropTerm, std::size_t, std::size_t>;
public:
	PropTerm(const GraphType &g, const PropString &pString, const StringStore &stringStore); // parse-construct
	bool getHasVariables() const;
	const std::string &getParsingError() const; // requires !isValid
	friend bool isValid(const PropTerm &p);
	friend const lib::Term::Wam &getMachine(const PropTerm &p);
private:
	std::optional<std::string> parsingError;
	lib::Term::Wam machine;
	bool hasVariables = false;
};

} // namespace mod::lib::graph

#endif // MOD_LIB_GRAPH_PROP_TERM_HPP
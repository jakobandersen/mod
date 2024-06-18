#ifndef MOD_LIB_GRAPHMORPHISM_IO_WRITECONSTRAINTS_HPP
#define MOD_LIB_GRAPHMORPHISM_IO_WRITECONSTRAINTS_HPP

#include <mod/Error.hpp>
#include <mod/lib/GraphMorphism/Constraints/AllVisitor.hpp>
#include <mod/lib/IO/IO.hpp>

namespace mod::lib::GraphMorphism::Write {
namespace detail {

template<typename Graph>
struct ConstraintsTexPrintVisitor : Constraints::AllVisitor<Graph> {
	ConstraintsTexPrintVisitor(std::ostream &s, const Graph &g) : s(s), g(g) {}

	void printOp(Constraints::Operator op) {
		using Constraints::Operator;
		switch(op) {
		case Operator::EQ:
			s << "=";
			break;
		case Operator::LT:
			s << "<";
			break;
		case Operator::GT:
			s << ">";
			break;
		case Operator::LEQ:
			s << "\\leq";
			break;
		case Operator::GEQ:
			s << "\\geq";
			break;
		}
	}

	virtual void operator()(const Constraints::VertexAdjacency<Graph> &c) override {
		s << "\\begin{align*}\n";
		s << "&|\\{e \\in \\mathrm{outEdges}(" << get(boost::vertex_index_t(), g, c.vConstrained) << ") ";
		if(!c.vertexLabels.empty() || !c.edgeLabels.empty()) s << " \\mid \\\\\n";
		if(!c.vertexLabels.empty()) {
			s << "&\\quad \\mathrm{label}(\\mathrm{target}(e)) \\in \\{";
			bool first = true;
			for(const std::string &str: c.vertexLabels) {
				if(!first) s << ", ";
				first = false;
				s << " \\text{`\\texttt{" << IO::escapeForLatex(str) << "}'}";
			}
			s << " \\}\\\\\n";
		}
		if(!c.vertexLabels.empty() && !c.edgeLabels.empty()) s << "&\\quad \\wedge\\\\\n";
		if(!c.edgeLabels.empty()) {
			s << "&\\quad \\mathrm{label}(e) \\in \\{";
			bool first = true;
			for(const std::string &str: c.edgeLabels) {
				if(!first) s << ", ";
				first = false;
				s << " \\text{`\\texttt{" << IO::escapeForLatex(str) << "}'}";
			}
			s << " \\}\\\\\n";
		}
		if(!c.vertexLabels.empty() || !c.edgeLabels.empty()) s << "&";
		s << "\\}| ";
		printOp(c.op);
		s << " " << c.count << "\n\\end{align*}\n";
	}

	virtual void operator()(const Constraints::LabelAny<Graph> &c) override {
		s << "$\\texttt{" << IO::escapeForLatex(c.label) << "} \\in \\{";
		bool first = true;
		for(const std::string &str: c.labels) {
			if(!first) s << ", ";
			first = false;
			s << " \\text{`\\texttt{" << IO::escapeForLatex(str) << "}'}";
		}
		s << " \\}$\n";
	}

	virtual void operator()(const Constraints::ShortestPath<Graph> &c) override {
		s << "$\\mathrm{shortestPath}(" << get(boost::vertex_index_t(), g, c.vSrc) << ", "
		  << get(boost::vertex_index_t(), g, c.vTar) << ") ";
		printOp(c.op);
		s << " " << c.length << "$\n";
	}
private:
	std::ostream &s;
	const Graph &g;
};

template<typename Graph>
struct ConstraintsGMLPrintVisitor : Constraints::AllVisitor<Graph> {
	ConstraintsGMLPrintVisitor(std::ostream &s, const Graph &g, const std::string &prefix)
			: s(s), g(g), prefix(prefix) {}

	void printOp(Constraints::Operator op) {
		using Constraints::Operator;
		switch(op) {
		case Operator::EQ:
			s << "=";
			break;
		case Operator::LT:
			s << "<";
			break;
		case Operator::GT:
			s << ">";
			break;
		case Operator::LEQ:
			s << "<=";
			break;
		case Operator::GEQ:
			s << ">=";
			break;
		}
	}

	virtual void operator()(const Constraints::VertexAdjacency<Graph> &c) {
		s << prefix << "constrainAdj [\n";
		s << prefix << "	id " << get(boost::vertex_index_t(), g, c.vConstrained) << "\n";
		s << prefix << "	op \"";
		printOp(c.op);
		s << "\"\n";
		s << prefix << "	count " << c.count << "\n";
		s << prefix << "	nodeLabels [";
		for(const auto &str: c.vertexLabels) s << " label \"" << str << "\"";
		s << " ]\n";
		s << prefix << "	edgeLabels [";
		for(const auto &str: c.edgeLabels) s << " label \"" << str << "\"";
		s << " ]\n";
		s << prefix << "]\n";
	}

	virtual void operator()(const Constraints::LabelAny<Graph> &c) {
		s << prefix << "constrainLabelAny [\n";
		s << prefix << "	label \"" << c.label << "\"\n";
		s << prefix << "	labels [";
		for(const auto &str: c.labels) s << " label \"" << str << "\"";
		s << " ]\n";
		s << prefix << "]\n";
	}

	virtual void operator()(const Constraints::ShortestPath<Graph> &c) {
		s << prefix << "constrainShortestPath [\n";
		s << prefix << "	source " << get(boost::vertex_index_t(), g, c.vSrc)
		  << " target " << get(boost::vertex_index_t(), g, c.vTar) << "\n";
		s << prefix << "	op \"";
		printOp(c.op);
		s << "\" length " << c.length << "\n";
		s << prefix << "]\n";
	}
private:
	std::ostream &s;
	const Graph &g;
	const std::string &prefix;
};

} // namespace detail

template<typename Graph>
void texConstraint(std::ostream &s, const Graph &g, const Constraints::Constraint<Graph> &c) {
	auto vis = detail::ConstraintsTexPrintVisitor(s, g);
	c.accept(vis);
}

template<typename Graph>
void gmlConstraint(std::ostream &s, const Graph &g, const std::string &prefix,
                   const Constraints::Constraint<Graph> &c) {
	auto vis = detail::ConstraintsGMLPrintVisitor(s, g, prefix);
	c.accept(vis);
}

} // namespace mod::lib::GraphMorphism::Write

#endif // MOD_LIB_GRAPHMORPHISM_IO_WRITECONSTRAINTS_HPP
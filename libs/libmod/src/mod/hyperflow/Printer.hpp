#ifndef MOD_HYPERFLOW_PRINTER_HPP
#define MOD_HYPERFLOW_PRINTER_HPP

#include <mod/BuildConfig.hpp>
#include <mod/dg/ForwardDecl.hpp>
#include <mod/dg/GraphInterface.hpp>
#include <mod/hyperflow/ForwardDecl.hpp>

#include <functional>
#include <memory>
#include <string>

namespace mod::hyperflow {

// rst-class: hyperflow::Printer
// rst:
// rst:		An object of this class represents options for printing flow solutions.
// rst:
// rst-class-start:
struct MOD_DECL Printer {
	Printer();
	~Printer();
	// rst: .. function:: dg::Printer &getDGPrinter()
	// rst:               const dg::Printer &getDGPrinter() const
	// rst:
	// rst:		:returns: a reference to the :class:`dg::Printer` used when printing flow solutions.
	dg::Printer &getDGPrinter();
	const dg::Printer &getDGPrinter() const;
	lib::HyperFlow::Write::Printer &getPrinter() const;
	// rst: .. function: void setWithFlowLabels(bool value)
	// rst:              bool getWithFlowLabels() const
	// rst:
	// rst:		Control whether a flow label is appended to each hyperedge label (see also :func:`Printer::pushEdgeLabel`).
	void setWithFlowLabels(bool value);
	bool getWithFlowLabels() const;
	// rst: .. function:: void pushInEdgeLabel(std::function<std::string(dg::DG::Vertex)> f)
	// rst:
	// rst:		Add another function for in-edge labelling.
	// rst:		The result of this function is appended to each label using the edge label separator
	// rst:		of the underlying :class:`dg::Printer`.
	void pushInEdgeLabel(std::function<std::string(dg::DG::Vertex)> f);
	// rst: .. function:: void popInEdgeLabel()
	// rst:
	// rst:		Remove the last pushed in-edge labelling function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popInEdgeLabel();
	// rst: .. function:: void pushOutEdgeLabel(std::function<std::string(dg::DG::Vertex)> f)
	// rst:
	// rst:		Add another function for out-edge labelling.
	// rst:		The result of this function is appended to each label using the edge label separator
	// rst:		of the underlying :class:`dg::Printer`.
	void pushOutEdgeLabel(std::function<std::string(dg::DG::Vertex)> f);
	// rst: .. function:: void popInOutEdgeLabel()
	// rst:
	// rst:		Remove the last pushed out-edge labelling function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popOutEdgeLabel();
	// rst: .. function:: void setPrintUnfiltered(bool value)
	// rst:               bool getPrintUnfiltered() const
	// rst:
	// rst:		Control whether the unfiltered version of a solution is printed.
	void setPrintUnfiltered(bool value);
	bool getPrintUnfiltered() const;
	// rst: .. function:: void setPrintFiltered(bool value)
	// rst:               bool getPrintFiltered() const
	// rst:
	// rst:		Control whether the filtered version of a solution is printed.
	void setPrintFiltered(bool value);
	bool getPrintFiltered() const;
	// rst: .. function:: void setUnfilteredFlowColour(std::string value)
	// rst:               std::string getUnfilteredFlowColour() const
	// rst:
	// rst:		When printing the unfiltered version, use this colour on vertices and edges with non-zero flow.
	// rst:		Use an empty string to not colour them.
	void setUnfilteredFlowColour(std::string value);
	std::string getUnfilteredFlowColour() const;
private:
	std::unique_ptr<dg::Printer> dgPrinter;
	std::unique_ptr<lib::HyperFlow::Write::Printer> printer;
};
// rst-class-end:

} // namespace mod::hyperflow

#endif // MOD_HYPERFLOW_PRINTER_HPP
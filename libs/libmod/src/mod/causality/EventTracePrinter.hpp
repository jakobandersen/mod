#ifndef MOD_CAUSALITY_EVENTTRACEPRINTER_HPP
#define MOD_CAUSALITY_EVENTTRACEPRINTER_HPP

#include <mod/BuildConfig.hpp>
#include <mod/causality/ForwardDecl.hpp>
#include <mod/dg/DG.hpp>

#include <functional>
#include <memory>

namespace mod::causality {

// rst-class: causality::EventTracePrinter
// rst:
// rst:		.. versionadded:: 1.1
// rst:
// rst:		.. todo:: check version added before release
// rst:
// rst:		This class is used to configure how event traces are visualised,
// rst:		e.g., which vertices/graphs should be shown, colours, and axis configuration.
// rst:
// rst:		The plot is rendered with the `PGFPlots package <https://mirrors.ctan.org/graphics/pgf/contrib/pgfplots/doc/pgfplots.pdf>`__ similarly to the following pseudo Latex code.
// rst:
// rst:		.. code-block:: latex
// rst:
// rst:			\begin{tikzpicture}
// rst:			\begin{axis}[
// rst:				% base options
// rst:				% options from callback
// rst:			]
// rst:			% result of pre-content callback
// rst:			% for each vertex in the trace:
// rst:			%    if the vertex is visible, according the callback
// rst:			        \addplot+[
// rst:			           % base vertex options
// rst:			           % options from vertex options callback
// rst:			        ] table {
// rst:			           % filename of file with data for that vertex
// rst:			        }
// rst:			        \addlegendentry{
// rst:			           % name of the graph associated with the vertex, escaped to be in math mode
// rst:			        }
// rst:			% result of post-content callback
// rst:			\end{axis}
// rst:			\end{tikzpicture}
// rst:
// rst-class-start:
struct MOD_DECL EventTracePrinter {
	EventTracePrinter();
	EventTracePrinter(const EventTracePrinter &other);
	EventTracePrinter &operator=(const EventTracePrinter &other);
	~EventTracePrinter();
public:
	const lib::Causality::Write::EventTracePrinter &getPrinter() const;
public:
	// rst: .. function:: void setMaxPointsPerVertex(int value)
	// rst:               int getMaxPointsPerVertex() const
	// rst:
	// rst:		Control how many points are rendered for each different vertex represented in the trace.
	// rst:		Data is then visualised at this amount of regular intervals on the time axis.
	// rst:		It defaults to 500.
	void setMaxPointsPerVertex(int value);
	int getMaxPointsPerVertex() const;
	// rst: .. function:: void setLogTime(bool value)
	// rst:               bool getLogTime() const
	// rst:
	// rst:		Control if the time axis should be logarithmic or not.
	// rst:		It defaults to `false`.
	void setLogTime(bool value);
	bool getLogTime() const;
	// rst: .. function:: void setLogCount(bool value)
	// rst:               bool getLogCount() const
	// rst:
	// rst:		Control if the count axis should be logarithmic or not.
	// rst:		It defaults to `false`.
	void setLogCount(bool value);
	bool getLogCount() const;
	// rst: .. function:: void pushOptions(std::function<std::string(std::shared_ptr<dg::DG>)> f)
	// rst:
	// rst:		Add another function or constant that adds options for the axis environment.
	// rst:		All options function results are applied.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushOptions(std::function<std::string(std::shared_ptr<dg::DG>)> f);
	// rst: .. function:: void popOptions()
	// rst:
	// rst:		Remove the last pushed options function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popOptions();
	// rst: .. function:: void pushVertexVisible(std::function<bool(dg::DG::Vertex)> f)
	// rst:
	// rst:		Add another function controlling the visibility of vertices.
	// rst:		All visibility functions must return `true` for the plot of a vertex to be visible.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushVertexVisible(std::function<bool(dg::DG::Vertex)> f);
	// rst: .. function:: void popVertexVisible()
	// rst:
	// rst:		Remove the last pushed vertex visibility function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popVertexVisible();
	// rst: .. function:: void pushVertexOptions(std::function<std::string(dg::DG::Vertex)> f)
	// rst:
	// rst:		Add another function or constant setting options for the plot of each vertex.
	// rst:		All options function results are applied.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void pushVertexOptions(std::function<std::string(dg::DG::Vertex)> f);
	// rst: .. function:: void popVertexOptions()
	// rst:
	// rst:		Remove the last pushed vertex options function.
	// rst:
	// rst:		:throws: :class:`LogicError` if no callback is left to pop.
	void popVertexOptions();
	// rst: .. function:: void setPreContent(std::function<std::string(std::shared_ptr<dg::DG>)> f)
	// rst:               void setPostContent(std::function<std::string(std::shared_ptr<dg::DG>)> f)
	// rst:
	// rst:		Set a callback for adding additional content to the plot,
	// rst:		either before or after the primary data has been added.
	// rst:
	// rst:		:throws: :class:`LogicError` if `!f`.
	void setPreContent(std::function<std::string(std::shared_ptr<dg::DG>)> f);
	void setPostContent(std::function<std::string(std::shared_ptr<dg::DG>)> f);
private:
	std::unique_ptr<lib::Causality::Write::EventTracePrinter> printer;
};
// rst-class-end:

} // namespace mod::causality

#endif // MOD_CAUSALITY_EVENTTRACEPRINTER_HPP
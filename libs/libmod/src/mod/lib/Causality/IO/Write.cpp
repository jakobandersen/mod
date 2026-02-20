#include "Write.hpp"

#include <mod/Post.hpp>
#include <mod/lib/Causality/Stochsim.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

namespace mod::lib::Causality::Write {

std::string EventTracePrinter::allOptions(const lib::DG::Hyper &dg) const {
	std::string res;
	for(const auto &f: options) {
		res += "\t";
		res += f(dg);
		res += ",\n";
	}
	return res;
}

bool EventTracePrinter::isVertexVisible(lib::DG::HyperVertex v, const lib::DG::Hyper &dg) const {
	for(const auto &f: vertexVisibles)
		if(!f(v, dg))
			return false;
	return true;
}

std::string EventTracePrinter::allVertexOptions(lib::DG::HyperVertex v, const lib::DG::Hyper &dg) const {
	std::string res;
	for(const auto &f: vertexOptions) {
		auto s = f(v, dg);
		if(!s.empty()) {
			res += ", ";
			res += s;
		}
	}
	return res;
}

namespace {

struct BinningData {
	std::vector<std::pair<double, int>> data;
	int bin = -1;
};

EventTraceData makeEventTraceData(const lib::DG::Hyper::GraphType &g, std::vector<BinningData> bData) {
	EventTraceData data;
	for(const auto v: asRange(vertices(g)))
		if(g[v].kind == DG::HyperVertexKind::Vertex)
			data.emplace(v, std::move(bData[get(boost::vertex_index_t(), g, v)].data));
	return data;
}

EventTraceData calculatePlotPoints(const lib::Causality::EventTrace &trace, const EventTracePrinter &p) {
	const auto &initialState = trace.getInitialState();
	const auto &g = trace.dg.getGraph();
	const auto idx = get(boost::vertex_index_t(), g);
	std::vector<BinningData> bData(num_vertices(g));
	// ensure all plots start at 0
	for(const auto v: asRange(vertices(g)))
		if(g[v].kind == DG::HyperVertexKind::Vertex)
			bData[idx[v]].data.emplace_back(0.0, initialState[v]);

	if(trace.getEvents().empty())
		return makeEventTraceData(g, std::move(bData));

	double maxTime = trace.getEvents().back().time;
	double minTime = trace.getEvents().front().time;
	if(p.logTime) {
		maxTime = std::log10(maxTime);
		minTime = std::log10(minTime);
	}
	if(minTime == maxTime) minTime = 0;

	// std::cout << "minTime=" << minTime << ", maxTime=" << maxTime << std::endl;
	const double binFactor = maxTime == 0 ? 0 : p.maxPointsPerMolecule / (maxTime - minTime);
	// std::cout << "binFactor=" << binFactor << std::endl;

	auto update = [&bData, binFactor, idx, minTime, logTime=p.logTime](double time, lib::DG::HyperVertex v, int delta) mutable {
		auto &d = bData[idx[v]];
		const double binTime = logTime ? (time == 0 ? 0 : std::log10(time)) : time;
		const int bin = (binTime - minTime) * binFactor;
		// std::cout << "v=" << v << ", binTime=" << binTime << ", bin=" << bin << ", d.bin=" << d.bin << std::endl;
		if(bin != d.bin) {
			d.data.emplace_back(time, d.data.back().second + delta);
			if(d.bin + 1 == bin) {
				d.bin = bin;
			} else {
				d.bin = bin - 1; // fake the bin to force an extra point
			}
		} else {
			// keep the last point in a bin
			d.data.back().first = time;
			d.data.back().second += delta;
		}
	};

	for(const auto &e: trace.getEvents()) {
		//		std::cout << "Event:\n";
		struct Visitor {
			Visitor(const lib::DG::HyperGraphType &dg, decltype(update) u, double time)
				: dg(dg), u(u), time(time) {}

			void operator()(lib::Causality::EdgeAction a) const {
				const auto e = a.e;
				for(const auto v: asRange(inv_adjacent_vertices(e, dg)))
					u(time, v, -1);
				for(const auto v: asRange(adjacent_vertices(e, dg)))
					u(time, v, 1);
			}

			void operator()(lib::Causality::InputAction event) const {
				u(time, event.v, 1);
			}

			void operator()(lib::Causality::OutputAction event) const {
				u(time, event.v, -1);
			}
		public:
			const lib::DG::HyperGraphType &dg;
			mutable std::remove_const<decltype(update)>::type u;
			double time;
		};
		std::visit(Visitor(g, update, e.time), e.action);
	}
	// ensure all plots ends at last time
	for(const auto v: asRange(vertices(g)))
		if(g[v].kind == DG::HyperVertexKind::Vertex)
			update(trace.getEvents().back().time, v, 0);
	return makeEventTraceData(g, std::move(bData));
}

} // namespace

std::string
makeEventTraceFilename(const lib::Causality::EventTrace &trace, const std::string &prefix,
                       const lib::DG::HyperVertex v) {
	const auto idx = get(boost::vertex_index_t(), trace.dg.getGraph(), v);
	return prefix + "v-" + std::to_string(idx) + ".txt";
}

// Returns the common prefix for all the printed files:
// - <prefix>v-<idx>.txt, for each graph in the data
std::pair<std::string, EventTraceData>
dataEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &p) {
	std::string prefix = IO::makeUniqueFilePrefix();
	auto data = calculatePlotPoints(trace, p);
	for(const auto &[v, vData]: data) {
		post::FileHandle f(makeEventTraceFilename(trace, prefix, v));
		f << "time\tcount\n";
		for(const auto &[time, count]: vData)
			f << time << "\t" << count << "\n";
	}
	return {std::move(prefix), std::move(data)};
}

std::string
texEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &printer,
              const std::string &prefix, EventTraceData data) {
	post::FileHandle s(prefix + "eventTrace.tex");
	s << R"tex(
\begin{tikzpicture}
\begin{axis}[
	width=\textwidth,
	legend style={at={(0.5,-0.1)},anchor=north},
	legend columns=5,
	xlabel=Time,
	ylabel=Count,
	every axis plot/.append style={const plot},
	cycle multi list={
		mark list\nextlist
		exotic
	},)tex" << '\n';
	if(printer.logTime) s << "\txmode=log,\n";
	if(printer.logCount) s << "\tymode=log,\n";
	s << printer.allOptions(trace.dg);
	s << "]\n";
	if(printer.preContent)
		s << printer.preContent(trace.dg) << "\n";
	// sort the data based on the legend label it will get
	std::vector<std::pair<EventTraceData::key_type, EventTraceData::mapped_type>>
			sortedData(std::move_iterator(data.begin()), std::move_iterator(data.end()));
	std::sort(sortedData.begin(), sortedData.end(), [&g = trace.dg.getGraph()](const auto &a, const auto &b) {
		return g[a.first].graph->getName() < g[b.first].graph->getName();
	});
	for(const auto &[v, vData]: sortedData) {
		if(!printer.isVertexVisible(v, trace.dg))
			continue;
		s << "\\addplot+[mark=none";
		auto opts = printer.allVertexOptions(v, trace.dg);
		if(!opts.empty()) s << ", " << opts;
		s << "] table {" << makeEventTraceFilename(trace, prefix, v) << "};\n";
		s << "\\addlegendentry{" << IO::asLatexMath(trace.dg.getGraph()[v].graph->getName()) << "}\n";
	}
	if(printer.postContent)
		s << printer.postContent(trace.dg) << "\n";
	s << R"x(\end{axis})x" << '\n';
	s << R"x(\end{tikzpicture})x" << '\n';
	return s;
}

std::string pdfEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &printer) {
	auto [prefix, data] = dataEventTrace(trace, printer);
	auto texFile = texEventTrace(trace, printer, prefix, std::move(data));
	std::string fileNoExt(texFile.begin(), texFile.end() - 4);
	IO::post() << "compileTikz \"" << fileNoExt << "\" \"" << fileNoExt << "\"" << std::endl;
	return fileNoExt + ".pdf";
}

std::string summaryEventTrace(const lib::Causality::EventTrace &trace, const EventTracePrinter &printer) {
	auto file = pdfEventTrace(trace, printer);
	std::string fileNoExt(file.begin(), file.end() - 4);
	IO::post() << "summarySubsection \"EventTrace\"\n";
	IO::post() << "summaryPDF \"" << fileNoExt << "\"" << std::endl;
	return file;
}

} // namespace mod::lib::Causality::Write
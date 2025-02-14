#include "Misc.hpp"

#include <mod/BuildConfig.hpp>
#include <mod/graph/Graph.hpp>
#include <mod/lib/DG/Hyper.hpp>
#include <mod/lib/Graph/Graph.hpp>
#include <mod/lib/Graph/Properties/Molecule.hpp>
#include <mod/lib/IO/IO.hpp>
#include <mod/lib/IO/Json.hpp>
#include <mod/lib/Random.hpp>
#include <mod/lib/Stereo/GeometryGraph.hpp>
#include <mod/lib/Stereo/IO/Write.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/lexical_cast.hpp>

#include <iomanip>
#include <iostream>
#include <vector>

namespace mod {

std::string version() {
	return MOD_VERSION;
}

void rngReseed(unsigned int seed) {
	lib::Random::getInstance().reseed(seed);
}

double rngUniformReal() {
	return std::uniform_real_distribution<double>(0, 1)(lib::getRng());
}

std::string strFromDump(const std::string &file) {
	boost::iostreams::mapped_file_source ifs(file);
	std::vector<std::uint8_t> data(ifs.begin(), ifs.end());
	std::ostringstream err;
	auto jOpt = lib::IO::readJson(data, err);
	if(!jOpt)
		throw InputError("Error showing dump: " + err.str());
	std::stringstream ss;
	ss << std::setw(3) << *jOpt << "\n";
	return ss.str();
}

void showDump(const std::string &file) {
	std::cout << strFromDump(file) << std::flush;
}

void printGeometryGraph() {
	lib::Stereo::Write::summary(lib::Stereo::getGeometryGraph());
}

} // namespace mod
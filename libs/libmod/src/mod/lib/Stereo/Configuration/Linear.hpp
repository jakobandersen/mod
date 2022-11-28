#ifndef MOD_LIB_STEREO_CONFIGURATION_LINEAR_HPP
#define MOD_LIB_STEREO_CONFIGURATION_LINEAR_HPP

#include <mod/lib/Stereo/Configuration/Configuration.hpp>

namespace mod::lib::Stereo {

struct Linear final : StaticDegree<2> {
	Linear(const GeometryGraph &g, const std::array<EmbeddingEdge, 2> &edges);
	virtual std::unique_ptr<Configuration> cloneFree(const GeometryGraph &g) const override;
	// this should never be called, as the fixation is always free
	virtual std::unique_ptr<Configuration> clone(const GeometryGraph &g, const std::vector<std::size_t> &offsetMap) const override;
public: // checking
	// these should never be called, as the morphismStaticOk should be used first
	virtual bool morphismIso(const Configuration &cCodom, std::vector<std::size_t> &perm) const override;
	virtual bool morphismSpec(const Configuration &cCodom, std::vector<std::size_t> &perm) const override;
public: // IO
	virtual void printCoords(std::ostream &s, const std::vector<std::size_t> &vIds) const override;
private:
	virtual std::pair<std::string, bool> asPrettyStringImpl(std::function<std::size_t(const EmbeddingEdge&)> getNeighbourId) const override;
};

} // namespace mod::lib::Stereo

#endif // MOD_LIB_STEREO_CONFIGURATION_LINEAR_HPP
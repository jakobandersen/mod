#ifndef MOD_LIB_IO_GRAPHWRITEGENERIC_HPP
#define MOD_LIB_IO_GRAPHWRITEGENERIC_HPP

#include <mod/Error.hpp>
#include <mod/lib/Algorithm/Point.hpp>
#include <mod/lib/Chem/MoleculeUtil.hpp>
#include <mod/lib/IO/IO.hpp>

#include <jla_boost/graph/PairToRangeAdaptor.hpp>

#include <boost/lexical_cast.hpp>

namespace {
namespace Loc {
static constexpr unsigned int
/*                      */T = 16,
/*        */T_left = 32, /*     */T_right = 8,
//
/*   */TL = 64, /*                     */TR = 4,
//
/**/L_up = 128, /*                        */R_up = 2,
//
//
L = 256, /*                                   */R = 1,
//
//
/**/L_down = 512, /*                      */R_down = 32768,
//
/*   */BL = 1024, /*                   */BR = 16384,
//
/*        */B_left = 2048, /*   */B_right = 8192,
/*                      */B = 4096;
static constexpr unsigned int
		R_narrow = Loc::R | Loc::R_up | Loc::R_down,
		L_narrow = Loc::L | Loc::L_up | Loc::L_down,
		T_narrow = Loc::T | Loc::T_left | Loc::T_right,
		B_narrow = Loc::B | Loc::B_left | Loc::B_right,
		R_medium = R_narrow | TR | BR,
		L_medium = L_narrow | TL | BL,
		T_medium = T_narrow | TR | TL,
		B_medium = B_narrow | BR | BL,
		R_wide = R_medium | T_right | B_right,
		L_wide = L_medium | T_left | B_left,
		T_wide = T_medium | R_up | L_up,
		B_wide = B_medium | R_down | L_down;
} // namespace Loc
} // namespace

namespace mod::lib::IO::Graph::Write {

template<typename Graph, typename Depict, typename AdvOptions, typename BonusWriter>
void tikz(std::ostream &s, const Options &options, const Graph &g, const Depict &depict,
          const std::string &fileCoords, const AdvOptions &advOptions, BonusWriter bonusWriter,
          const std::string &idPrefix) {
	[[maybe_unused]] const auto printBlocked = [&s](auto b) {
		const auto at = [&b](int i) {
			return bool((b & (1 << i)) != 0);
		};
		s << "% " << at(6) << " " << at(5) << at(4) << at(3) << " " << at(2) << "\n";
		s << "% " << at(7) << "     " << at(1) << "\n";
		s << "% " << at(8) << "     " << at(0) << "\n";
		s << "% " << at(9) << "     " << at(15) << "\n";
		s << "% " << at(10) << " " << at(11) << at(12) << at(13) << " " << at(14) << "\n";
	};

	// set up data
	std::vector<bool> isVisible(num_vertices(g), true);
	std::vector<bool> isSimpleCarbon(num_vertices(g), false);
	std::vector<unsigned int> implicitHydrogenCount(num_vertices(g), 0);
	std::vector<bool> hasBondBlockingAtChargeRight(num_vertices(g), false);
	std::vector<bool> hasBondBlockingAtChargeLeft(num_vertices(g), false);

	const auto localIdx = get(boost::vertex_index_t(), g);

	{ // visibility from adv
		for(const auto v: asRange(vertices(g)))
			isVisible[get(localIdx, v)] = advOptions.isVisible(v);
	}
	{ // stuff not depending on coordinates being available now
		if(options.simpleCarbons) {
			for(const auto v: asRange(vertices(g))) {
				if(depict.getAtomId(v) != AtomIds::Carbon) continue;
				int adjCount = 0;
				for(const auto vAdj: asRange(adjacent_vertices(v, g))) {
					// for now, we allow it even though the edges are not bonds
					const auto adjAtomId = depict.getAtomId(vAdj);
					switch(adjAtomId) {
					case AtomIds::Carbon:
					case AtomIds::Oxygen:
					case AtomIds::Nitrogen:
					case AtomIds::Sulfur:
						++adjCount;
					}
				}
				if(adjCount < 2) continue;
				if(depict.getIsotope(v) != Isotope()) continue;
				if(depict.getCharge(v) != 0) continue;
				if(depict.getRadical(v)) continue;
				isSimpleCarbon[get(boost::vertex_index_t(), g, v)] = true;
			}
		}

		if(options.collapseHydrogens) { // collapse most hydrogen atoms to it's neighbour
			for(const auto v: asRange(vertices(g))) {
				const auto vId = get(localIdx, v);
				if(!isVisible[vId]) continue;
				const auto hasImportantStereo = [&depict](const auto v) {
					return depict.hasImportantStereo(v);
				};
				if(!Chem::isCollapsibleHydrogen(v, g, depict, depict, hasImportantStereo)) continue;
				assert(out_degree(v, g) == 1);
				const auto e = *out_edges(v, g).first;
				const auto vAdj = target(e, g); // TODO: just use adjacent_vertices, we don't need the edge
				const auto vAdjId = get(boost::vertex_index_t(), g, vAdj);
				if(!isVisible[vAdjId]) continue;
				if(advOptions.disallowHydrogenCollapse(v)) continue;
				++implicitHydrogenCount[vAdjId];
				isVisible[vId] = false;
			}
		}
	}

	const auto areAllBlocked = [](unsigned int b) {
		return (b & (Loc::R_down * 2 - 1)) == (Loc::R_down * 2 - 1);
	};
	std::vector<unsigned int> auxLabelBlocked(num_vertices(g), 0);
	// stuff where we absolutely need coordinates right now
	if(!options.withGraphvizCoords && depict.getHasCoordinates()) {
		// see where we can have aux data
		for(const auto v: asRange(vertices(g))) {
			const auto vId = get(localIdx, v);
			if(!isVisible[vId]) continue;
			const auto[x, y] = pointTransform(
					depict.getX(v, !options.collapseHydrogens),
					depict.getY(v, !options.collapseHydrogens),
					options.rotation, options.mirror);
			for(const auto eOut: asRange(out_edges(v, g))) {
				const auto vAdj = target(eOut, g);
				const auto vAdjId = get(localIdx, vAdj);
				if(!isVisible[vAdjId]) continue;
				const auto[xAdj, yAdj] = pointTransform(
						depict.getX(vAdj, !options.collapseHydrogens),
						depict.getY(vAdj, !options.collapseHydrogens),
						options.rotation, options.mirror);
				auto &blocked = auxLabelBlocked[vId];
				constexpr double eps = 0.000001;
				const double xDiff = xAdj - x;
				const double yDiff = yAdj - y;
				double angle = std::atan2(yDiff, xDiff); // [-pi; pi], later shifted to [0; 2*pi]

				const BondType bType = depict.getBondData(eOut);
				// a single bond blocks less than a double/aromatic, and they block less than a triple
				const double f = [&]() {
					switch(bType) {
					case BondType::Single:
						return 8;
					case BondType::Aromatic:
					case BondType::Double:
						return 5;
					case BondType::Invalid:
					case BondType::Triple:
						return 3;
					}
					__builtin_unreachable();
				}();
				// check if a bond blocks a charge, using angle in [-pi/2; 3/2*pi]
				{ // right
					const double shifted_angle = angle < -pi / 2 ? angle + 2 * pi : angle;
					constexpr double a = pi / 16;
					const double lower = a - pi / f + eps;
					const double upper = a + pi / f - eps;
					if(shifted_angle >= lower && shifted_angle <= upper)
						hasBondBlockingAtChargeRight[vId] = true;
				}
				{ // left
					const double shifted_angle = angle < -pi / 2 ? angle + 2 * pi : angle;
					constexpr double a = pi - pi / 16;
					const double lower = a - pi / f + eps;
					const double upper = a + pi / f - eps;
					if(shifted_angle >= lower && shifted_angle <= upper)
						hasBondBlockingAtChargeLeft[vId] = true;
				}
				// now check block where the bond is
				const auto check = [&](int i, double lower, double upper) {
					//					s << "% " << vId << "(" << x << ", " << y << "): " << vAdjId << "(" << xAdj << ", " << yAdj << "), "
					//							<< " a=" << angle << ", a d=" << (angle * 180 / pi)
					//							<< " l=" << lower << ", l d=" << (lower * 180 / pi)
					//							<< " u=" << upper << ", u d=" << (upper * 180 / pi) << "\n";
					if(angle >= lower && angle <= upper) {
						blocked |= (1 << i);
						//						s << "%     blocked=" << i << "\n";
					}
				};
				const auto posToAngle = [](int i) {
					return 2 * pi * i / 16;
				};
				// take the 1st and 4th quadrant first
				for(int i = -4; i < 4; ++i) {
					const int pos = i < 0 ? i + 16 : i;
					check(pos, posToAngle(i) - pi / f + eps, posToAngle(i) + pi / f - eps);
				}
				//				s << "% " << v << ": " << vAdj << ", angle=" << angle << std::endl;
				//				printBlocked(blocked);
				// now shift to get contiguous in the 2nd and 3rd quadrants
				if(angle < 0) angle += 2 * pi; // [0; 2*pi]
				for(int i = 4; i < 12; ++i) {
					check(i, posToAngle(i) - pi / f + eps, posToAngle(i) + pi / f - eps);
				}
				//				s << "% " << v << ": " << vAdj << ", angle=" << angle << std::endl;
				//				printBlocked(blocked);
			}
		}
	} else { // we don't have coordinates, block all
		for(auto &c: auxLabelBlocked) c = ~0L;
	}

	// Tikz code
	// ---------------------------------------------------------------------------
	std::string textModifiersBegin, textModifiersEnd;
	if(options.thick) {
		textModifiersBegin += "\\textbf{";
		textModifiersEnd += "}";
	}
	if(options.withTexttt) {
		textModifiersBegin += "\\texttt{";
		textModifiersEnd += "}";
	}
	s << "\\begin{tikzpicture}[remember picture, scale=\\modGraphScale";
	s << R"XXX(, baseline={([yshift={-0.5ex}]current bounding box)})XXX";
	if(options.thick) s << ", thick";
	// circumvent the strange inherited 'dashed' http://tex.stackexchange.com/questions/115887/pattern-in-addplot-inherits-dashed-option-from-previous-draw
	s << ", solid";
	s << "]\n";
	if(!idPrefix.empty())
		s << "\\renewcommand\\modIdPrefix{" << idPrefix << "}\n";
	s << "\\input{\\modInputPrefix/" << fileCoords << "}\n";

	for(const auto v: asRange(vertices(g))) {
		const auto vId = get(localIdx, v);
		if(!isVisible[vId]) continue;
		const auto atomId = depict.getAtomId(v);

		const auto createDummy = [&s, v, &advOptions, &textModifiersBegin, &textModifiersEnd](
				std::string suffix, bool subscript, bool superscript) {
			// create dummy vertex to make sure the bounding box is large enough
			s << "\\node[modStyleGraphVertex, at=(\\modIdPrefix v-" << advOptions.getOutputId(v) << suffix << ")";
			if(subscript) s << ", text depth=.25ex";
			if(superscript) s << ", text height=2.25ex";
			s << "] {\\phantom{" << textModifiersBegin << "H";
			if(subscript) s << "$_2$";
			if(superscript) s << "$^{12}$";
			s << textModifiersEnd << "}};\n";
		};

		std::string colourString = advOptions.getColour(v);
		if(options.withColour && colourString.empty()) {
			switch(atomId) {
			case AtomIds::Hydrogen:
				colourString = "gray";
				break;
			case AtomIds::Nitrogen:
				colourString = "blue";
				break;
			case AtomIds::Oxygen:
				colourString = "red";
				break;
			case AtomIds::Phosphorus:
				colourString = "orange";
				break;
			case AtomIds::Sulfur:
				colourString = "olive";
				break;
			}
		}
		if(!colourString.empty()) colourString = ", text=" + colourString;

		std::string indexString;
		if(options.withIndex && !advOptions.overwriteWithIndex(v)) {
			indexString = "{\\tiny $\\langle " + advOptions.getShownId(v) + "\\rangle$}";
		}

		s << "\\node[modStyleGraphVertex";
		s << colourString;
		if(isSimpleCarbon[vId]) {
			if(indexString.empty()) s << ", modStyleCarbon";
			else s << ", modStyleCarbonIndex";
		}
		s << ", at=(\\modIdPrefix v-coord-" << advOptions.getOutputId(v) << ")";
		std::string userOpts = advOptions.getOpts(v);
		if(!userOpts.empty())
			s << ", " << userOpts;
		s << "] (\\modIdPrefix v-" << advOptions.getOutputId(v) << ") ";
		auto auxBlocked = auxLabelBlocked[vId];
		if(isSimpleCarbon[vId]) { // Simple Cs
			s << "{" << textModifiersBegin << indexString << textModifiersEnd << "};\n";
			if(!indexString.empty()) createDummy("", false, false);
		} else { // not simple carbon
			const auto isotope = depict.getIsotope(v);
			const auto charge = depict.getCharge(v);
			const auto hCount = implicitHydrogenCount[vId];
			const bool allAuxBlocked = areAllBlocked(auxBlocked);
			const bool hInLabel = !options.collapseHydrogens || allAuxBlocked;
			const bool chargeOnLeft =
					hasBondBlockingAtChargeRight[vId] && !hasBondBlockingAtChargeLeft[vId]; // && isotope == Isotope();

			std::string labelNoAux = lib::IO::escapeForLatex(depict.getVertexLabelNoIsotopeChargeRadical(v));
			std::string isotopeString;
			if(isotope != Isotope()) {
				if(options.raiseIsotopes) isotopeString += "$^{";
				isotopeString += boost::lexical_cast<std::string>(isotope);
				if(options.raiseIsotopes) isotopeString += "}$";
			}
			std::string chargeString;
			if(charge != 0) {
				if(options.raiseCharges) chargeString += "$^{";
				if(charge != 1 && charge != -1) chargeString += boost::lexical_cast<std::string>(std::abs(charge));
				if(charge < 0) chargeString += '-';
				else chargeString += '+';
				if(options.raiseCharges) chargeString += "}$";
			}
			std::string hString;
			if(hCount > 0) {
				hString += "H";
				if(hCount > 1) hString += "$_{" + boost::lexical_cast<std::string>(hCount) + "}$";
			}

			// handle H_2 special
			if(hCount == 1 && charge == 0 && atomId == 1 && isotope == Isotope()) {
				s << "{" << textModifiersBegin;
				s << indexString << "H$_2$";
				s << textModifiersEnd << "};\n";
				createDummy("", true, false);
				continue;
			}
			// handle water special
			if(hCount == 2 && charge == 0 && atomId == 8 && isotope == Isotope() && degree(v, g) == 2) {
				s << "{" << textModifiersBegin;
				s << "H$_2$O" << indexString;
				s << textModifiersEnd << "};\n";
				createDummy("", true, false);
				continue;
			}

			s << "{" << textModifiersBegin;
			s << labelNoAux;
			if(hInLabel) s << hString;
			s << textModifiersEnd << "};\n";

			const bool subscript = hInLabel && hCount > 1;
			const bool superscript = isotope != Isotope();
			if(subscript || !indexString.empty())
				createDummy("", subscript, superscript);
			const bool hasAuxHydrogen = !hInLabel && hCount != 0;

			decltype(auxBlocked) auxHPosition = -1;
			if(hCount < 2) {
				/**/ if((auxBlocked & Loc::R_narrow) == 0) auxHPosition = Loc::R_narrow;
				else if((auxBlocked & Loc::L_narrow) == 0) auxHPosition = Loc::L_narrow;
				else if((auxBlocked & Loc::T_narrow) == 0) auxHPosition = Loc::T_narrow;
				else if((auxBlocked & Loc::B_narrow) == 0) auxHPosition = Loc::B_narrow;
			} else { // the subscript takes extra space
				/**/ if((auxBlocked & Loc::R_narrow) == 0) auxHPosition = Loc::R_narrow;
				else if((auxBlocked & (Loc::L_narrow | Loc::BL)) == 0) auxHPosition = Loc::L_narrow;
				else if((auxBlocked & (Loc::T_narrow | Loc::TR)) == 0) auxHPosition = Loc::T_narrow;
				else if((auxBlocked & Loc::B_narrow) == 0) auxHPosition = Loc::B_narrow;
			}
			const bool isAuxVertical = auxHPosition == Loc::T_narrow || auxHPosition == Loc::B_narrow;
			// if we have something like:
			//              y
			//             /
			//            xH
			// then make it
			//              y
			//             /
			//           Hx
			//
			if(auxHPosition == Loc::R_narrow &&
			   (auxBlocked & Loc::R_wide) != 0 &&
			   (auxBlocked & Loc::L_wide) == 0
					) {
				auxHPosition = Loc::L_narrow;
			}
			// if we have something like:
			//      Y_
			//        \_H
			//          x
			// then make it
			//      Y_
			//        \_
			//          x
			//          H
			if(/**/ auxHPosition == Loc::T_narrow
			        && (auxBlocked & Loc::T_wide) != 0
			        && (auxBlocked & Loc::B_wide) == 0
					) {
				auxHPosition = Loc::B_narrow;
			}
			// forget about all this placement stuff if we don't have any hydrogens
			if(!hasAuxHydrogen) auxHPosition = -1;
			// reserve the aux space
			if(auxHPosition != -1) auxBlocked |= auxHPosition;
			// see if aux should have the charge or isotope as well
			const bool chargeInAux = [&]() {
				if(auxHPosition == -1) return false;
				if(hInLabel) return false;
				if(hCount == 0) return false;
				if(chargeOnLeft && auxHPosition == Loc::L_narrow) return true;
				if(!chargeOnLeft && auxHPosition == Loc::R_narrow) return true;
				return false;
			}();
			const bool isotopeInAux = [&]() {
				if(auxHPosition == -1) return false;
				if(hInLabel) return false;
				if(hCount == 0) return false;
				return auxHPosition == Loc::L_narrow;
			}();

			if(!chargeInAux && charge != 0) {
				s << "\\node[modStyleGraphVertex" << colourString << ", at=(\\modIdPrefix v-"
				  << advOptions.getOutputId(v);
				if(chargeOnLeft) s << ".west), anchor=east";
				else s << ".east), anchor=west";
				s << "] {";
				s << textModifiersBegin;
				s << chargeString;
				s << textModifiersEnd << "};\n";
				if(chargeOnLeft) auxBlocked |= Loc::L & Loc::L_up & Loc::TL;
				else auxBlocked |= Loc::R & Loc::R_up & Loc::TR;
			}
			if(!isotopeInAux && isotope != Isotope()) {
				s << "\\node[modStyleGraphVertex" << colourString << ", at=(\\modIdPrefix v-"
				  << advOptions.getOutputId(v);
				s << ".west), anchor=east";
				s << "] {";
				s << textModifiersBegin;
				s << isotopeString;
				s << textModifiersEnd << "};\n";
				createDummy("", false, true);
				auxBlocked |= Loc::L & Loc::L_up & Loc::TL;
			}

			if(auxHPosition != -1) {
				s << "\\node[modStyleGraphVertex" << colourString << ", at=(\\modIdPrefix v-" << advOptions.getOutputId(v)
				  << ".";
				/**/ if(auxHPosition == Loc::R_narrow) s << "east), anchor=west";
				else if(auxHPosition == Loc::L_narrow) s << "west), anchor=east";
				else if(auxHPosition == Loc::T_narrow) s << "north), anchor=south, yshift=1pt";
				else if(auxHPosition == Loc::B_narrow) s << "south), anchor=north, yshift=-1pt";
				else
					MOD_ABORT;
				s << "] (\\modIdPrefix v-" << advOptions.getOutputId(v) << "-aux) {";
				s << textModifiersBegin;
				if(chargeInAux && chargeOnLeft) s << chargeString;
				std::string output;
				if(!isAuxVertical) output += hString;
				else output += "H";
				if(chargeInAux && !chargeOnLeft) {
					assert(!output.empty());
					// hString may end in $ and chargeString may start with $. Shortcut that.
					if(!chargeString.empty() && chargeString[0] == '$' && output[output.size() - 1] == '$') {
						output.resize(output.size() - 1);
						output += std::string(chargeString.begin() + 1, chargeString.end());
					} else {
						output += chargeString;
					}
				}
				s << output;
				if(isotopeInAux) s << isotopeString;
				s << textModifiersEnd << "};\n";
				createDummy("-aux", hCount > 1, isotopeInAux && isotope != Isotope());
				if(isAuxVertical && hCount > 1) {
					s << "\\node[modStyleGraphVertex" << colourString << ", at=(\\modIdPrefix v-"
					  << advOptions.getOutputId(v) << "-aux.east), anchor=west] {";
					s << textModifiersBegin;
					s << "$_{" << hCount << "}$";
					s << textModifiersEnd << "};\n";
				}
			}
			if(depict.getRadical(v)) {
				std::size_t auxRadicalPosition;
				// first try the x/y-axis
				/**/ if((auxBlocked & Loc::R) == 0 && charge == 0) auxRadicalPosition = 0;
				else if((auxBlocked & Loc::T) == 0) auxRadicalPosition = 4;
				else if((auxBlocked & Loc::L) == 0) auxRadicalPosition = 8;
				else if((auxBlocked & Loc::B) == 0) auxRadicalPosition = 12;
				else {
					for(auxRadicalPosition = 0; auxRadicalPosition < 16; ++auxRadicalPosition) {
						if((auxBlocked & (1 << auxRadicalPosition)) == 0) break;
					}
				}
				// fallback
				if(auxRadicalPosition == 16) auxRadicalPosition = 12;
				// and now process it
				auxBlocked |= (1 << auxRadicalPosition);
				const double atAngle = auxRadicalPosition * 22.5;
				s << "\\node[outer sep=0, inner sep=1, minimum size=0, fill=black, circle, "
				  << "at=(\\modIdPrefix v-" << advOptions.getOutputId(v) << "." << atAngle << "), "
				  << "shift=(" << atAngle << ":3pt)] {};\n";
			} // end if has radical
		} // end if simpleCarbon
		if(options.withRawStereo || options.withPrettyStereo) {
			const std::string strStereo =
					(options.withRawStereo ? advOptions.getRawStereoString(v) : std::string())
					+ (options.withPrettyStereo ? advOptions.getPrettyStereoString(v) : std::string());
			if(!strStereo.empty()) {
				const auto stereoPosition = [&]() {
					for(const unsigned int loc: {0, 8, 4, 12, 1, 15, 7, 9}) {
						if((auxBlocked & (1 << loc)) == 0) return loc;
					}
					for(unsigned int loc = 16; loc > 0; --loc) {
						if((auxBlocked & (1 << (loc - 1))) == 0) return loc - 1;
					}
					return 10u;
				}();
				auxBlocked |= (1u << stereoPosition);
				double atAngle = stereoPosition * 22.5;
				s << "\\node[outer sep=0, inner sep=1, at=(\\modIdPrefix v-" << advOptions.getOutputId(v)
				  << "." << atAngle << "), anchor=";
				[&s, stereoPosition]() -> std::ostream & {
					switch(stereoPosition) {
					case 0:
						return s << "west";
					case 8:
						return s << "east";
					}
					if(stereoPosition < 8) s << "south";
					else s << "north";
					switch(stereoPosition) {
					case 4:
					case 12:
						return s;
					}
					if(stereoPosition > 4 && stereoPosition < 12) return s << " east";
					else return s << " west";
				}();
				s << "] (\\modIdPrefix v-" << advOptions.getOutputId(v) << "-stereoId) ";
				s << "{\\tiny " << textModifiersBegin << strStereo << textModifiersEnd << "};\n";
			}
		} // end if with any kind of stereo

		// index, which were not already handled due to simple carbon
		if(!isSimpleCarbon[vId] && options.withIndex) {
			std::size_t auxIndexPosition;
			// first try the x/y-axis
			/**/ if((auxBlocked & Loc::R_narrow) == 0) auxIndexPosition = 0;
			else if((auxBlocked & Loc::T_narrow) == 0) auxIndexPosition = 4;
			else if((auxBlocked & Loc::L_narrow) == 0) auxIndexPosition = 8;
			else if((auxBlocked & Loc::B_narrow) == 0) auxIndexPosition = 12;
			else {
				if((auxBlocked & ((1 << 15) | 1 | 2)) == 0) auxIndexPosition = 0;
				else if((auxBlocked & ((3 << 14) | 1)) == 0) auxIndexPosition = 0;
				else {
					for(auxIndexPosition = 1; auxIndexPosition < 15; ++auxIndexPosition) {
						if((auxBlocked & (7 << (auxIndexPosition - 1))) == 0) break;
					}
				}
			}
			// fallback
			if(auxIndexPosition == 16) auxIndexPosition = 10;
			// and then process it
			auxBlocked |= (1 << auxIndexPosition);
			const double atAngle = auxIndexPosition * 22.5;
			s << "\\node[outer sep=0, inner sep=1" << colourString
			  << ", at=(\\modIdPrefix v-" << advOptions.getOutputId(v)
			  << "." << atAngle << "), anchor=" << (atAngle + 180)
			  << "] (\\modIdPrefix v-" << advOptions.getOutputId(v) << "-auxId) {";
			s << textModifiersBegin;
			s << indexString;
			s << textModifiersEnd << "};\n";
			createDummy("-auxId", true, false);
		} // end if withIndex
	} // foreach vertex

	for(const auto e: asRange(edges(g))) {
		const auto vSrc = source(e, g);
		const auto vTar = target(e, g);
		const auto vSrcId = get(localIdx, vSrc);
		const auto vTarId = get(localIdx, vTar);
		if(!isVisible[vSrcId]) continue;
		if(!isVisible[vTarId]) continue;
		std::string colourString = advOptions.getColour(e);
		if(!colourString.empty()) colourString += ", text=" + colourString;
		BondType bType = depict.getBondData(e);
		std::string drawCommand = "\\modDraw";
		if(options.edgesAsBonds) {
			switch(bType) {
			case BondType::Invalid:
				drawCommand += "GraphEdge";
				break;
			case BondType::Single:
				drawCommand += "SingleBond";
				break;
			case BondType::Aromatic:
				drawCommand += "AromaticBond";
				break;
			case BondType::Double:
				drawCommand += "DoubleBond";
				break;
			case BondType::Triple:
				drawCommand += "TripleBond";
				break;
			}
		} else { // !edgesAsBonds
			drawCommand += "GraphEdge";
		}
		EdgeFake3DType fake3Dtype = advOptions.getEdgeFake3DType(e, !options.collapseHydrogens);
		switch(fake3Dtype) {
		case EdgeFake3DType::None:
			break;
		case EdgeFake3DType::WedgeSL:
			drawCommand += "WedgeSL";
			break;
		case EdgeFake3DType::WedgeLS:
			drawCommand += "WedgeLS";
			break;
		case EdgeFake3DType::HashSL:
			drawCommand += "HashSL";
			break;
		case EdgeFake3DType::HashLS:
			drawCommand += "HashLS";
			break;
		}
		int fromOffset = 1, toOffset = 1;
		if(!options.withIndex) {
			if(isSimpleCarbon[vSrcId]) fromOffset = 0;
			if(isSimpleCarbon[vTarId]) toOffset = 0;
		}
		s << drawCommand << "{" << advOptions.getOutputId(vSrc) << "}{" << advOptions.getOutputId(vTar) << "}{"
		  << fromOffset << "}{" << toOffset << "}";
		s << "{" << colourString << "}";
		// find the label text
		const std::string label = (!options.edgesAsBonds || bType == BondType::Invalid)
		                          ? lib::IO::escapeForLatex(depict.getEdgeLabel(e)) : std::string();
		const std::string rawStereo = options.withRawStereo
		                              ? "{\\tiny " + advOptions.getStereoString(e) + "}" : std::string();
		const std::string extraAnnotation = advOptions.getEdgeAnnotation(e);
		s << "{";
		if(!label.empty() || !rawStereo.empty()) {
			s << "node[auto]{" << textModifiersBegin << label;
			if(!label.empty() && !rawStereo.empty())
				s << ", ";
			s << rawStereo << textModifiersEnd << "}";
		}
		if((!label.empty() || !rawStereo.empty()) && !extraAnnotation.empty())
			s << " ";
		s << extraAnnotation;
		s << "}";
		s << "\n";
	}
	bonusWriter(s);
	s << "\\end{tikzpicture}\n";
}

template<typename Graph, typename Depict>
struct DefaultAdvancedOptions {
	using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
	using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
public:
	DefaultAdvancedOptions(const Graph &g, const Depict &depict) : g(g), depict(depict) {}

	template<typename VE>
	std::string getColour(VE) const {
		return "";
	}

	bool isVisible(Vertex) const {
		return true;
	}

	std::string getShownId(Vertex v) const {
		return boost::lexical_cast<std::string>(get(boost::vertex_index_t(), g, v));
	}

	bool overwriteWithIndex(Vertex) const {
		return false;
	}

	EdgeFake3DType getEdgeFake3DType(Edge e, bool withHydrogen) const {
		return depict.getEdgeFake3DType(e, withHydrogen);
	}

	std::string getEdgeAnnotation(Edge) const {
		return "";
	}

	std::string getRawStereoString(Vertex v) const {
		return depict.getRawStereoString(v);
	}

	std::string getPrettyStereoString(Vertex v) const {
		return depict.getPrettyStereoString(v);
	}

	std::string getStereoString(Edge e) const {
		return depict.getStereoString(e);
	}

	std::string getOpts(Vertex v) const {
		return std::string();
	}

	bool disallowHydrogenCollapse(Vertex) const {
		return false;
	}

	auto getOutputId(Vertex v) const {
		return get(boost::vertex_index_t(), g, v);
	}
private:
	const Graph &g;
	const Depict &depict;
};

template<typename Graph, typename Depict>
void tikz(std::ostream &s, const Options &options, const Graph &g, const Depict &depict, const std::string &fileCoords,
          bool asInline, const std::string &idPrefix) {
	DefaultAdvancedOptions<Graph, Depict> adv(g, depict);
	tikz(s, options, g, depict, fileCoords, adv, [](std::ostream &s) {}, idPrefix);
}

} // namespace mod::lib::IO::Graph::Write

#endif // MOD_LIB_IO_GRAPHWRITEGENERIC_HPP
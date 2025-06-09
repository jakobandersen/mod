#include <mod/lib/StringStore.hpp>
#include <mod/lib/Term/WAM.hpp>
#include <mod/lib/Test.hpp>

#include <array>
#include <iomanip>
#include <iostream>

#undef NDEBUG

#include <cassert>

using namespace mod::lib::Term;

Address makeRef(Wam &m) {
	return m.putRefPtr();
}

Address makeStr(Wam &m) {
	const auto aid = mod::lib::StringStore_getIndex("aRef0");
	const auto bid = mod::lib::StringStore_getIndex("bRef1");
	const auto cid = mod::lib::StringStore_getIndex("c");
	const auto iid = mod::lib::StringStore_getIndex("iInline");
	const auto fid = mod::lib::StringStore_getIndex("f");
	const auto aa = m.putStructure(aid, 0);
	const auto ab = m.putStructure(bid, 1);
	m.putStructure(cid, 0);
	const auto fa = m.putStructure(fid, 4);
	m.putRefPtr();
	m.putStructure(iid, 0);
	m.putStructurePtr(aa.addr);
	m.putStructurePtr(ab.addr);
	return fa;
}

Address makeStrRef(Wam &m) {
	const auto addr = makeStr(m);
	return m.putStructurePtr(addr.addr);
}

int main() {
	std::array<std::tuple<Address (*)(Wam &), std::string>, 3> data;
	data[0] = {makeRef, "REF"};
	data[1] = {makeStr, "Structure"};
	data[2] = {makeStrRef, "STR"};

	const auto print = [](const Wam &m, const Wam &expected, const Wam &beforeRevert) {
		std::cout << "  Before revert:" << std::endl;
		mod::lib::Wam_write(beforeRevert, 2);
		std::cout << "  After revert:" << std::endl;
		mod::lib::Wam_write(m, 2);
		std::cout << "  Expected:" << std::endl;
		mod::lib::Wam_write(expected, 2);
	};

	for(const auto &[f1, n1]: data) {
		for(const auto &[f2, n2]: data) {
			std::cout << "UnifyHeapHeap(" << n1 << ", " << n2 << ")" << std::endl;
			Wam m;
			const auto a1 = f1(m);
			const auto a2 = f2(m);
			const Wam expected = m;
			MGU mgu(m.getHeap().size());
			m.unifyHeapHeap(a1.addr, a2.addr, mgu);
			const Wam beforeRevert = m;
			m.revert(mgu);
			if(m.getHeap() != expected.getHeap()) {
				print(m, expected, beforeRevert);
				std::cout << "Not equal heap" << std::endl;
				assert(false);
			}
			if(m.getTemp() != expected.getTemp()) {
				print(m, expected, beforeRevert);
				std::cout << "Not equal temp" << std::endl;
				assert(false);
			}
		}
	}
	for(const auto &[f1, n1]: data) {
		for(const auto &[f2, n2]: data) {
			std::cout << "UnifyHeapTemp(" << n1 << ", " << n2 << ")" << std::endl;
			Wam m;
			const auto a1 = f1(m);
			Wam temp;
			const auto a2 = f2(temp);
			m.setTemp(temp);
			const Wam expected = m;
			MGU mgu(m.getHeap().size());
			m.unifyHeapTemp(a1.addr, a2.addr, mgu);
			const Wam beforeRevert = m;
			m.revert(mgu);
			if(m.getHeap() != expected.getHeap()) {
				print(m, expected, beforeRevert);
				std::cout << "Not equal heap" << std::endl;
				assert(false);
			}
			if(m.getTemp() != expected.getTemp()) {
				print(m, expected, beforeRevert);
				std::cout << "Not equal temp" << std::endl;
				assert(false);
			}
		}
	}
	for(const auto &[f, n]: data) {
		std::cout << "CopyFromTemp(" << n << ")" << std::endl;
		Wam temp;
		const auto a = f(temp);
		Wam m;
		m.setTemp(temp);
		const Wam expected = m;
		MGU mgu(m.getHeap().size());
		m.copyFromTemp(a.addr, mgu);
		const Wam beforeRevert = m;
		m.revert(mgu);
		if(m.getHeap() != expected.getHeap()) {
			print(m, expected, beforeRevert);
			std::cout << "Not equal heap" << std::endl;
			assert(false);
		}
		if(m.getTemp() != expected.getTemp()) {
			print(m, expected, beforeRevert);
			std::cout << "Not equal temp" << std::endl;
			assert(false);
		}
	}
}
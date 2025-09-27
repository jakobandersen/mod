#ifndef MOD_PY_VERTEXMAP_HPP
#define MOD_PY_VERTEXMAP_HPP

#include <mod/VertexMap.hpp>

#include <mod/py/Common.hpp>

namespace mod::Py {

template<typename Type>
void exportVertexMap(const char *name) {
	// documentation is in protocols.rst under VertexMap
	py::class_<Type>(name, py::no_init)
			.def(str(py::self))
			.def_readonly("domain", &Type::getDomain)
			.def_readonly("codomain", &Type::getCodomain)
			.def("__getitem__", &Type::operator[])
			.def("inverse", &Type::getInverse);
}

} // namespace mod::Py

#endif // MOD_PY_VERTEXMAP_HPP
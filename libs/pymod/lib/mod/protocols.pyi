from typing import Iterable, Protocol

class Graph(Protocol):
	vertices: Iterable[Vertex]
	edges: Iterable[Edge]

	class Vertex:
		id: int

	class Edge:
		source: Graph.Vertex
		target: Graph.Vertex


class LabelledGraph(Graph):
	vertices: Iterable[Vertex]
	edges: Iterable[Edge]

	class Vertex(Graph.Vertex):
		stringLabel: str

	class Edge(Graph.Edge):
		stringLabel: str

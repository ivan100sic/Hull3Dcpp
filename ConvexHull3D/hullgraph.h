#pragma once

#include <memory>
#include <vector>
#include <unordered_set>

namespace hullgraph {

	template<class T>
	class vertex;

	template<class T>
	class face;

	template<class T>
	class edge;

	template<class T>
	struct join_faces_result;

	template<class T>
	struct hullgraph_implementations;

	template<class T>
	class edge {
		std::shared_ptr<vertex<T>> m_origin;
		std::shared_ptr<edge<T>> m_twin;
		std::shared_ptr<edge<T>> m_next;
		std::shared_ptr<edge<T>> m_prev;
		std::shared_ptr<face<T>> m_incidentFace;
	public:
		size_t m_tag;

		std::shared_ptr<vertex<T>> origin() const {
			return m_origin;
		}

		std::shared_ptr<vertex<T>> destination() const {
			if (m_twin) {
				return m_twin->m_origin;
			}
			else {
				return nullptr;
			}
		}

		std::shared_ptr<edge<T>> twin() const {
			return m_twin;
		}
		
		std::shared_ptr<edge<T>> next() const {
			return m_next;
		}

		std::shared_ptr<edge<T>> prev() const {
			return m_prev;
		}

		std::shared_ptr<face<T>> incidentFace() const {
			return m_incidentFace;
		}

		void invalidate() {
			m_origin = nullptr;
			m_twin = nullptr;
			m_next = nullptr;
			m_prev = nullptr;
			m_incidentFace = nullptr;
		}

		friend struct hullgraph_implementations<T>;
	};

	template<class T>
	class vertex {
		T m_data;
		std::shared_ptr<edge<T>> m_incidentEdge;
	public:
		size_t m_tag;

		const T& data() const {
			return m_data;
		}

		std::shared_ptr<edge<T>> incidentEdge() const {
			return m_incidentEdge;
		}

		void invalidate() {
			m_incidentEdge = nullptr;
		}

		friend struct hullgraph_implementations<T>;
	};

	template<class T>
	class face {
		std::shared_ptr<edge<T>> m_outerComponent;
	public:
		std::shared_ptr<edge<T>> outerComponent() {
			return m_outerComponent;
		}

		void invalidate() {
			m_outerComponent = nullptr;
		}

		friend struct hullgraph_implementations<T>;
	};

	template<class T>
	struct join_faces_result {
		std::vector<std::shared_ptr<vertex<T>>> removedVertices;
		std::vector<std::shared_ptr<edge<T>>> removedEdges;
		std::shared_ptr<face<T>> newFace;
		std::vector<std::shared_ptr<edge<T>>> borderEdges;
		std::vector<std::shared_ptr<face<T>>> borderFaces;
	};

	/*
	 * Returns the inner face of a triangle containing the three given data labels, in normal order.
	 */
	template<class T>
	std::shared_ptr<face<T>> makeTriangle(const T& dataA, const T& dataB, const T& dataC) {
		return makePolygon(std::vector<T>{ dataA, dataB, dataC });
	}

	/*
	 * Returns the inner face of a polygon containing the given data labels, in normal order.
	 * Returns null if given fewer than three labels.
	 */
	template<class T>
	std::shared_ptr<face<T>> makePolygon(const std::vector<T>& data) {
		return hullgraph_implementations<T>::makePolygon(data);
	}
	
	/*
	 * Returns the list of all half-edges of the given face, in normal order, starting
	 * from the face's outerComponent() edge.
	 */
	template<class T>
	std::vector<std::shared_ptr<edge<T>>> faceToEdgeList(const std::shared_ptr<face<T>>& theFace) {
		if (!theFace) {
			return {};
		}

		std::vector<std::shared_ptr<edge<T>>> edges;
		std::shared_ptr<edge<T>> startEdge = theFace->outerComponent();
		std::shared_ptr<edge<T>> currEdge = startEdge;

		do {
			if (!currEdge) {
				return {};
			}

			edges.push_back(currEdge);
			currEdge = currEdge->next();
		} while (currEdge != startEdge);

		return edges;
	}

	/*
	 * Returns the list of all half-edges exiting the given vertex, in normal order, starting
	 * from the vertex's incidentEdge().
	 */
	template<class T>
	std::vector<std::shared_ptr<edge<T>>> adjacentEdges(const std::shared_ptr<vertex<T>>& theVertex) {
		if (!theVertex) {
			return {};
		}

		std::vector<std::shared_ptr<edge<T>>> edges;
		std::shared_ptr<edge<T>> startEdge = theVertex->incidentEdge();
		std::shared_ptr<edge<T>> currEdge = startEdge;

		do {
			if (!currEdge) {
				return {};
			}

			edges.push_back(currEdge);
			currEdge = currEdge->prev()->twin();
		} while (currEdge != startEdge);

		return edges;
	}

	/*
	 * Adds a new vertex and connects it to all the vertices of the old face.
	 * This invalidates the reference to the old face.
	 * Returns a pointer to the newly created vertex.
	 * The returned vertex's incidentEdge is guaranteed to point to the origin
	 * of oldFace's outerComponent edge.
	 */
	template<class T>
	std::shared_ptr<vertex<T>> inscribeVertex(const std::shared_ptr<face<T>>& oldFace, const T& data) {
		return hullgraph_implementations<T>::inscribeVertex(oldFace, data);
	}

	/*
	 * Given a half-edge, removes that edge from the graph. The result is undefined if one of the
	 * endpoints of the edge has degree 2. Returns a pointer to the newly created face.
	 */
	template<class T>
	std::shared_ptr<face<T>> removeEdge(const std::shared_ptr<edge<T>>& halfEdge) {
		return hullgraph_implementations<T>::removeEdge(halfEdge);
	}

	/*
	 * Joins a set of faces with a common outside border. Returns a struct describing
 	 * the removed vertices and edges (all invalidated), the newly created face, the vector
	 * listing the outside border edges and a vector containing corresponding removed faces,
	 * one for each edge. The behavior is undefined if the faces don't have a common outside border.
	 * The returned face is guaranteed to have its outerComponent edge equal to borderEdges[0]
	 */
	template<class T>
	join_faces_result<T> joinFaces(const std::vector<std::shared_ptr<face<T>>>& faces) {
		return hullgraph_implementations<T>::joinFaces(faces);
	}

	/*
	 * Removes a degree-2 node from the graph. Invalidates that node and its adjacent edges.
	 * Adds and returns a new edge bridging that node. If the given node has degree more than two,
	 * this function doesn't modify anything and returns null.
	 */
	template<class T>
	std::shared_ptr<edge<T>> removeRedundantVertex(const std::shared_ptr<vertex<T>>& theVertex) {
		return hullgraph_implementations<T>::removeRedundantVertex(theVertex);
	}

	/*
	 * Returns the list of all edges reachable from the given vertex.
	 */
	template<class T>
	std::vector<std::shared_ptr<edge<T>>> exploreGraph(const std::shared_ptr<vertex<T>>& initialVertex) {
		if (!initialVertex || !initialVertex->incidentEdge()) {
			return {};
		}

		std::vector<std::shared_ptr<edge<T>>> edgesQueue = { initialVertex->incidentEdge() };
		std::unordered_set<std::shared_ptr<edge<T>>> visitedEdges;

		size_t queueStart = 0;
		while (queueStart != edgesQueue.size()) {
			std::shared_ptr<edge<T>> currEdge = edgesQueue[queueStart++];
			for (const std::shared_ptr<edge<T>>& newEdge : { currEdge->twin(), currEdge->next(), currEdge->prev() }) {
				if (visitedEdges.count(newEdge) == 0) {
					visitedEdges.insert(newEdge);
					edgesQueue.push_back(newEdge);
				}
			}
		}

		return edgesQueue;
	}

	template<class T>
	struct hullgraph_implementations {

		static std::shared_ptr<face<T>> makePolygon(const std::vector<T>& data) {
			size_t degree = data.size();

			if (degree < 3) {
				return nullptr;
			}

			std::shared_ptr<face<T>> innerFace = std::make_shared<face<T>>();
			std::shared_ptr<face<T>> outerFace = std::make_shared<face<T>>();

			if (!innerFace || !outerFace) {
				return nullptr;
			}

			std::vector<std::shared_ptr<vertex<T>>> vertices(degree);
			std::vector<std::shared_ptr<edge<T>>> forwardEdges(degree);
			std::vector<std::shared_ptr<edge<T>>> backwardEdges(degree);

			for (size_t i = 0; i < degree; i++) {
				vertices[i] = std::make_shared<vertex<T>>();
				forwardEdges[i] = std::make_shared<edge<T>>();
				backwardEdges[i] = std::make_shared<edge<T>>();
				if (!vertices[i] || !forwardEdges[i] || !backwardEdges[i]) {
					return nullptr;
				}
			}

			for (size_t i = 0; i < degree; i++) {
				size_t iPrev = i == 0 ? degree - 1 : i - 1;
				size_t iNext = i == degree - 1 ? 0 : i + 1;

				vertices[i]->m_data = data[i];
				vertices[i]->m_incidentEdge = forwardEdges[i];

				forwardEdges[i]->m_origin = vertices[i];
				backwardEdges[i]->m_origin = vertices[iNext];

				forwardEdges[i]->m_twin = backwardEdges[i];
				backwardEdges[i]->m_twin = forwardEdges[i];

				forwardEdges[i]->m_next = forwardEdges[iNext];
				backwardEdges[i]->m_next = backwardEdges[iPrev];

				forwardEdges[i]->m_prev = forwardEdges[iPrev];
				backwardEdges[i]->m_prev = backwardEdges[iNext];

				forwardEdges[i]->m_incidentFace = innerFace;
				backwardEdges[i]->m_incidentFace = outerFace;
			}

			innerFace->m_outerComponent = forwardEdges[0];
			outerFace->m_outerComponent = backwardEdges[0];

			return innerFace;
		}

		static std::shared_ptr<vertex<T>> inscribeVertex(const std::shared_ptr<face<T>>& oldFace, const T& data) {
			std::vector<std::shared_ptr<edge<T>>> edges = faceToEdgeList(oldFace);
			size_t degree = edges.size();

			if (!degree) {
				return nullptr;
			}

			std::shared_ptr<vertex<T>> newVertex = std::make_shared<vertex<T>>();
			if (!newVertex) {
				return nullptr;
			}

			std::vector<std::shared_ptr<edge<T>>> newEdgesFrom(degree);
			std::vector<std::shared_ptr<edge<T>>> newEdgesTo(degree);
			std::vector<std::shared_ptr<face<T>>> newFaces(degree);

			for (size_t i = 0; i < degree; i++) {
				newEdgesFrom[i] = std::make_shared<edge<T>>();
				newEdgesTo[i] = std::make_shared<edge<T>>();
				newFaces[i] = std::make_shared<face<T>>();

				if (!newEdgesFrom[i] || !newEdgesTo[i] || !newFaces[i]) {
					return nullptr;
				}
			}

			for (size_t i = 0; i < degree; i++) {
				size_t iPrev = i == 0 ? degree - 1 : i - 1;
				size_t iNext = i == degree - 1 ? 0 : i + 1;

				newEdgesFrom[i]->m_twin = newEdgesTo[i];
				newEdgesTo[i]->m_twin = newEdgesFrom[i];

				newEdgesFrom[i]->m_origin = newVertex;
				newEdgesTo[i]->m_origin = edges[i]->m_origin;

				newEdgesFrom[i]->m_next = edges[i];
				newEdgesFrom[i]->m_prev = newEdgesTo[iNext];
				newEdgesTo[i]->m_next = newEdgesFrom[iPrev];
				newEdgesTo[i]->m_prev = edges[iPrev];

				newEdgesFrom[i]->m_incidentFace = newFaces[i];
				newEdgesTo[i]->m_incidentFace = newFaces[iPrev];

				newFaces[i]->m_outerComponent = newEdgesFrom[i];

				edges[i]->m_next = newEdgesTo[iNext];
				edges[i]->m_prev = newEdgesFrom[i];
				edges[i]->m_incidentFace = newFaces[i];
			}

			newVertex->m_data = data;
			newVertex->m_incidentEdge = newEdgesFrom[0];

			oldFace->invalidate();

			return newVertex;
		}

		static std::shared_ptr<face<T>> removeEdge(const std::shared_ptr<edge<T>>& halfEdge) {
			std::shared_ptr<face<T>> newFace = std::make_shared<face<T>>();

			if (!newFace) {
				return nullptr;
			}

			std::shared_ptr<vertex<T>> u = halfEdge->origin();
			std::shared_ptr<vertex<T>> v = halfEdge->destination();

			std::shared_ptr<edge<T>> twinEdge = halfEdge->twin();
			std::shared_ptr<edge<T>> fromU = halfEdge->next();
			std::shared_ptr<edge<T>> toU = twinEdge->prev();
			std::shared_ptr<edge<T>> fromV = twinEdge->next();
			std::shared_ptr<edge<T>> toV = halfEdge->prev();

			std::shared_ptr<face<T>> upperFace = halfEdge->incidentFace();
			std::shared_ptr<face<T>> lowerFace = twinEdge->incidentFace();

			std::vector<std::shared_ptr<edge<T>>> upperFaceEdges = faceToEdgeList(upperFace);
			std::vector<std::shared_ptr<edge<T>>> lowerFaceEdges = faceToEdgeList(lowerFace);

			newFace->m_outerComponent = fromU;

			for (const std::shared_ptr<edge<T>>& upperFaceEdge : upperFaceEdges) {
				upperFaceEdge->m_incidentFace = newFace;
			}

			for (const std::shared_ptr<edge<T>>& lowerFaceEdge : lowerFaceEdges) {
				lowerFaceEdge->m_incidentFace = newFace;
			}

			fromU->m_prev = toU;
			toU->m_next = fromU;
			toV->m_next = fromV;
			fromV->m_prev = toV;

			u->m_incidentEdge = fromU;
			v->m_incidentEdge = fromV;

			upperFace->invalidate();
			lowerFace->invalidate();
			halfEdge->invalidate();
			twinEdge->invalidate();

			return newFace;
		}

		static join_faces_result<T> joinFaces(const std::vector<std::shared_ptr<face<T>>>& faces) {
			join_faces_result<T> result;

			result.newFace = std::make_shared<face<T>>();

			if (!faces.size() || !result.newFace) {
				return result;
			}

			// Remove tags from all vertices and edges (both half-edges and twins)
			for (const std::shared_ptr<face<T>>& facePtr : faces) {
				for (const std::shared_ptr<edge<T>>& edgePtr : faceToEdgeList(facePtr)) {
					edgePtr->m_tag = 0;
					edgePtr->twin()->m_tag = 0;
					edgePtr->origin()->m_tag = 0;
				}
			}

			// Tag all half-edges
			for (const std::shared_ptr<face<T>>& facePtr : faces) {
				for (const std::shared_ptr<edge<T>>& edgePtr : faceToEdgeList(facePtr)) {
					edgePtr->m_tag = 1;
				}
			}

			// The half-edges whose twins are not tagged form the border of the new face
			std::vector<std::shared_ptr<edge<T>>>& borderEdges = result.borderEdges;
			std::shared_ptr<edge<T>> startEdge;

			// Find a starting edge
			for (const std::shared_ptr<face<T>>& facePtr : faces) {
				bool found = false;
				for (const std::shared_ptr<edge<T>>& edgePtr : faceToEdgeList(facePtr)) {
					if (edgePtr->twin()->m_tag == 0) {
						startEdge = edgePtr;
						found = true;
						break;
					}
				}

				if (found) {
					break;
				}
			}

			// Walk around to find the border
			std::shared_ptr<edge<T>> currEdge = startEdge;

			do {
				// Rotate until you find a border edge
				currEdge = currEdge->next();
				while (currEdge->twin()->m_tag == 1) {
					currEdge = currEdge->twin()->next();
				}
				result.borderEdges.push_back(currEdge);
				result.borderFaces.push_back(currEdge->incidentFace());
			} while (currEdge != startEdge);

			// We have the border, tag all the vertices on it
			for (const std::shared_ptr<edge<T>> borderEdge : borderEdges) {
				borderEdge->origin()->m_tag = 1;
			}

			std::unordered_set<std::shared_ptr<vertex<T>>> removedVerticesSet;

			// Process all vertices and edges ready for removal
			for (const std::shared_ptr<face<T>>& facePtr : faces) {
				for (const std::shared_ptr<edge<T>>& edgePtr : faceToEdgeList(facePtr)) {
					if (edgePtr->twin()->m_tag == 1) {
						result.removedEdges.push_back(edgePtr);
					}

					if (edgePtr->origin()->m_tag == 0) {
						removedVerticesSet.insert(edgePtr->origin());
					}
				}
			}

			result.removedVertices.assign(removedVerticesSet.begin(), removedVerticesSet.end());

			// Invalidate removed objects
			for (const std::shared_ptr<vertex<T>>& removedVertex : result.removedVertices) {
				removedVertex->invalidate();
			}

			for (const std::shared_ptr<edge<T>>& removedEdge : result.removedEdges) {
				removedEdge->invalidate();
			}

			for (const std::shared_ptr<face<T>>& removedFace : faces) {
				removedFace->invalidate();
			}

			// Connect the new face with its border
			result.newFace->m_outerComponent = borderEdges[0];

			for (size_t i = 0; i < borderEdges.size(); i++) {
				size_t iPrev = i == 0 ? borderEdges.size() - 1 : i - 1;
				size_t iNext = i == borderEdges.size() - 1 ? 0 : i + 1;

				borderEdges[i]->m_incidentFace = result.newFace;
				borderEdges[i]->m_origin->m_incidentEdge = borderEdges[i];
				borderEdges[i]->m_next = borderEdges[iNext];
				borderEdges[i]->m_prev = borderEdges[iPrev];
			}

			return result;
		}

		static std::shared_ptr<edge<T>> removeRedundantVertex(const std::shared_ptr<vertex<T>>& theVertex) {
			std::shared_ptr<edge<T>> outEdge1 = theVertex->incidentEdge();
			std::shared_ptr<edge<T>> outEdge1twin = outEdge1->twin();
			std::shared_ptr<edge<T>> outEdge2 = outEdge1twin->next();
			std::shared_ptr<edge<T>> outEdge2twin = outEdge2->twin();

			if (outEdge2twin->next() != outEdge1) {
				// The degree is at least 3
				return nullptr;
			}

			std::shared_ptr<edge<T>> newEdge = std::make_shared<edge<T>>();
			std::shared_ptr<edge<T>> twinEdge = std::make_shared<edge<T>>();

			if (!newEdge || !twinEdge) {
				return nullptr;
			}

			std::shared_ptr<edge<T>> nextEdge1 = outEdge1->next();
			std::shared_ptr<edge<T>> nextEdge2 = outEdge2->next();
			std::shared_ptr<edge<T>> prevEdge1 = outEdge1twin->prev();
			std::shared_ptr<edge<T>> prevEdge2 = outEdge2twin->prev();

			// Set up the new edge pair

			newEdge->m_origin = nextEdge2->origin();
			twinEdge->m_origin = nextEdge1->origin();

			newEdge->m_twin = twinEdge;
			twinEdge->m_twin = newEdge;

			newEdge->m_next = nextEdge1;
			twinEdge->m_next = nextEdge2;

			newEdge->m_prev = prevEdge2;
			twinEdge->m_prev = prevEdge1;

			newEdge->m_incidentFace = outEdge1->incidentFace();
			twinEdge->m_incidentFace = outEdge2->incidentFace();

			// Restore vertex properties
			newEdge->origin()->m_incidentEdge = newEdge;
			twinEdge->origin()->m_incidentEdge = twinEdge;

			// Restore face properties
			newEdge->incidentFace()->m_outerComponent = newEdge;
			twinEdge->incidentFace()->m_outerComponent = twinEdge;

			// Restore edge properties
			nextEdge1->m_prev = newEdge;
			prevEdge1->m_next = twinEdge;
			prevEdge2->m_next = newEdge;
			nextEdge2->m_prev = twinEdge;

			// Invalidate the vertex
			theVertex->invalidate();

			// Invalidate the old edges
			outEdge1->invalidate();
			outEdge1twin->invalidate();
			outEdge2->invalidate();
			outEdge2twin->invalidate();

			return newEdge;
		}
	};
}

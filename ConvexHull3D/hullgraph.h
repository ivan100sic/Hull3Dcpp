#pragma once

#include <memory>
#include <vector>

namespace hullgraph {

	template<class T>
	class vertex;

	template<class T>
	class face;

	template<class T>
	class edge;

	template<class T>
	std::shared_ptr<face<T>> makeTriangle(const T& dataA, const T& dataB, const T& dataC);

	template<class T>
	std::shared_ptr<vertex<T>> inscribePoint(std::shared_ptr<face<T>> oldFace, const T& data);

	template<class T>
	std::shared_ptr<face<T>> removeEdge(std::shared_ptr<edge<T>> halfEdge);

	template<class T>
	class edge {
		std::shared_ptr<vertex<T>> m_origin;
		std::shared_ptr<edge<T>> m_twin;
		std::shared_ptr<edge<T>> m_next;
		std::shared_ptr<edge<T>> m_prev;
		std::shared_ptr<face<T>> m_incidentFace;
	public:
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
			return m_next;
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

		friend std::shared_ptr<face<T>> makeTriangle<T>(const T& dataA, const T& dataB, const T& dataC);
		friend std::shared_ptr<vertex<T>> inscribePoint<T>(std::shared_ptr<face<T>> oldFace, const T& data);
		friend std::shared_ptr<face<T>> removeEdge<T>(std::shared_ptr<edge<T>> halfEdge);
	};

	template<class T>
	class vertex {
		T m_data;
		std::shared_ptr<edge<T>> m_incidentEdge;
	public:
		const T& data() const {
			return m_data;
		}

		std::shared_ptr<edge<T>> incidentEdge() const {
			return m_incidentEdge;
		}

		void invalidate() {
			m_incidentEdge = nullptr;
		}

		friend std::shared_ptr<face<T>> makeTriangle<T>(const T& dataA, const T& dataB, const T& dataC);
		friend std::shared_ptr<vertex<T>> inscribePoint<T>(std::shared_ptr<face<T>> oldFace, const T& data);
		friend std::shared_ptr<face<T>> removeEdge<T>(std::shared_ptr<edge<T>> halfEdge);
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

		friend std::shared_ptr<face<T>> makeTriangle<T>(const T& dataA, const T& dataB, const T& dataC);
		friend std::shared_ptr<vertex<T>> inscribePoint<T>(std::shared_ptr<face<T>> oldFace, const T& data);
		friend std::shared_ptr<face<T>> removeEdge<T>(std::shared_ptr<edge<T>> halfEdge);
	};

	/*
	 * Returns the inner face of a triangle containing the three given data labels, in normal order.
	 */
	template<class T>
	std::shared_ptr<face<T>> makeTriangle(const T& dataA, const T& dataB, const T& dataC) {
		std::shared_ptr<vertex<T>> a = std::make_shared<vertex<T>>();
		std::shared_ptr<vertex<T>> b = std::make_shared<vertex<T>>();
		std::shared_ptr<vertex<T>> c = std::make_shared<vertex<T>>();

		if (!a || !b || !c) {
			return nullptr;
		}

		a->m_data = dataA;
		b->m_data = dataB;
		c->m_data = dataC;

		std::shared_ptr<edge<T>> ab = std::make_shared<edge<T>>();
		std::shared_ptr<edge<T>> bc = std::make_shared<edge<T>>();
		std::shared_ptr<edge<T>> ca = std::make_shared<edge<T>>();
		std::shared_ptr<edge<T>> ba = std::make_shared<edge<T>>();
		std::shared_ptr<edge<T>> cb = std::make_shared<edge<T>>();
		std::shared_ptr<edge<T>> ac = std::make_shared<edge<T>>();

		if (!ab || !bc || !ca || !ba || !cb || !ac) {
			return nullptr;
		}

		ab->m_origin = a;
		bc->m_origin = b;
		ca->m_origin = c;
		ba->m_origin = b;
		cb->m_origin = c;
		ac->m_origin = a;

		ab->m_twin = ba;
		bc->m_twin = cb;
		ca->m_twin = ac;
		ba->m_twin = ab;
		cb->m_twin = bc;
		ac->m_twin = ca;

		ab->m_next = bc;
		bc->m_next = ca;
		ca->m_next = ab;
		ba->m_next = ac;
		ac->m_next = cb;
		cb->m_next = ba;

		ab->m_prev = ca;
		bc->m_prev = ab;
		ca->m_prev = bc;
		ba->m_prev = cb;
		ac->m_prev = ba;
		cb->m_prev = ac;

		a->m_incidentEdge = ab;
		b->m_incidentEdge = bc;
		c->m_incidentEdge = ca;

		std::shared_ptr<face<T>> innerFace = std::make_shared<face<T>>();
		std::shared_ptr<face<T>> outerFace = std::make_shared<face<T>>();

		if (!innerFace || !outerFace) {
			return nullptr;
		}

		innerFace->m_outerComponent = ab;
		outerFace->m_outerComponent = ba;

		ab->m_incidentFace = innerFace;
		bc->m_incidentFace = innerFace;
		ca->m_incidentFace = innerFace;
		ba->m_incidentFace = outerFace;
		cb->m_incidentFace = outerFace;
		ac->m_incidentFace = outerFace;

		return innerFace;
	}
	
	template<class T>
	std::vector<std::shared_ptr<edge<T>>> faceToEdgeList(std::shared_ptr<face<T>> theFace) {
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
	 * Adds a new vertex and connects it to all the vertices of the old face.
	 * This invalidates the reference to the old face.
	 * Returns a pointer to the newly created vertex.
	 */
	template<class T>
	std::shared_ptr<vertex<T>> inscribePoint(std::shared_ptr<face<T>> oldFace, const T& data) {
		std::vector<std::shared_ptr<edge<T>>> edges = faceToEdgeList(oldFace);
		size_t degree = edges.size();

		if (!degree) {
			return nullptr;
		}

		std::shared_ptr<vertex<T>> newVertex = std::make_shared<vertex<T>>();
		if (!newVertex) {
			return nullptr;
		}

		newVertex->m_data = data;

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
			newEdgesTo[i]->m_next = newEdgesTo[iPrev];
			newEdgesTo[i]->m_prev = edges[iPrev];

			newEdgesFrom[i]->m_incidentFace = newFaces[i];
			newEdgesTo[i]->m_incidentFace = newFaces[iPrev];

			newFaces[i]->m_outerComponent = newEdgesFrom[i];

			edges[i]->m_next = newEdgesTo[iNext];
			edges[i]->m_prev = newEdgesFrom[i];
			edges[i]->m_incidentFace = newFaces[i];
		}
		
		newVertex->m_incidentEdge = newEdgesFrom[0];

		oldFace->invalidate();

		return newVertex;
	}

	/*
	 * Given a half-edge, removes an edge from the graph. The result is undefined if one of the
	 * endpoints of the edge has degree 2. Returns a pointer to the newly created face.
	 */
	template<class T>
	std::shared_ptr<face<T>> removeEdge(std::shared_ptr<edge<T>> halfEdge) {
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

		for (auto& upperFaceEdge : upperFaceEdges) {
			upperFaceEdge->m_incidentFace = newFace;
		}

		for (auto& upperFaceEdge : lowerFaceEdges) {
			upperFaceEdge->m_incidentFace = newFace;
		}

		fromU->m_prev = toU;
		fromU->m_twin->m_next = toU->m_twin;
		toU->m_next = fromU;
		toU->m_twin->m_prev = fromU->m_twin;
		toV->m_next = fromV;
		toV->m_twin->m_prev = fromV->m_twin;
		fromV->m_prev = toV;
		fromV->m_twin->m_next = toV->m_twin;

		u->m_incidentEdge = fromU;
		v->m_incidentEdge = fromV;

		upperFace->invalidate();
		lowerFace->invalidate();
		halfEdge->invalidate();
		twinEdge->invalidate();

		return newFace;
	}
}


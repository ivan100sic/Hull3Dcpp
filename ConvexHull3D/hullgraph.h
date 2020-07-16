#pragma once

#include <memory>

namespace hullgraph {

	template<class T>
	class vertex;

	template<class T>
	class face;

	template<class T>
	std::shared_ptr<face<T>> makeTriangle(const T& dataA, const T& dataB, const T& dataC);

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

		friend std::shared_ptr<face<T>> makeTriangle<T>(const T& dataA, const T& dataB, const T& dataC);
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

		friend std::shared_ptr<face<T>> makeTriangle<T>(const T& dataA, const T& dataB, const T& dataC);
	};

	template<class T>
	class face {
		std::shared_ptr<edge<T>> m_outerComponent;
	public:
		std::shared_ptr<edge<T>> outerComponent() {
			return m_outerComponent;
		}

		friend std::shared_ptr<face<T>> makeTriangle<T>(const T& dataA, const T& dataB, const T& dataC);
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
}


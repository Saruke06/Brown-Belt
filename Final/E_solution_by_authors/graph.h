#pragma once

#include <cstdlib>
#include <deque>
#include <vector>
#include <string>

template <typename It>
class Range {
public:
	using ValueType = typename std::iterator_traits<It>::value_type;

	Range(It begin, It end) : begin_(begin), end_(end) {}
        [[nodiscard]] auto begin() const -> It { return begin_; }
        [[nodiscard]] auto end() const -> It { return end_; }

    private:
	It begin_;
	It end_;
};

namespace Graph {

	enum EDGE_TYPE {
		WAIT,
		BUS
	};

	using VertexId = size_t;
	using EdgeId = size_t;

	template <typename Weight>
	struct Edge {
		VertexId from;
		VertexId to;
		Weight weight;
		EDGE_TYPE edge_type;
		std::string name;
		size_t span = 0;
	};

	template <typename Weight>
	class DirectedWeightedGraph {
	private:
		using IncidenceList = std::vector<EdgeId>;
		using IncidentEdgesRange = Range<typename IncidenceList::const_iterator>;

	public:
		DirectedWeightedGraph(size_t vertex_count);
                auto AddEdge(const Edge<Weight>& edge) -> EdgeId;

                [[nodiscard]] auto GetVertexCount() const -> size_t;
                [[nodiscard]] auto GetEdgeCount() const -> size_t;
                auto GetEdge(EdgeId edge_id) const -> const Edge<Weight>&;
                [[nodiscard]] auto GetIncidentEdges(VertexId vertex) const -> IncidentEdgesRange;

            private:
		std::vector<Edge<Weight>> edges_;
		std::vector<IncidenceList> incidence_lists_;
	};


	template <typename Weight>
	DirectedWeightedGraph<Weight>::DirectedWeightedGraph(size_t vertex_count) : incidence_lists_(vertex_count) {}

        template <typename Weight>
        auto DirectedWeightedGraph<Weight>::AddEdge(const Edge<Weight>& edge) -> EdgeId
        {
            edges_.push_back(edge);
            const EdgeId id = edges_.size() - 1;
            incidence_lists_[edge.from].push_back(id);
            return id;
        }

        template <typename Weight>
        auto DirectedWeightedGraph<Weight>::GetVertexCount() const -> size_t
        {
            return incidence_lists_.size();
        }

        template <typename Weight>
        auto DirectedWeightedGraph<Weight>::GetEdgeCount() const -> size_t
        {
            return edges_.size();
        }

        template <typename Weight>
        auto DirectedWeightedGraph<Weight>::GetEdge(EdgeId edge_id) const -> const Edge<Weight>&
        {
            return edges_[edge_id];
        }

        template <typename Weight>
        auto
        DirectedWeightedGraph<Weight>::GetIncidentEdges(VertexId vertex) const -> typename DirectedWeightedGraph<Weight>::IncidentEdgesRange
        {
            const auto& edges = incidence_lists_[vertex];
            return { std::begin(edges), std::end(edges) };
        }
}
#ifndef NM_HydraulicNetworkModel_pH
#define NM_HydraulicNetworkModel_pH

#include <vector>

namespace NANDRAD_MODEL {

/*! Define a network connection. */
struct Element {
	Element() {}
	Element(unsigned int n_inlet, unsigned int n_outlet) :
		m_nInlet(n_inlet), m_nOutlet(n_outlet) {}

	/*! Index of inlet node. */
	unsigned int m_nInlet;
	/*! Index of outlet node. */
	unsigned int m_nOutlet;
};

/*! Defines a network node including connected elements. */
struct Node {
	Node() {}
	Node(unsigned int i1, unsigned int i2) {
		m_elementIndexesInlet.push_back(i1);
		m_elementIndexesOutlet.push_back(i2);
		m_elementIndexes.push_back(i1);
		m_elementIndexes.push_back(i2);
	}

	/*! Vector with indexes of inlet flow elements. */
	std::vector<unsigned int> m_elementIndexesInlet;
	std::vector<unsigned int> m_elementIndexesOutlet;
	/*! Complete vector of indexes. */
	std::vector<unsigned int> m_elementIndexes;
};

/*! Defines a hydraulic network including nodes and elements.
	Contrary to the NANDRAD data structure, indexes (not ids) of elements and nodes
	are stored as well as their connecitivy information.
*/
struct Network {
	/*! Sorted list of elements*/
	std::vector<Element>	m_elements;
	/*! Sorted list of Nodes*/
	std::vector<Node>		m_nodes;
};


} // namespace NANDRAD_MODEL

#endif // NM_HydraulicNetworkModel_pH
/*	NANDRAD Solver Framework and Model Implementation.

	Copyright (c) 2012-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  Anne Paepcke     <anne.paepcke -[at]- tu-dresden.de>

	This library is part of SIM-VICUS (https://github.com/ghorwin/SIM-VICUS)

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 3 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
*/

#include <IBK_messages.h>

#include "NM_ThermalNetworkPrivate.h"

#include "NM_HydraulicNetwork.h"

#include <IBK_assert.h>
#include <IBK_Exception.h>
#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



const double *ThermalNetworkModelImpl::heatFluxes() const {
	if(!m_heatFluxes.empty())
		return &m_heatFluxes[0];
	return nullptr;
}

void ThermalNetworkModelImpl::setup(const HydraulicNetwork &nw) {
	// copy nodes pointer from network
	m_network = &nw;
	// resize specific enthalpy
	m_specificEnthalpy.resize(nw.m_nodes.size());
	// resize heat fluxes
	m_heatFluxes.resize(nw.m_elements.size());
}


int ThermalNetworkModelImpl::updateStates() {

	// calculate enthalpy fluxes for all nodes
	for(unsigned int i = 0; i < m_network->m_nodes.size(); ++i) {
		// set enthalpy flux to 0
		double specEnthalp = 0;

		std::vector<unsigned int> inletIdxs =
				m_network->m_nodes[i].m_elementIndexesInlet;
		std::vector<unsigned int> outletIdxs =
				m_network->m_nodes[i].m_elementIndexesOutlet;

		double massFluxInlet = 0.0;
		// select all pipes with positive flux into element
		for(unsigned int idx : inletIdxs) {
			const double massFlux = m_massFluxes[idx];
			if(massFlux > 0) {
				massFluxInlet += massFlux;
				// and retrieve specfic enthalpy
				double specEnthalpy;
				m_flowElements[idx]->outletSpecificEnthalpy(specEnthalpy);
				// sum up
				specEnthalp += massFlux * specEnthalpy;
			}
		}
		// select all pipes with negative flux into element
		for(unsigned int idx : outletIdxs) {
			const double massFlux = m_massFluxes[idx];
			if(massFlux < 0) {
				massFluxInlet -= massFlux;
				// and retrieve specfic enthalpy
				double specEnthalpy;
				m_flowElements[idx]->outletSpecificEnthalpy(specEnthalpy);
				// sum up
				specEnthalp -= massFlux * specEnthalpy;
			}
		}
		IBK_ASSERT(massFluxInlet != 0.0);
		specEnthalp/=massFluxInlet;

		m_specificEnthalpy[i] = specEnthalp;
	}
	return 0;
}

int ThermalNetworkModelImpl::updateFluxes() 	{
	// set enthalpy and mass fluxes for all flow elements
	// and update their simulation results
	for(unsigned int i = 0; i < m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *flowElem = m_flowElements[i];
		const HydraulicNetworkElement &fe = m_network->m_elements[i];
		// get inlet node
		const double massFlux = m_massFluxes[i];
		const double specEnthalpInlet = m_specificEnthalpy[fe.m_nInlet];
		const double specEnthalpOutlet = m_specificEnthalpy[fe.m_nOutlet];
		// positive mass flux
		if(massFlux >= 0) {
			flowElem->setInletFluxes(massFlux, specEnthalpInlet * massFlux);
		}
		// negative mass flux
		else {
			flowElem->setInletFluxes(massFlux, specEnthalpOutlet * massFlux);
		}
		// heat los equals difference of enthalpy fluxes between inlet and outlet
		m_heatFluxes[i] = massFlux * (specEnthalpInlet - specEnthalpOutlet);
	}
	return 0;
}



} // namespace NANDRAD_MODEL
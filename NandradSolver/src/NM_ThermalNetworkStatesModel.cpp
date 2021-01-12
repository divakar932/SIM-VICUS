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

#include "NM_ThermalNetworkStatesModel.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_ThermalNetworkPrivate.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw,
									  const HydraulicNetworkModel &networkModel) {
	FUNCID(ThermalNetworkStatesModel::setup);

	// store network pointer
	m_network = &nw;
	// create implementation instance
	m_p = new ThermalNetworkModelImpl; // we take ownership

	// first register all nodes
	std::set<unsigned int> nodeIds;
	// for this purpose process all hydraulic network elements
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		nodeIds.insert(e.m_inletNodeId);
		nodeIds.insert(e.m_outletNodeId);
	}

	// now populate the m_edges vector of the network solver

	// process all hydraulic network elements and instatiate respective flow equation classes
	for (const NANDRAD::HydraulicNetworkElement & e : nw.m_elements) {
		// - instance-specific parameters from HydraulicNetworkElement e
		// - fluid property object from nw.m_fluid
		// - component definition (via reference from e.m_componentId) and component DB stored
		//   in network

		try {
			// retrieve component

			std::vector<NANDRAD::HydraulicNetworkComponent>::const_iterator itComp =
					std::find(nw.m_components.begin(), nw.m_components.end(), e.m_componentId);
			IBK_ASSERT(itComp != nw.m_components.end());

			switch (itComp->m_modelType) {
				case NANDRAD::HydraulicNetworkComponent::MT_StaticPipe :
				case NANDRAD::HydraulicNetworkComponent::MT_StaticAdiabaticPipe :
				case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
				case NANDRAD::HydraulicNetworkComponent::MT_DynamicAdiabaticPipe :
				{
					// lookup pipe
					std::vector<NANDRAD::HydraulicNetworkPipeProperties>::const_iterator itPipe =
							std::find(m_network->m_pipeProperties.begin(), m_network->m_pipeProperties.end(), e.m_pipeId);
					if (itPipe == m_network->m_pipeProperties.end()) {
						throw IBK::Exception(IBK::FormatString("Missing pipe properties reference in hydraulic network element '%1' (id=%2).")
											 .arg(e.m_displayName).arg(e.m_id), FUNC_ID);
					}

					// create hydraulic pipe model
					TNPipeElement * pipeElement = new TNPipeElement(e, *itComp,  *itPipe, m_network->m_fluid);
					// add to flow elements
					m_p->m_flowElements.push_back(pipeElement); // transfer ownership
				} break;


				case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePumpModel :
				{
					TNPump * pumpElement = new TNPump(e, *itComp, m_network->m_fluid);
					m_p->m_flowElements.push_back(pumpElement);
					break;
				}


				case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger:
				{
					TNHeatExchanger * heatEx = new TNHeatExchanger(e, *itComp, m_network->m_fluid);
					m_p->m_flowElements.push_back(heatEx);
					break;
				}
				default: {
					throw IBK::Exception(IBK::FormatString("Model of type %1 is not supported, yet!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::ModelType",
								itComp->m_modelType)),
								FUNC_ID);
				}
				break;
			}
			// decide which heat exchange is chosen
			switch(itComp->m_heatExchangeType) {
				case NANDRAD::HydraulicNetworkComponent::HT_HeatFluxConstant: {
					// retrieve constant temperature
					if(!e.m_para[NANDRAD::HydraulicNetworkElement::P_Temperature].name.empty()) {
						m_p->m_ambientTemperatureRefs.push_back(
							&e.m_para[NANDRAD::HydraulicNetworkElement::P_Temperature].value);
						// retrieve external heat transfer coefficient
						if(itComp->m_para[NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].name.empty()){
							throw IBK::Exception(IBK::FormatString("Missing parameteter '%1 for exchange type %2!")
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::para_t",
										NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient))
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
										itComp->m_heatExchangeType)),
										FUNC_ID);
						}
						m_p->m_ambientHeatTransferRefs.push_back(&itComp->m_para[
							NANDRAD::HydraulicNetworkComponent::P_ExternalHeatTransferCoefficient].value);
						// set heat flux to zero
						m_p->m_ambientHeatFluxRefs.push_back(nullptr);
					}
					else if(!e.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].name.empty()) {
						m_p->m_ambientHeatFluxRefs.push_back(
							&e.m_para[NANDRAD::HydraulicNetworkElement::P_HeatFlux].value);
						// set temperature and heat trabsfer coefficient to zero
						m_p->m_ambientTemperatureRefs.push_back(nullptr);
						m_p->m_ambientHeatTransferRefs.push_back(nullptr);
					}
					else {
						m_p->m_ambientHeatFluxRefs.push_back(nullptr);
						m_p->m_ambientTemperatureRefs.push_back(nullptr);
						m_p->m_ambientHeatTransferRefs.push_back(nullptr);
					}
				} break;
				default: {
					throw IBK::Exception(IBK::FormatString("Heat exchange type %1 is not supported, yet!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								itComp->m_heatExchangeType)),
								FUNC_ID);
				}
				break;
			}
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing HydraulicFlowElement with id %1")
								.arg(e.m_componentId), FUNC_ID);
		}
	}
	// setup the enetwork
	try {
		m_p->setup(*networkModel.network());
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting up flow network.", FUNC_ID);
	}

	// TODO
	// initialize all models

	// resize vectors
	m_n = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		m_n += fe->nInternalStates();
	}
	m_y.resize(m_n,0.0);
}


void ThermalNetworkStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	// TODO: implement
}


const double * ThermalNetworkStatesModel::resultValueRef(const QuantityName & quantityName) const {
	// return y
	if(quantityName == std::string("y")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return &m_y[0];
		return nullptr;
	}
	return nullptr;
}


unsigned int ThermalNetworkStatesModel::nPrimaryStateResults() const {
	return m_n;
}


void ThermalNetworkStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & resultInputValueReferences) const {
	// TODO: implement
}


void ThermalNetworkStatesModel::yInitial(double * y) const {
	// per copy default states from all models
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->initialStates(y + offset);
		offset += fe->nInternalStates();
	}
}


int ThermalNetworkStatesModel::update(const double * y) {
	// copy states vector
	std::memcpy(&m_y[0], y, m_n*sizeof(double));
	// set internal states
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->setInternalStates(y + offset);
		offset += fe->nInternalStates();
	}
	return 0;
}


} // namespace NANDRAD_MODEL

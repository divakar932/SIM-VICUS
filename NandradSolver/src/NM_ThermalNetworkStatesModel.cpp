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

#include <IBK_LinearSpline.h>
#include <IBK_messages.h>

#include "NM_ThermalNetworkStatesModel.h"

#include "NM_HydraulicNetworkModel.h"
#include "NM_ThermalNetworkPrivate.h"
#include "NM_ThermalNetworkFlowElements.h"
#include "NM_KeywordList.h"

#include <NANDRAD_HydraulicNetwork.h>
#include <NANDRAD_HydraulicNetworkComponent.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_SimulationParameter.h>

#include <IBKMK_SparseMatrixPattern.h>

#include <algorithm>

namespace NANDRAD_MODEL {



// *** ThermalNetworkStatesModel members ***

ThermalNetworkStatesModel::~ThermalNetworkStatesModel() {
	delete m_p; // delete pimpl object
}


void ThermalNetworkStatesModel::setup(const NANDRAD::HydraulicNetwork & nw,
									  const HydraulicNetworkModel &networkModel,
									  const NANDRAD::SimulationParameter &simPara) {
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

	// resize vectors
	m_heatExchangeValues.resize(nw.m_elements.size(), 0.0);
	m_heatExchangeSplineRefs.resize(nw.m_elements.size(),nullptr);

	for (unsigned int i =0; i < nw.m_elements.size(); ++i) {
		const NANDRAD::HydraulicNetworkElement & e = nw.m_elements[i];
		IBK_ASSERT(e.m_component != nullptr);

		try {
			// set reference to heat exchange value
			double &heatExchangeValue = m_heatExchangeValues[i];

			// Instantiate thermal flow element calculation objects.
			// The objects are selected based on a **combination** of modelType and heatExchangeType and
			// the parametrization of the calculation objects differs.

			switch (e.m_component->m_modelType) {
				case NANDRAD::HydraulicNetworkComponent::MT_SimplePipe :
				{
					IBK_ASSERT(e.m_pipeProperties != nullptr);

					// distinguish based on heat exchange type
					switch (e.m_heatExchange.m_modelType) {
						// create adiabatic pipe model
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							// calculate pipe volume
							const double d = e.m_pipeProperties->m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
							const double l = e.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
							double volume = PI/4. * d * d * l;
							// create pipe model with given heat flux
							TNAdiabaticElement * pipeElement = new TNAdiabaticElement(
									m_network->m_fluid, volume);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(nullptr); // no heat loss
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						{
							// calculate pipe volume
							const double d = e.m_pipeProperties->m_para[NANDRAD::HydraulicNetworkPipeProperties::P_PipeInnerDiameter].value;
							const double l = e.m_para[NANDRAD::HydraulicNetworkElement::P_Length].value;
							double volume = PI/4. * d * d * l;

							// create generic flow element with given heat flux
							TNElementWithExternalHeatLoss * pipeElement = new TNElementWithExternalHeatLoss(
									m_network->m_fluid, volume, heatExchangeValue);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_AmbientTemperatureConstant:
// toggle the define in NM_ThermalNetworkFlowElements.h
#ifdef STATIC_PIPE_MODEL_ENABLED
						{
							// create pipe model with heat exchange and static properties
							TNStaticPipeElement * pipeElement = new TNStaticPipeElement(e, *e.m_component,
									*e.m_pipeProperties, m_network->m_fluid, heatExchangeValue);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
						} break;
#endif // STATIC_PIPE_MODEL_ENABLED


						case NANDRAD::HydraulicNetworkHeatExchange::T_AmbientTemperatureSpline:
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatExchangeWithZoneTemperature:
						{
							// create pipe model with heat exchange
							TNSimplePipeElement * pipeElement = new TNSimplePipeElement(e, *e.m_component,
									*e.m_pipeProperties, m_network->m_fluid, heatExchangeValue);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatExchangeWithFMUTemperature :
							// TODO : Andreas, Milestone FMU-Networks
						break;

					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_SimplePipe


				case NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe :
				{
					IBK_ASSERT(e.m_pipeProperties != nullptr);
					// distinguish based on heat exchange type
					switch (e.m_heatExchange.m_modelType) {
						// create adiabatic pipe model
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							TNDynamicAdiabaticPipeElement * pipeElement = new TNDynamicAdiabaticPipeElement(e,
									*e.m_component,  *e.m_pipeProperties, m_network->m_fluid);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(nullptr); // no heat loss
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
							throw IBK::Exception(IBK::FormatString("Heat exchange model %1 cannot be used with DynamicPipe components.")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType", e.m_heatExchange.m_modelType)), FUNC_ID);

						case NANDRAD::HydraulicNetworkHeatExchange::T_AmbientTemperatureConstant:
						case NANDRAD::HydraulicNetworkHeatExchange::T_AmbientTemperatureSpline:
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatExchangeWithZoneTemperature:
						{
							// create pipe model with heat exchange
							TNDynamicPipeElement * pipeElement = new TNDynamicPipeElement(e,
									*e.m_component, *e.m_pipeProperties, m_network->m_fluid, heatExchangeValue);
							// add to flow elements
							m_p->m_flowElements.push_back(pipeElement); // transfer ownership
							m_p->m_heatLossElements.push_back(pipeElement); // copy of pointer
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatExchangeWithFMUTemperature :
							// TODO : Andreas, Milestone FMU-Networks
						break;

					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_DynamicPipe


				case NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePump :
				{
					// create pump model with heat loss
					TNPumpWithPerformanceLoss * element = new TNPumpWithPerformanceLoss(m_network->m_fluid,
							*e.m_component, e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_PressureHead].value);
					// add to flow elements
					m_p->m_flowElements.push_back(element); // transfer ownership
					m_p->m_heatLossElements.push_back(element); // no heat loss

				} break; // NANDRAD::HydraulicNetworkComponent::MT_ConstantPressurePump


				case NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger :
				{
					switch (e.m_heatExchange.m_modelType) {
						// create general adiabatic model
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							TNAdiabaticElement * element = new TNAdiabaticElement(m_network->m_fluid,
									e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value);
							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(nullptr); // no heat loss
						} break;

						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant :
						case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline :
						{
							// create general model with given heat flux
							TNElementWithExternalHeatLoss * element = new TNElementWithExternalHeatLoss(m_network->m_fluid,
									e.m_component->m_para[NANDRAD::HydraulicNetworkComponent::P_Volume].value, heatExchangeValue);
							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(element); // copy of pointer
						} break;

						default:
							throw IBK::Exception(IBK::FormatString("Flow element model %1 does currently not support HeatExchangeType %2.")
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::ModelType",
										e.m_component->m_modelType))
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
										e.m_heatExchange.m_modelType)),
										FUNC_ID);
					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatExchanger


				case NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot :
				{
					switch (e.m_heatExchange.m_modelType) {
						case NANDRAD::HydraulicNetworkHeatExchange::NUM_T : {
							// create general model with given heat flux
							TNHeatPumpIdealCarnot * element = new TNHeatPumpIdealCarnot(m_network->m_fluid,
									*e.m_component, heatExchangeValue);
							// add to flow elements
							m_p->m_flowElements.push_back(element); // transfer ownership
							m_p->m_heatLossElements.push_back(element); // copy of pointer
						} break;


						default:
							throw IBK::Exception(IBK::FormatString("Flow element model %1 does currently not support HeatExchangeType %2.")
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::ModelType",
										e.m_component->m_modelType))
										.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
										e.m_heatExchange.m_modelType)),
										FUNC_ID);
					} // switch heat exchange type

				} break; // NANDRAD::HydraulicNetworkComponent::MT_HeatPumpIdealCarnot

				case NANDRAD::HydraulicNetworkComponent::NUM_MT:
				break; // just to make compiler happy
			}

			// retrieve component
			// and decide which heat exchange is chosen
			switch(e.m_heatExchange.m_modelType) {
				case NANDRAD::HydraulicNetworkHeatExchange::T_AmbientTemperatureConstant:
					heatExchangeValue = e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_AmbientTemperature].value;
				break;

				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossConstant:
					heatExchangeValue = e.m_heatExchange.m_para[NANDRAD::HydraulicNetworkHeatExchange::P_HeatLoss].value;
				break;

				case NANDRAD::HydraulicNetworkHeatExchange::T_AmbientTemperatureSpline:
					// store heat flux spline
					m_heatExchangeSplineRefs[i] = &e.m_heatExchange.m_heatExchangeSpline.m_values;
				break;

				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatLossSpline:
					// store heat flux spline and construct data value
					m_heatExchangeSplineRefs[i] = &e.m_heatExchange.m_heatExchangeSpline.m_values;
				break;

				case NANDRAD::HydraulicNetworkHeatExchange::T_HeatExchangeWithZoneTemperature: {
					// check for zone id
					IBK_ASSERT(!e.m_heatExchange.m_intPara[NANDRAD::HydraulicNetworkHeatExchange::IP_ZoneId].name.empty());
					// parameters are checked, already
					unsigned int zoneId = (unsigned int) e.m_heatExchange.m_intPara[NANDRAD::HydraulicNetworkHeatExchange::IP_ZoneId].value;
					// check whether zone is registered
					if (!m_zoneIds.empty()) {
						std::vector<unsigned int>::iterator fIt = std::find(m_zoneIds.begin(), m_zoneIds.end(), zoneId);
						// add a new entry
						if(fIt == m_zoneIds.end()) {
							m_zoneIdxs[i] = m_zoneIds.size();
							m_zoneIds.push_back(zoneId);
						}
						else {
							unsigned int index = std::distance(m_zoneIds.begin(), fIt);
							m_zoneIdxs[i] = index;
						}
					}
					else {
						// resize zone idx vector
						m_zoneIdxs.resize(m_network->m_elements.size(), NANDRAD::INVALID_ID);
						m_zoneIdxs[i] = 0;
						m_zoneIds.push_back(zoneId);
					}
					// reserve reference vector
					m_zoneTemperatureRefs.push_back(nullptr);
					// set initial temperature
					heatExchangeValue = simPara.m_para[NANDRAD::SimulationParameter::P_InitialTemperature].value;
				} break;

				case NANDRAD::HydraulicNetworkHeatExchange::NUM_T:
					// No thermal exchange, nothing to initialize
				break;

				default:
					throw IBK::Exception(IBK::FormatString("Heat exchange type %1 is not supported, yet!")
								.arg(NANDRAD::KeywordList::Keyword("HydraulicNetworkComponent::HeatExchangeType",
								e.m_heatExchange.m_modelType)), FUNC_ID);
			}
		}
		catch(IBK::Exception &ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error initializing HydraulicFlowElement with id %1")
								.arg(e.m_componentId), FUNC_ID);
		}
	}
	m_elementIds = networkModel.m_elementIds;

	// setup the enetwork
	try {
		m_p->setup(*networkModel.network(), nw.m_fluid);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error setting up flow network.", FUNC_ID);
	}

	// resize vectors
	m_n = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		m_n += fe->nInternalStates();
	}
	m_y.resize(m_n,0.0);
	m_meanTemperatureRefs.resize(m_p->m_flowElements.size(), nullptr);

	// initialize all fluid temperatures
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];

		double fluidTemp = m_network->m_para[NANDRAD::HydraulicNetwork::P_InitialFluidTemperature].value;
		fe->setInitialTemperature(fluidTemp);
		m_meanTemperatureRefs[i] = &fe->m_meanTemperature;
	}
}


void ThermalNetworkStatesModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {
	if(!resDesc.empty())
		resDesc.clear();
	QuantityDescription desc("FluidTemperature", "C", "Internal fluid temperature of network element", false);
	// adjust reference type
	desc.m_referenceType = NANDRAD::ModelInputReference::MRT_NETWORKELEMENT;
	// loop through all flow elements
	for(unsigned int i = 0; i < m_elementIds.size(); ++i) {
		desc.m_id = m_elementIds[i];
		resDesc.push_back(desc);
	}
}


const double * ThermalNetworkStatesModel::resultValueRef(const InputReference & quantity) const {
	const QuantityName & quantityName = quantity.m_name;
	// return y
	if(quantityName == std::string("y")) {
		// whole vector access
		if(quantityName.m_index == -1)
			return &m_y[0];
		return nullptr;
	}
	if(quantityName == std::string("FluidTemperature")) {
		if(quantity.m_referenceType != NANDRAD::ModelInputReference::MRT_NETWORKELEMENT)
			return nullptr;
		// access to an element temperature
		std::vector<unsigned int>::const_iterator fIt =
				std::find(m_elementIds.begin(), m_elementIds.end(),
						  (unsigned int) quantity.m_id);
		// invalid index access
		if(fIt == m_elementIds.end())
			return nullptr;
		unsigned int pos = (unsigned int) std::distance(m_elementIds.begin(), fIt);
		return m_meanTemperatureRefs[pos];
	}
	return nullptr;
}


unsigned int ThermalNetworkStatesModel::nPrimaryStateResults() const {
	return m_n;
}


void ThermalNetworkStatesModel::stateDependencies(std::vector<std::pair<const double *, const double *> > & /*resultInputValueReferences*/) const {
	// TODO: implement
}


void ThermalNetworkStatesModel::yInitial(double * y) {
	// set internal states
	unsigned int offset = 0;
	for(ThermalNetworkAbstractFlowElement* fe :m_p->m_flowElements) {
		fe->initialInternalStates(y + offset);
		offset += fe->nInternalStates();
	}
	// copy states
	std::memcpy(&m_y[0], y, m_n * sizeof(double));
}


int ThermalNetworkStatesModel::update(const double * y) {
	// copy states vector
	std::memcpy(&m_y[0], y, m_n*sizeof(double));
	// update zone temperatures
	for(unsigned int i = 0; i < m_zoneIdxs.size(); ++i) {
		// skip invalid elements without access to zone temperature
		unsigned int refIdx = m_zoneIdxs[i];
		if(refIdx == (unsigned int)(-1))
			continue;
		IBK_ASSERT(m_zoneTemperatureRefs[refIdx] != nullptr);
		m_heatExchangeValues[i] = *m_zoneTemperatureRefs[refIdx];
	}
	// set internal states
	unsigned int offset = 0;
	for(unsigned int i = 0; i < m_p->m_flowElements.size(); ++i) {
		ThermalNetworkAbstractFlowElement *fe = m_p->m_flowElements[i];
		// calculate internal enthalpies for all flow elements
		fe->setInternalStates(y + offset);
		// retrieve fluid temperatures
		unsigned int nStates = fe->nInternalStates();
		offset += nStates;
	}
	return 0;
}

int ThermalNetworkStatesModel::setTime(double t) {
	// update all spline values
	for(unsigned int i = 0; i < m_heatExchangeSplineRefs.size(); ++i) {
		// no spline
		if(m_heatExchangeSplineRefs[i] == nullptr)
			continue;
		m_heatExchangeValues[i] = m_heatExchangeSplineRefs[i]->value(t);
	}
	return 0;
}


} // namespace NANDRAD_MODEL

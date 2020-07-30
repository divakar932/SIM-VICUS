/*	The Nandrad model library.

Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 3 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
*/

#ifndef RoomStatesModelH
#define RoomStatesModelH

#include "NM_DefaultModel.h"

namespace NANDRAD {
	class Zone;
}

namespace NANDRAD_MODEL {

/*!	A model that computes zone states (temperature) out of a given energy density.

	The room state model updates all states from the y-vector and
	offers it for references from other models. This model is the
	complement to the RoomBalanceModel which evaluates all heat fluxes
	and calculate divergences(). As the RoomBalanceModel owns the
	integrator communication facility it also holds the vector of states y
	(internal energy). Using input references this quantity will be
	accessed using a constant value reference by the RoomStatesModel.
*/
class RoomStatesModel : public DefaultModel {
public:

	enum Results {
		R_AirTemperature,						// Keyword: AirTemperature				[C]		'Room air temperature.'
		NUM_R
	};
	enum InputReferences {
		InputRef_InternalEnergy,				// Keyword: InternalEnergy				[J]		'Internal energy of the room.'
		NUM_InputRef
	};

	/*! Constructor, relays ID to DefaultModel constructor. */
	RoomStatesModel(unsigned int id, const std::string &displayName) :
		DefaultModel(id, displayName)
	{
	}

	/*! Initializes model by providing zone parameters.
		The relevant parameters are cached in the RoomStatesModel results
		for access by other models.
		This function also checks for require parameters and tests for validity of values.
	*/
	void setup(const NANDRAD::Zone & zone);


	// *** Re-implemented from AbstractModel

	/*! Room state model can be referenced via Zone and ID. */
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const override {
		return NANDRAD::ModelInputReference::MRT_ZONE;
	}

	/*! Return unique class ID name of implemented model. */
	virtual const char * ModelIDName() const override { return "RoomStatesModel"; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model. */
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const override;

	/*! Retrieves reference pointer to a value with given quantity ID name.
		\return Returns pointer to memory location with this quantity, otherwise NULL if parameter ID was not found.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const override;

	/*! Resizes m_results vector.*/
	virtual void initResults(const std::vector<AbstractModel*> & models) override;


	// *** Other public member functions

	/*! Returns the underlying zone. */
	const NANDRAD::Zone * zone() const {	return m_zone; }

	/*! Composes all input references.*/
	void initInputReferences(const std::vector<AbstractModel*> & models);

	/*! Sets initial states in y vector.
		This function is called after setup(), so that parameters needed for
		computing the initial condition are already present.
	*/
	void yInitial(double * y) const;

	/*! Computes room air temperature. */
	int update();

protected:

	/*! Characteristics of fluid (only type air is allowed). Note that fluid must be
		referenced by an independent FluidModel.
	*/
//	const FluidModel								*m_fluid;
	/*! Constant pointer to the referenced zone. */
	const NANDRAD::Zone								*m_zone = nullptr;
};

} // namespace NANDRAD_MODEL

#endif // RoomStatesModelH

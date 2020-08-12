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

#ifndef AbstractModelH
#define AbstractModelH

#include <NANDRAD_ModelInputReference.h>

#include "NM_QuantityDescription.h"
#include "NM_QuantityName.h"

namespace NANDRAD_MODEL {

/*!	This is the base class of all models (that generate results).
	A model should define a unqiue ModelIDName and provide result quantities. For each model
	instance a unqiue id number should be reserved.
*/
class AbstractModel {
public:

	/*! Virtual destructor.
		\note Definition of abstractor code is in cpp file, which serves as 'first translation unit'
		and also stores virtual function table. This speeds up compilation (and possibly execution),
		since otherwise this vft is placed and compiled and linked in *every* class deriving from
		AbstractModel.
	*/
	virtual ~AbstractModel();

	/*! Returns a reference type which in conjunction with the model ID allows identification
		of model object(s), generated from the same parametrization block.
		This can be a single model object in some cases, but in others (e.g. Zone) there will
		be several objects with the same reference type (e.g. MRT_ZONE) and the same zone ID.
		Among these objects, all published variables must have unique names!

		This function must be re-implemented by model objects that provide result variables.
		Default implementation returns invalid reference type.
		\sa NANDRAD::ModelInputReference::referenceType_t
	*/
	virtual NANDRAD::ModelInputReference::referenceType_t referenceType() const { return NANDRAD::ModelInputReference::NUM_MRT; }

	/*! Returns a unique class ID name of the implemented model for debugging/output purposes.
		Should be implemented by derived models.
		Default implementation returns an empty string.
	*/
	virtual const char * ModelIDName() const { return ""; }

	/*! Returns unique ID of this abstract state model.
		Must be implemented by derived models that generate results.
		Default implementation returns 0.
	*/
	virtual unsigned int id() const { return 0; }

	/*! Returns display name of this abstract state model.
		Should be implemented by derived models.
		Default implementation returns an empty string.
	*/
	virtual const char * displayName() const { return ""; }

	/*! Populates the vector resDesc with descriptions of all results provided by this model.
		Must be implemented by derived models that generate results.
		Default implementation does nothing (i.e. no results generated).

		\warning This function generates and populates the vector refDesc from scratch and
			may not be the fastest. If you need to access the reference description several
			times, consider caching the resulting vector.
	*/
	virtual void resultDescriptions(std::vector<QuantityDescription> & resDesc) const { (void)resDesc; }

	/*! Returns vector of all scalar and vector valued results pointer.
		\param res Vector with pointers to resulting values.

		\todo Clarify, when this is needed...for iterative solution of cycles?

		Default implementation does nothing (i.e. no results generated).
	*/
	virtual void resultValueRefs(std::vector<const double *> &res) const { (void)res; }

	/*! Looks up a result quantity and returns a pointer to the double value memory location.
		Must be implemented by derived models that generate results.
		Default implementation returns a nullptr (i.e. variable does not exist).

		This memory location must be, once reported to the framework, persistant throughout
		the lifetime of the model object.

		\return A const pointer to the double, or nullptr of no such quantity was found.
		\note If a vector quantity is requested without indication of an index, the pointer returned points to
			the begin of the memory array holding the vector data.
	*/
	virtual const double * resultValueRef(const QuantityName & quantityName) const { (void)quantityName; return nullptr; }

	/*! Resizes m_results and m_vectorValuedResults vectors.
		This function is called when all models are already initialized (constructed).

		Must be implemented by derived models that generate results.
		Default implementation does nothing.

		When re-implementing this class, make sure that m_results and
		m_vectorValuedResults a fully resized and defined. Ensure, that the functions
		resultDescriptions() and resultValueRef() can be called afterwards.

		The default implementation uses the keyword list and expects the
		enumerations Variables to be present in derived classes. It
		resizes and fills the m_results vectors. It then initializes
		IBK::Parameters with name and unit.

		Suggested default procedure when re-implementing this function:

		Re-implement this function and call DefaultModel::initResults().
		Resize all vector valued results (m_vectorValuedResults).
	*/
	virtual void initResults(const std::vector<AbstractModel*> & /* models */) {}
};


} // namespace NANDRAD_MODEL

#endif // AbstractModelH

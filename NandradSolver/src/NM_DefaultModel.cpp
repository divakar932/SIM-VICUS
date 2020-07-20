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

#include "NM_DefaultModel.h"
#include "NM_KeywordList.h"

#include <NANDRAD_KeywordList.h>

#include <algorithm>
#include <limits>
#include <stddef.h>

#include <IBK_assert.h>

namespace NANDRAD_MODEL {

const unsigned int DefaultModel::InvalidVectorIndex = std::numeric_limits<unsigned int>::max();

void DefaultModel::initResults(const std::vector<AbstractModel*> & /* models */) {
	// retreive all result quantities from quantity description list
	std::vector<QuantityDescription> resDesc;

	resultDescriptions(resDesc);
	for (unsigned int i = 0; i < resDesc.size(); ++i) {

		const QuantityDescription &desc = resDesc[i];
		// skip constant descriptions (no quantity is necessary)
		if (desc.m_constant)
			continue;

		if (desc.m_size == 1 && desc.m_indexKeys.empty()) {
			// scalar results are stored as IBK::Parameter
			m_results.push_back(IBK::Parameter(
				desc.m_name,
				0,
				desc.m_unit));
		}
		else  {
			// vector valued results are stored as IBK::UnitList
			VectorValuedQuantity quantity(desc.m_name,
				IBK::Unit(desc.m_unit));
			// resize quantity
			if (!desc.m_indexKeys.empty()) {
				const std::vector<VectorValuedQuantityIndex> &indexKeys =
					desc.m_indexKeys;
				// tralste into vector
				std::set<unsigned int> keys;
				for (unsigned int i = 0; i < indexKeys.size(); ++i) {
					keys.insert(indexKeys[i].m_keyValue);
				}
				quantity.resize(keys, indexKeys.front().m_keyType);
			}
			// store result
			m_vectorValuedResults.push_back(quantity);
		}
	}
	// After call of this routine all result vectors are prepared.
	// All vector valued result quantities must now be resized from
	// the inherited model itself. Prepare a set of indices either containing
	// numbers or IDs and use this set for resizing.
}

void DefaultModel::resultDescriptions(std::vector<QuantityDescription> & resDesc) const {

	const char * const FUNC_ID = "[DefaultModel::resultDescriptions]";
	// fill result discriptions with informations from the keyword list
	std::string category = std::string(ModelIDName()) + "::Results";

	try {
		if(KeywordList::CategoryExists(category.c_str()) ) {
			for (unsigned int varIndex = 0; varIndex <= (unsigned int) KeywordList::MaxIndex(category.c_str()); ++varIndex) {
				bool constant = false;
				resDesc.push_back( QuantityDescription(
					KeywordList::Keyword( category.c_str(), varIndex ),
					KeywordList::Unit( category.c_str(), varIndex ),
					KeywordList::Description( category.c_str(), varIndex ),
					constant) );
			}
		}
		// The m_vectorValuedResults vector may either be empty or filled already. If
		// the vector is filled it contains all information about the vector elements
		// including indices that are occupied.
		category = std::string(ModelIDName()) + "::VectorValuedResults";
		if(KeywordList::CategoryExists(category.c_str()) ) {
			for (unsigned int varIndex = 0; varIndex <= (unsigned int) KeywordList::MaxIndex(category.c_str()); ++varIndex) {
				bool constant = false;
				// retreive index information from vector valued results
				std::vector<VectorValuedQuantityIndex> indexKeys;
				std::vector<std::string> indexKeyDescriptions;
				// store name, unit and description of the vector quantity
				const std::string &quantityName = KeywordList::Keyword( category.c_str(), varIndex );
				const std::string &quantityUnit = KeywordList::Unit( category.c_str(), varIndex );
				const std::string &quantityDescription = KeywordList::Description( category.c_str(), varIndex );
				// vector valued quantity descriptions store the description
				// of the quantity itself as well as key strings and descriptions
				// for all vector elements
				resDesc.push_back( QuantityDescription(
					quantityName, quantityUnit,quantityDescription,
					constant,indexKeys, indexKeyDescriptions) );
			}
		}
	}
	catch(IBK::Exception &ex)
	{
		throw IBK::Exception(ex, IBK::FormatString("Error initializing input reference description for Model #%1 with id #%2")
			.arg(ModelIDName()).arg(id()), FUNC_ID);
	}
}


void DefaultModel::resultValueRefs(std::vector<const double *> &res) const {

	res.clear();
	// fill with all results and vector valued results

	for (unsigned int i = 0; i < m_results.size(); ++i) {
		res.push_back(&m_results[i].value);
	}

	for (unsigned int i = 0; i < m_vectorValuedResults.size(); ++i) {
		// loop over all vector valued results
		for(std::vector<double>::const_iterator valueIt =
			m_vectorValuedResults[i].begin();
			valueIt != m_vectorValuedResults[i].end();
			++valueIt)
			res.push_back(&(*valueIt));
	}

}

const double * DefaultModel::resultValueRef(const QuantityName & quantityName) const {
	//const char * const FUNC_ID ="[DefaultModel::resultValueRef]";

	try {
		// find corresponding quantity description
		int quantity = decodeResultType(quantityName.name());
		// scalar results
		if (quantity != -1 && quantityName.index() == -1) {
			if (quantity >= (int)m_results.size())
				return NULL;

			// wrong definition
			IBK_ASSERT_XX(quantityName.index() == -1,
				IBK::FormatString("Invalid index definition for scalar quantity %1.")
				.arg(quantityName.name()));

			return &m_results[quantity].value;
		}

		// a vector valued result
		quantity = decodeVectorValuedResultType(quantityName.name());

		// inbvylid quantity
		if (quantity == -1)
			return NULL;

		if (quantity >= (int)m_vectorValuedResults.size())
			return NULL;

		// no index is given
		if (quantityName.index() == -1) {
			// return access to the first vector element
			if (!m_vectorValuedResults[quantity].empty())
				return &m_vectorValuedResults[quantity].m_data[0];
			return NULL;
		}
		// index definition
		else {
			std::vector<double>::const_iterator vecElem =
				m_vectorValuedResults[quantity].find(quantityName.index());
			// return access to the requested vector element
			if (vecElem != m_vectorValuedResults[quantity].end())
				return &(*vecElem);
			return NULL;
		}
	}
	catch (IBK::Exception) {
		return NULL;
	}
}


int DefaultModel::decodeResultType(
	const std::string &quantity) const
{
	//const char * const FUNC_ID = "[DefaultStateDependency::decodeResultType]";

	// first check results
	std::string category = ModelIDName() + std::string("::Results");

	if (KeywordList::CategoryExists(category.c_str()) &&
		KeywordList::KeywordExists(category.c_str(), quantity))

		return (int)KeywordList::Enumeration(category.c_str(), quantity);

	// find corresponding quantity description
	std::vector<QuantityDescription> resDesc;
	resultDescriptions(resDesc);

	// check if we have a quantity defined
	std::vector<QuantityDescription>::iterator resDescIt =
		std::find(resDesc.begin(), resDesc.end(), quantity);
	// no quantity defined
	if (resDescIt == resDesc.end()) {
		return -1;
	}


	// vector results are listed in another routine
	if (resDescIt->m_size != 1 || !resDescIt->m_indexKeys.empty())
		return -1;

	int quantityType = (int) (resDescIt - resDesc.begin());

	return quantityType;
}


int DefaultModel::decodeVectorValuedResultType(
	const std::string &quantity) const
{
	//const char * const FUNC_ID = "[DefaultStateDependency::decodeResultType]";

	// first check results
	std::string category = ModelIDName() + std::string("::VectorValuedResults");

	if (KeywordList::CategoryExists(category.c_str()) &&
		KeywordList::KeywordExists(category.c_str(), quantity))

		return (int)KeywordList::Enumeration(category.c_str(), quantity);

	// find corresponding quantity description inside description vector
	std::vector<QuantityDescription> resDesc;
	resultDescriptions(resDesc);

	// check if we have a quantity defined
	std::vector<QuantityDescription>::iterator resDescIt =
		std::find(resDesc.begin(), resDesc.end(), quantity);
	// no quantity defined
	if (resDescIt == resDesc.end()) {
		return -1;
	}

	/// a scalör result
	if (resDescIt->m_size == 1 && resDescIt->m_indexKeys.empty()) {
		return -1;
	}

	int quantityType = (int)(resDescIt - resDesc.begin());
	// vector results are listed after scalar results
	quantityType -= (int)m_results.size();

	return quantityType;
}


} // namespace NANDRAD_MODEL


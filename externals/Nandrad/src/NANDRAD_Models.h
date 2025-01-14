/*	The NANDRAD data model library.

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

#ifndef NANDRAD_ModelsH
#define NANDRAD_ModelsH

#include <vector>

#include "NANDRAD_NaturalVentilationModel.h"
#include "NANDRAD_InternalLoadsModel.h"
#include "NANDRAD_ShadingControlModel.h"

namespace NANDRAD {

/*! A container class for all models. */
class Models {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Verifies that all models have different IDs. */
	void checkForUniqueIDs() const;

	/*! Container for all natural ventilation models. */
	std::vector<NaturalVentilationModel>	m_naturalVentilationModels;
	/*! Container for all internal loads models. */
	std::vector<InternalLoadsModel>			m_internalLoadsModels;
	/*! Container for all shading control models. */
	std::vector<ShadingControlModel>		m_shadingControlModels;
};

} // namespace NANDRAD

#endif // NANDRAD_ModelsH

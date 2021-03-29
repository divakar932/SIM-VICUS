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

#include "NANDRAD_FMIDescription.h"

namespace NANDRAD {


void FMIDescription::writeModelDescription(const IBK::Path & modelDesc, const std::string & modelDescTemplate) const {
	// use template with placeholders

	// placeholders to substitute:
	// ${MODELNAME} - Projekt file base name with replaced whitespaces
	// ${NANDRAD_VERSION}
	// ${DATETIME}
	// ${SIMDURATION}
	// ${MODELVARIABLES}
	// ${MODEL_STRUCTURE_OUTPUTS}

}


} // namespace NANDRAD

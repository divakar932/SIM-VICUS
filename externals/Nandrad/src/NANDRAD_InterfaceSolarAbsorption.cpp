/*	The NANDRAD data model library.
Copyright (c) 2012, Institut fuer Bauklimatik, TU Dresden, Germany

Written by
A. Nicolai		<andreas.nicolai -[at]- tu-dresden.de>
A. Paepcke		<anne.paepcke -[at]- tu-dresden.de>
St. Vogelsang	<stefan.vogelsang -[at]- tu-dresden.de>
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

#include "NANDRAD_InterfaceSolarAbsorption.h"

namespace NANDRAD {

bool InterfaceSolarAbsorption::operator!=(const InterfaceSolarAbsorption & other) const {
	// model comparison
	if (m_modelType != other.m_modelType) return true;
	for (int i=0; i<NUM_P; ++i)
		if (m_para[i] != other.m_para[i]) return true;
	return false; // not different
}

} // namespace NANDRAD


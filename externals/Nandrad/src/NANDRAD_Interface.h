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

#ifndef NANDRAD_InterfaceH
#define NANDRAD_InterfaceH

#include <IBK_Parameter.h>
#include <IBK_Flag.h>

#include "NANDRAD_InterfaceAirFlow.h"
#include "NANDRAD_InterfaceHeatConduction.h"
#include "NANDRAD_InterfaceSolarAbsorption.h"
#include "NANDRAD_InterfaceLongWaveEmission.h"
#include "NANDRAD_InterfaceVaporDiffusion.h"
#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"

namespace NANDRAD {

class Zone;

/*!	An Interface identifies a surface of a wall and stores all data that are needed for boundary
	condition calculation. Its position at the wall is identified by the location_t-attribute, either
	A or B side of the construction (A - layer index 0, B - other side).

	The zone at the other side of the wall surface is identified by its zone ID.
	The zone ID = 0 indicates ambient climate. If the construction is adiabatic,
	there must not be an interface defined for this side.
*/
class Interface {
public:

	enum location_t {
		IT_A = 0,		// Keyword: A				'Interface is situated at left side labeled A.'
		IT_B,			// Keyword: B				'Interface is situated at right side labeled B.'
		NUM_IT
	};

	/*! Boundary condition parameter blocks. */
	enum condition_t {
		IP_HEATCONDUCTION,		// Keyword: HeatConduction			'Heat conduction boundary condition.'
		IP_SOLARABSORPTION,		// Keyword: SolarAbsorption			'Short-wave solar absorption boundary condition.'
		IP_LONGWAVEEMISSION,	// Keyword: LongWaveEmission		'Long wave emission (and counter radiation) boundary condition.'
		IP_VAPORDIFFUSION,		// Keyword: VaporDiffusion			'Vapor diffusion boundary condition.'
		NUM_IP
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	/*! Special form of comparison operator, tests if parameters that have an
		impact on result calculation are the same (zone, location, physical parameters).
	*/
	bool behavesLike(const Interface & other) const {
		return (m_location == other.m_location &&
				m_zoneId == other.m_zoneId &&
				m_heatConduction == other.m_heatConduction &&
				m_solarAbsorption == other.m_solarAbsorption &&
				m_longWaveEmission == other.m_longWaveEmission &&
				m_vaporDiffusion == other.m_vaporDiffusion);
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! ID of the referenced surface/interface. */
	unsigned int								m_id = INVALID_ID;		// XML:A:required
	/*! The position of the interface, left ore right of the construction. */
	location_t									m_location = IT_A;		// XML:A
	/*! The id number of the neighboring zone. */
	unsigned int								m_zoneId = 0;			// XML:A

	/*! Enables the interface models. */
	IBK::Flag									m_condition[NUM_IP];	// XML:E

	// Boundary condition parameters
	/*! Model for heat transfer coefficient. */
	InterfaceHeatConduction						m_heatConduction;		// XML:E
	/*! Model for solar absorption coefficient. */
	InterfaceSolarAbsorption					m_solarAbsorption;		// XML:E
	/*! Model for long wave emissivity. */
	InterfaceLongWaveEmission					m_longWaveEmission;		// XML:E
	/*! Model for vapor diffusion. */
	InterfaceVaporDiffusion						m_vaporDiffusion;		// XML:E
	/*! Model for air flow calculation. */
	InterfaceAirFlow							m_airFlow;				// XML:E

	/*! Comment, indicating the zone this interface links to.
		\warning This comment is set automatically when the project is written
				 with Project::writeXML(). Any existing text will be overwritten.
	*/
	std::string									m_comment;				// XML:C

	// *** Variables used only during simulation ***

	/*! Reference to neighbor zone.*/
	const Zone*									m_zoneRef = nullptr;
};

} // namespace NANDRAD

#endif // InterfaceH

#ifndef VICUS_BuildingLevelH
#define VICUS_BuildingLevelH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_Room.h"
#include "VICUS_Object.h"

#include <QString>

namespace VICUS {

class BuildingLevel : public Object {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	void updateParents() {
		m_children.clear();
		for (Room & s : m_rooms) {
			s.m_parent = this;
			m_children.push_back(&s);
			s.updateParents();
		}
	}

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Persistant ID of building level. */
	unsigned int						m_id = INVALID_ID;			// XML:A:required

	QString								m_displayName;				// XML:A

	/*! The nominal elevation [m] of the floor's surface above ground. */
	double								m_elevation = 0;			// XML:E

	/*! The nominal height [m] (floor surface to ceiling) of the building level. */
	double								m_height = 2.7;				// XML:E

	/*! Stores visibility information for this surface.
		Note: keep the next line - this will cause the code generator to create serialization code
			  for the inherited m_visible variable.
	*/
	//:inherited	bool								m_visible = true;			// XML:A

	/*! Vector of all rooms in a building level. */
	std::vector<Room>					m_rooms;					// XML:E
};

} // namespace VICUS


#endif // VICUS_BuildingLevelH

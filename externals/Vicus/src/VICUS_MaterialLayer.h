#ifndef VICUS_MaterialLayerH
#define VICUS_MaterialLayerH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"

#include <QString>

#include <vector>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>

namespace VICUS {

/* TODO : replace with NANDRAD::MaterialLayer */
class MaterialLayer {
public:

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Default c'tor. */
	MaterialLayer() {}

	/*! Simple Constructor with thickness in [m] and material id. */
	MaterialLayer(double thickness, unsigned int id):
		m_matId(id),
		m_thickness(IBK::Parameter("Thickness", thickness, "m"))
	{}

	/*! Simple Constructor with thickness and material id. */
	MaterialLayer(IBK::Parameter thickness, unsigned int id):
		m_matId(id),
		m_thickness(thickness)
	{}


	VICUS_READWRITE

	/*! Inequality operator. */
	bool operator!=(const MaterialLayer & other) const { return (m_thickness != other.m_thickness || m_matId != other.m_matId);	}
	/*! Equality operator. */
	bool operator==(const MaterialLayer & other) const { return !operator!=(other); }

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of material. */
	unsigned int					m_matId = INVALID_ID;	// XML:A:required

	/*! Thickness of the material layer. */
	IBK::Parameter					m_thickness;			// XML:E:required

	/*! Active for calculation in thermal simulation. */
	/// TODO Andreas :  wollen wir das integrieren? An NANDRAD wird dann eine Konstruktion übergeben in der alle
	/// false layers nicht vorhanden sind. Hätte zur Folge das bei rein thermischen Berechnung die Simu
	/// schneller geht und das im Bericht später die realistische Konstruktion abgebildet ist.
	/// dort würde alle Layer nämlich gebraucht werden
	bool							m_isActive = true;		// XML:A


};

} // namespace VICUS


#endif // VICUS_MaterialLayerH

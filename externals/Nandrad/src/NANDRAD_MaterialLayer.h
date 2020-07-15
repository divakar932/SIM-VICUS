#ifndef NANDRAD_MaterialLayerH
#define NANDRAD_MaterialLayerH

#include "NANDRAD_CodeGenMacros.h"

namespace NANDRAD {

/*! A layer of a multi-layered construction. */
class MaterialLayer {
public:
	/*! Default c'tor. */
	MaterialLayer() {}

	/*! Simple Constructor with thickness in m and id. */
	MaterialLayer(double thickness, unsigned int id):
		m_thickness(thickness),
		m_matId(id)
	{}

	NANDRAD_READWRITE

	/*! Inequality operator. */
	bool operator!=(const MaterialLayer & other) const { return (m_thickness != other.m_thickness || m_matId != other.m_matId);	}
	/*! Equality operator. */
	bool operator==(const MaterialLayer & other) const { return !operator!=(other); }

	/*! Thickness in m. */
	double					m_thickness;				// XML:A

	/*! Material id. */
	unsigned int			m_matId;					// XML:A
};

} // namespace NANDRAD

#endif // NANDRAD_MaterialLayerH

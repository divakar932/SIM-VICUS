#ifndef PickObjectH
#define PickObjectH

#include <IBKMK_Vector3D.h>
#include <IBK_bitfield.h>

#include <QPoint>

namespace Vic3D {

/*! An object to hold information on what to pick and also store the result.
	Basically, whenever you ask the scene to pick a point (after a mouse click, or if the mouse
	cursor has moved), configure such a pick object and call Scene::pick(obj).

	The pick object will now process all objects in the scene and for all activated for picking
	perform a collision test with the line-of-sight. All objects/planes that are hit by the
	line-of-sight are stored in the pick result candidate vector m_candidates.

	Afterwards you can access the data. If you are only interested in the nearest point, you
	can use the convenience function nearestHit() to retrieve the hit closest to the near plane.

*/
struct PickObject {

	/*! Identifies the type of snap point found. */
	enum ResultType {
		RT_Object,
		RT_GlobalXYPlane,
		RT_LocalXYPlane,
		/*! Intersection point with local axis if only one axis is locked. */
		RT_LocalPlaneFixedAxis,
		/*! Lotpunkt on free axis if two axes are locked. */
		RT_LocalFixedAxis,
		RT_NetworkNode,
		RT_NetworkEdge
	};

	/*! Stores information about a particular snap point candidate. */
	struct PickResult {
		bool operator<(const PickResult & other) const {
			return m_depth < other.m_depth;
		}

		ResultType		m_snapPointType;
		/*! Scale factor to be used in line-of-sight equation to get the plumb point (Lotpunkt). */
		double			m_depth;

		/*! Stores the unique ID of the clicked-on object, only for m_snapPointType == SP_Object or SP_Network*** . */
		unsigned int	m_uniqueObjectID = 0;

		/*! Coordinates of the picked point. */
		IBKMK::Vector3D	m_pickPoint;

	};


	PickObject(const QPoint & localMousePos) :
		m_pickPerformed(false), m_localMousePos(localMousePos)
	{
	}

	/*! Set to true, if picking was already performed. */
	bool					m_pickPerformed;

	/*! The local mouse position. */
	QPoint					m_localMousePos;

	/*! Here we store all possible pick candidates.
		Added will be only those, whose depths is between 0 and 1 (line-of-sight factor).
	*/
	std::vector<PickResult>	m_candidates;
};


} // namespace Vic3D

#endif // PickObjectH

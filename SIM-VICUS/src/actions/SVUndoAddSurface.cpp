#include "SVUndoAddSurface.h"
#include "SVProjectHandler.h"

SVUndoAddSurface::SVUndoAddSurface(const QString & label, const VICUS::Surface & addedSurface,
								   unsigned int parentNodeID, const VICUS::ComponentInstance * compInstance) :
	m_addedSurface(addedSurface),
	m_parentNodeID(parentNodeID)
{
	setText( label );
	if (compInstance != nullptr)
		m_componentInstance = *compInstance;
}


void SVUndoAddSurface::undo() {

	// find room, if given
	if (m_parentNodeID != 0) {
		// find the parent room node (it must be a room node!)
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(theProject().objectById(m_parentNodeID));
		Q_ASSERT(r != nullptr);
		// remove previously added surface
		Q_ASSERT(!r->m_surfaces.empty());
		const_cast<VICUS::Room*>(r)->m_surfaces.pop_back();
	}
	else {
		Q_ASSERT(!theProject().m_plainGeometry.empty());
		theProject().m_plainGeometry.pop_back();
	}
	if (m_componentInstance.m_componentID != VICUS::INVALID_ID) {
		Q_ASSERT(!theProject().m_componentInstances.empty());
		theProject().m_componentInstances.pop_back();
	}

	// tell project that the geometry has changed (i.e. rebuild navigation tree and scene)
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


void SVUndoAddSurface::redo() {
	// find room, if given
	if (m_parentNodeID != 0) {

		// find the parent room node (it must be a room node!)
		const VICUS::Room * r = dynamic_cast<const VICUS::Room *>(theProject().objectById(m_parentNodeID));
		Q_ASSERT(r != nullptr);
		// add surface to room surfaces
		const_cast<VICUS::Room*>(r)->m_surfaces.push_back(m_addedSurface);
	}
	else {
		// add to anonymous geometry
		theProject().m_plainGeometry.push_back(m_addedSurface);
	}

	if (m_componentInstance.m_componentID != VICUS::INVALID_ID) {
		theProject().m_componentInstances.push_back(m_componentInstance);
	}
	theProject().updatePointers();

	// tell project that the network has changed
	SVProjectHandler::instance().setModified( SVProjectHandler::BuildingGeometryChanged);
}


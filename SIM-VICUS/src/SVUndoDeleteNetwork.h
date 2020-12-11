#ifndef SVUNDODELETENETWORK_H
#define SVUNDODELETENETWORK_H

#include "SVUndoCommandBase.h"

class SVUndoDeleteNetwork: public SVUndoCommandBase {
		Q_DECLARE_TR_FUNCTIONS(SVUndoDeleteNetwork)
	public:
		SVUndoDeleteNetwork(const QString & label, const VICUS::Network & deletedNetwork);

		virtual void undo();
		virtual void redo();

	private:

		/*! Cache for added network. */
		VICUS::Network	m_deletedNetwork;

		double			m_gridWidth;
		double			m_gridSpacing;
		double			m_farDistance;
};

#endif // SVUNDODELETENETWORK_H
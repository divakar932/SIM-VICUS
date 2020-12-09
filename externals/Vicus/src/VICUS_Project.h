/*	The SIM-VICUS data model library.

	Copyright (c) 2020-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Andreas Nicolai  <andreas.nicolai -[at]- tu-dresden.de>
	  ... all the others ... :-)

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

#ifndef VICUS_ProjectH
#define VICUS_ProjectH

#include <vector>

#include <IBK_Path.h>

#include <NANDRAD_Project.h>

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Network.h"
#include "VICUS_Building.h"
#include "VICUS_ViewSettings.h"
#include "VICUS_NetworkFluid.h"
#include "VICUS_NetworkPipe.h"

namespace VICUS {

class Project {
	VICUS_READWRITE
public:

	enum ViewFlags {
		VF_All,			// Keyword: All
		NUM_VF
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	/*! Constructor, creates dummy data. */
	Project();

	/*! Parses only the header of the XML file.
		This function is supposed to be fast, yet not a complete XML parser.
		\param filename  The full path to the project file.
	*/
	void parseHeader(const IBK::Path & filename);

	/*! Reads the project data from an XML file.
		\param filename  The full path to the project file.
	*/
	void readXML(const IBK::Path & filename);

	/*! Writes the project file to an XML file.
		\param filename  The full path to the project file.
	*/
	void writeXML(const IBK::Path & filename) const;

	/*! Removes un-referenced/un-needed data structures. */
	void clean();

	/*! Call this function whenever project data has changed that depends on
		objects linked through pointers (building hierarchies, networks etc.).
	*/
	void updatePointers();

	/*! Searches through all unique id-objects in project structure for the uniqueID.
		Throws an exception, if no object with this unique ID can be found.
	*/
	const VICUS::Object * objectById(unsigned int uniqueID) const;

	/*! This function checks all surfaces in the project if they are selected or not.
		\returns Returns true, if any surface is selected, and if so, stores the arithmetic average of all
				 surface vertexes in variable centerPoint.
	*/
	bool haveSelectedSurfaces(IBKMK::Vector3D & centerPoint) const;

	// *** FUNCTIONS ***

	/*! Function to find an element by ID. */
	template <typename T>
	static T * element(std::vector<T>& vec, unsigned int id) {
		typename std::vector<T>::iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
	}

	/*! Function to find an element by ID (const-version). */
	template <typename T>
	static const T * element(const std::vector<T>& vec, unsigned int id) {
		typename std::vector<T>::const_iterator it = std::find(vec.begin(), vec.end(), id);
		if (it == vec.end())
			return nullptr;
		else
			return &(*it);
	}

	/*! Function to generate unique ID */
	template <typename T>
	static unsigned uniqueId(std::vector<T>& vec) {
		for (unsigned id=0; id<std::numeric_limits<unsigned>::max(); ++id){
			if (std::find(vec.begin(), vec.end(), id) == vec.end())
				return id;
		}
		// TODO: Hauke: discuss with Andreas
	}

	/*! Function to generate unique ID (const-version). */
	template <typename T>
	static unsigned uniqueId(const std::vector<T>& vec) {
		for (unsigned id=0; id<std::numeric_limits<unsigned>::max(); ++id){
			if (std::find(vec.begin(), vec.end(), id) == vec.end())
				return id;
		}
		// TODO: Hauke: discuss with Andreas
	}


	// *** PUBLIC MEMBER VARIABLES ***

	/*! Project info tag is manually written (not by code generator), to be located directly under root-node
		and not inside Project node.
	*/
	NANDRAD::ProjectInfo								m_projectInfo;

	ViewSettings										m_viewSettings;				// XML:E

	std::vector<Network>								m_geometricNetworks;		// XML:E

	std::vector<Building>								m_buildings;				// XML:E

	/*! Vector with plain (dumb) geometry. */
	std::vector<Surface>								m_plainGeometry;			// XML:E


	// *** Database elements used in the project (normally stored in built-in and user databases)
	//     These database elements need to be merged with program databased when project is read.

	/*! Database of fluids */
	std::vector<NetworkFluid>							m_networkFluids;			// XML:E

};


} // namespace VICUS

#endif // VICUS_ProjectH

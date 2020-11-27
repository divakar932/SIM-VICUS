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

#include <VICUS_NetworkEdge.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkEdge::readXML(const TiXmlElement * element) {
	FUNCID(NetworkEdge::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "nodeId1"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'nodeId1' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "nodeId2"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'nodeId2' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "nodeId1")
				m_nodeId1 = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "nodeId2")
				m_nodeId2 = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Length")
				m_length = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "DiameterInside")
				m_diameterInside = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "DiameterOutside")
				m_diameterOutside = NANDRAD::readPODElement<double>(c, cName);
			else if (cName == "Supply")
				m_supply = NANDRAD::readPODElement<bool>(c, cName);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkEdge' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkEdge' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkEdge::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkEdge");
	parent->LinkEndChild(e);

	if (m_nodeId1 != VICUS::INVALID_ID)
		e->SetAttribute("nodeId1", IBK::val2string<unsigned int>(m_nodeId1));
	if (m_nodeId2 != VICUS::INVALID_ID)
		e->SetAttribute("nodeId2", IBK::val2string<unsigned int>(m_nodeId2));
	TiXmlElement::appendSingleAttributeElement(e, "Length", nullptr, std::string(), IBK::val2string<double>(m_length));
	TiXmlElement::appendSingleAttributeElement(e, "DiameterInside", nullptr, std::string(), IBK::val2string<double>(m_diameterInside));
	TiXmlElement::appendSingleAttributeElement(e, "DiameterOutside", nullptr, std::string(), IBK::val2string<double>(m_diameterOutside));
	TiXmlElement::appendSingleAttributeElement(e, "Supply", nullptr, std::string(), IBK::val2string<bool>(m_supply));
	return e;
}

} // namespace VICUS
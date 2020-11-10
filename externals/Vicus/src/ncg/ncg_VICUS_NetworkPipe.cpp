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

#include <VICUS_NetworkPipe.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>

#include <tinyxml.h>

namespace VICUS {

void NetworkPipe::readXML(const TiXmlElement * element) {
	FUNCID(NetworkPipe::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "dOutside"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'dOutside' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "sWall"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'sWall' attribute.") ), FUNC_ID);

		if (!TiXmlAttribute::attributeByName(element, "roughness"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'roughness' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = attrib->ValueStr();
			else if (attribName == "dOutside")
				m_dOutside = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "sWall")
				m_sWall = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else if (attribName == "roughness")
				m_roughness = NANDRAD::readPODAttributeValue<double>(element, attrib);
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'NetworkPipe' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'NetworkPipe' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * NetworkPipe::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("NetworkPipe");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.empty())
		e->SetAttribute("displayName", m_displayName);
	e->SetAttribute("dOutside", IBK::val2string<double>(m_dOutside));
	e->SetAttribute("sWall", IBK::val2string<double>(m_sWall));
	e->SetAttribute("roughness", IBK::val2string<double>(m_roughness));
	return e;
}

} // namespace VICUS

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

#include <VICUS_Component.h>
#include <VICUS_KeywordList.h>

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <VICUS_Constants.h>
#include <NANDRAD_Utilities.h>
#include <VICUS_Constants.h>
#include <VICUS_KeywordList.h>

#include <tinyxml.h>

namespace VICUS {

void Component::readXML(const TiXmlElement * element) {
	FUNCID(Component::readXML);

	try {
		// search for mandatory attributes
		if (!TiXmlAttribute::attributeByName(element, "id"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'id' attribute.") ), FUNC_ID);

		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "id")
				m_id = NANDRAD::readPODAttributeValue<unsigned int>(element, attrib);
			else if (attribName == "displayName")
				m_displayName = QString::fromStdString(attrib->ValueStr());
			else if (attribName == "color")
				m_color.setNamedColor(QString::fromStdString(attrib->ValueStr()));
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}
		// search for mandatory elements
		if (!element->FirstChildElement("Type"))
			throw IBK::Exception( IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Missing required 'Type' element.") ), FUNC_ID);

		// reading elements
		const TiXmlElement * c = element->FirstChildElement();
		while (c) {
			const std::string & cName = c->ValueStr();
			if (cName == "Manufacturer")
				m_manufacturer = QString::fromStdString(c->GetText());
			else if (cName == "DataSource")
				m_dataSource = QString::fromStdString(c->GetText());
			else if (cName == "IdOpaqueConstruction")
				m_idOpaqueConstruction = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdGlazingSystem")
				m_idGlazingSystem = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdOutsideBoundaryCondition")
				m_idOutsideBoundaryCondition = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdInsideBoundaryCondition")
				m_idInsideBoundaryCondition = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "IdSurfaceProperty")
				m_idSurfaceProperty = NANDRAD::readPODElement<unsigned int>(c, cName);
			else if (cName == "Type") {
				try {
					m_type = (CompontType)KeywordList::Enumeration("Component::CompontType", c->GetText());
				}
				catch (IBK::Exception & ex) {
					throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(c->Row()).arg(
						IBK::FormatString("Invalid or unknown keyword '"+std::string(c->GetText())+"'.") ), FUNC_ID);
				}
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ELEMENT).arg(cName).arg(c->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			c = c->NextSiblingElement();
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'Component' element."), FUNC_ID);
	}
	catch (std::exception & ex2) {
		throw IBK::Exception( IBK::FormatString("%1\nError reading 'Component' element.").arg(ex2.what()), FUNC_ID);
	}
}

TiXmlElement * Component::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("Component");
	parent->LinkEndChild(e);

	if (m_id != VICUS::INVALID_ID)
		e->SetAttribute("id", IBK::val2string<unsigned int>(m_id));
	if (!m_displayName.isEmpty())
		e->SetAttribute("displayName", m_displayName.toStdString());
	if (!m_color.isValid())
		e->SetAttribute("color", m_color.name().toStdString());
	if (!m_manufacturer.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "Manufacturer", nullptr, std::string(), m_manufacturer.toStdString());
	if (!m_dataSource.isEmpty())
		TiXmlElement::appendSingleAttributeElement(e, "DataSource", nullptr, std::string(), m_dataSource.toStdString());

	if (m_type != NUM_CK)
		TiXmlElement::appendSingleAttributeElement(e, "Type", nullptr, std::string(), KeywordList::Keyword("Component::CompontType",  m_type));
	if (m_idOpaqueConstruction != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdOpaqueConstruction", nullptr, std::string(), IBK::val2string<unsigned int>(m_idOpaqueConstruction));
	if (m_idGlazingSystem != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdGlazingSystem", nullptr, std::string(), IBK::val2string<unsigned int>(m_idGlazingSystem));
	if (m_idOutsideBoundaryCondition != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdOutsideBoundaryCondition", nullptr, std::string(), IBK::val2string<unsigned int>(m_idOutsideBoundaryCondition));
	if (m_idInsideBoundaryCondition != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdInsideBoundaryCondition", nullptr, std::string(), IBK::val2string<unsigned int>(m_idInsideBoundaryCondition));
	if (m_idSurfaceProperty != VICUS::INVALID_ID)
		TiXmlElement::appendSingleAttributeElement(e, "IdSurfaceProperty", nullptr, std::string(), IBK::val2string<unsigned int>(m_idSurfaceProperty));
	return e;
}

} // namespace VICUS
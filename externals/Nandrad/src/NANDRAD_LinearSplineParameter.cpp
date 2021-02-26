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

#include "NANDRAD_LinearSplineParameter.h"

#include <IBK_messages.h>
#include <IBK_Exception.h>
#include <IBK_StringUtils.h>
#include <IBK_UnitVector.h>
#include <IBK_CSVReader.h>
#include <NANDRAD_Constants.h>
#include <NANDRAD_KeywordList.h>
#include <NANDRAD_Utilities.h>

#include <tinyxml.h>

namespace NANDRAD {

void LinearSplineParameter::readXML(const TiXmlElement * element) {
	FUNCID(LinearSplineParameter::readXML);

	try {
		// search for mandatory attributes
		// reading attributes
		const TiXmlAttribute * attrib = element->FirstAttribute();
		while (attrib) {
			const std::string & attribName = attrib->NameStr();
			if (attribName == "name")
				m_name = attrib->ValueStr();
			else if (attribName == "interpolationMethod")
			try {
				m_interpolationMethod = (interpolationMethod_t)KeywordList::Enumeration("LinearSplineParameter::interpolationMethod_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			else if (attribName == "wrapMethod")
			try {
				m_wrapMethod = (wrapMethod_t)KeywordList::Enumeration("LinearSplineParameter::wrapMethod_t", attrib->ValueStr());
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					IBK::FormatString("Invalid or unknown keyword '"+attrib->ValueStr()+"'.") ), FUNC_ID);
			}
			else {
				IBK::IBK_Message(IBK::FormatString(XML_READ_UNKNOWN_ATTRIBUTE).arg(attribName).arg(element->Row()), IBK::MSG_WARNING, FUNC_ID, IBK::VL_STANDARD);
			}
			attrib = attrib->Next();
		}

		// now read actual spline data
		std::string name;
		std::string xunitstr, yunitstr, interpolationMethod, pathStr;
		std::vector<double> x,y;
		try {
			// note: interpolation method is not written or read, it will always be ""
			TiXmlElement::readIBKLinearSplineParameterElement(element, name, interpolationMethod, xunitstr, x,
															  yunitstr, y, pathStr);
		}
		catch (std::runtime_error & ex) {
			throw IBK::Exception( ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				IBK::FormatString("Error reading 'LinearSplineParameter' tag.") ), FUNC_ID);
		}

		// check if both x,y and TSVFile are given
		if (!pathStr.empty() && !x.empty() && !y.empty()){
			throw IBK::Exception(IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
				 IBK::FormatString("Error reading 'LinearSplineParameter' tag. There must be either TSVFile or X and Y") ), FUNC_ID);
		}

		// set either path
		if (!pathStr.empty()){
			m_tsvFile = IBK::Path(pathStr);
		}
		// or set spline
		else{
			try {
				m_xUnit = IBK::Unit(xunitstr); // may throw in case of invalid unit
				m_yUnit = IBK::Unit(yunitstr);
				m_values.setValues(x,y); // may throw in case of invalid data
			}
			catch (IBK::Exception & ex) {
				throw IBK::Exception(ex, IBK::FormatString(XML_READ_ERROR).arg(element->Row()).arg(
					 IBK::FormatString("Error reading 'LinearSplineParameter' tag.") ), FUNC_ID);
			}
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception( ex, IBK::FormatString("Error reading 'LinearSplineParameter' element."), FUNC_ID);
	}
}


TiXmlElement * LinearSplineParameter::writeXML(TiXmlElement * parent) const {
	TiXmlElement * e = new TiXmlElement("LinearSplineParameter");
	parent->LinkEndChild(e);

	if (!m_name.empty())
		e->SetAttribute("name", m_name);
	if (m_interpolationMethod != NUM_I)
		e->SetAttribute("interpolationMethod", KeywordList::Keyword("LinearSplineParameter::interpolationMethod_t",  m_interpolationMethod));
	if (m_wrapMethod != NUM_C)
		e->SetAttribute("wrapMethod", KeywordList::Keyword("LinearSplineParameter::wrapMethod_t",  m_wrapMethod));

	// set attributes
	if (!m_values.empty()){
		TiXmlElement::appendIBKUnitVectorElement(e, "X", m_xUnit.name(), m_values.x(), true);
		TiXmlElement::appendIBKUnitVectorElement(e, "Y", m_yUnit.name(), m_values.y(), true);
	}

	if (m_tsvFile.isValid()){
		TiXmlElement::appendSingleAttributeElement(e, "TSVFile", nullptr, std::string(), m_tsvFile.str());
	}

	return e;
}


void LinearSplineParameter::checkAndInitialize(const std::string & expectedName, const IBK::Unit & targetXUnit, const IBK::Unit & targetYUnit,
											   const IBK::Unit & limitYUnit, double minYVal, bool isGreaterEqual,
											   double maxYVal, bool isLessEqual, const char * const errmsg)
{
	FUNCID(LinearSplineParameter::checkAndInitialize);

	std::string suffix = (errmsg == nullptr) ? "" : "\n" + std::string(errmsg);

	// check 0: parameter name must not be empty
	if (m_name.empty())
		throw IBK::Exception(IBK::FormatString("Linear spline parameter '%1' is missing/undefined.").arg(expectedName), FUNC_ID);

	// check 1: check if name is correct
	if (m_name != expectedName)
		throw IBK::Exception(IBK::FormatString("Name '%1' expected, but '%2' given.").arg(expectedName).arg(m_name), FUNC_ID);

	// check 2: either tsvFile or values can exist
	if (m_tsvFile.isValid() && !m_values.empty())
		throw IBK::Exception("Either a tsvFile or values can be specified. Both is not possible", FUNC_ID);

	// if there is a valid tsv-file: read it and set values
	if (m_tsvFile.isValid()) {
		// extract column identifier, if any
		std::string fpath = m_tsvFile.str();
		unsigned int colIndex = 1; // 1 means "first data column" (actually 2nd column in file, with 0-based index 1)
		std::size_t pos = fpath.find('?');
		if (pos != std::string::npos) {
			try {
				colIndex = IBK::string2val<unsigned int>(fpath.substr(pos+1));
			} catch (IBK::Exception & ex) {
				throw IBK::Exception(IBK::FormatString("Malformed file name '%1' (invalid column indicator).").arg(m_tsvFile.str()), FUNC_ID);
			}
			fpath = fpath.substr(0, pos);
		}

		// this is now the path without column indicator
		IBK::Path tsvFilePath(fpath);

		if (!IBK::Path(tsvFilePath).exists())
			throw IBK::Exception(IBK::FormatString("File '%1' does not exist").arg(m_tsvFile.str()), FUNC_ID);
		IBK::CSVReader reader;
		reader.read(tsvFilePath, false, true);  // may throw exception
		if (reader.m_nColumns <= colIndex)
			throw IBK::Exception(IBK::FormatString("File '%1' must have exactly %2 columns")
								 .arg(tsvFilePath.str()).arg(colIndex+1), FUNC_ID); // Mind: column count = 1 (time column) + colIndex
		if (reader.m_nRows < 2)
			throw IBK::Exception(IBK::FormatString("File '%1' must have at least 2 rows")
								 .arg(tsvFilePath.str()), FUNC_ID);
		m_xUnit = IBK::Unit(reader.m_units[0]);
		m_yUnit = IBK::Unit(reader.m_units[colIndex]);
		m_values.setValues(reader.colData(0), reader.colData(colIndex));
	}

	// argument checks
	if (targetXUnit.id() != targetXUnit.base_id() ||
		targetYUnit.id() != targetYUnit.base_id())
	{
		throw IBK::Exception("Target units must be base SI units.", FUNC_ID);
	}
	if (targetYUnit.base_id() != limitYUnit.base_id())
		throw IBK::Exception(IBK::FormatString("Incompatible y target unit '%1' and limit unit '%2'.")
							 .arg(targetYUnit).arg(limitYUnit), FUNC_ID);

	// now check m_xUnit and m_yUnit

	// check 2: units convertible?
	if (targetXUnit.base_id() != m_xUnit.base_id())
		throw IBK::Exception( IBK::FormatString("Mismatching x units, cannot convert from '%1' to '%2'.")
							  .arg(m_xUnit.name()).arg(targetXUnit).arg(suffix), FUNC_ID);
	if (targetYUnit.base_id() != m_yUnit.base_id())
		throw IBK::Exception( IBK::FormatString("Mismatching y units, cannot convert from '%1' to '%2'.")
							  .arg(m_yUnit.name()).arg(targetYUnit).arg(suffix), FUNC_ID);

	// check 3: convert to base unit
	convert2BaseUnits(); // throws an exception if invalid
	// and update units if successful
	m_xUnit = targetXUnit;
	m_yUnit = targetYUnit;

	// check 4: range check?
//	for (double d : m_values.y()) {

//	}

}


void LinearSplineParameter::convert2BaseUnits() {
	FUNCID(LinearSplineParameter::convert2BaseUnits);
	try {
		// construct unitvector for x values
		IBK::UnitVector xVec(m_values.x().begin(), m_values.x().end(), m_xUnit);
		xVec.convert(m_xUnit.base_unit());
		// construct unitvector for y values
		IBK::UnitVector yVec(m_values.y().begin(), m_values.y().end(), m_yUnit);
		yVec.convert(m_yUnit.base_unit());
		m_values.setValues(xVec.m_data, yVec.m_data);
	} catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, "Error converting spline data from input/output units to base SI units.", FUNC_ID);
	}
}


bool LinearSplineParameter::operator!=(const LinearSplineParameter & other) const {
	if (m_name != other.m_name) return true;
	if (m_interpolationMethod != other.m_interpolationMethod) return true;
	if (m_values != other.m_values) return true;
	if (m_xUnit != other.m_xUnit) return true;
	if (m_yUnit != other.m_yUnit) return true;
	return false;
}

} // namespace NANDRAD


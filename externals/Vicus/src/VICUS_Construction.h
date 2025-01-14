#ifndef VICUS_ConstructionH
#define VICUS_ConstructionH

#include "VICUS_CodeGenMacros.h"
#include "VICUS_Constants.h"
#include "VICUS_MaterialLayer.h"

#include <QString>
#include <QColor>

#include <vector>

#include <IBK_Flag.h>
#include <IBK_Parameter.h>

#include "VICUS_AbstractDBElement.h"
#include "VICUS_Database.h"
#include "VICUS_Material.h"

namespace VICUS {

class Construction : public AbstractDBElement {
public:
	enum UsageType {
		UT_OutsideWall,				// Keyword: OutsideWall				'Outside wall construction'
		UT_OutsideWallToGround,		// Keyword: OutsideWallToGround		'Outside wall construction in contact with ground'
		UT_InsideWall,				// Keyword: InsideWall				'Interior construction'
		UT_FloorToCellar,			// Keyword: FloorToCellar			'Floor to basement'
		UT_FloorToGround,			// Keyword: FloorToGround			'Floor in contact with ground'
		UT_Ceiling,					// Keyword: Ceiling					'Ceiling construction'
		UT_SlopedRoof,				// Keyword: SlopedRoof				'Sloped roof construction'
		UT_FlatRoof,				// Keyword: FlatRoof				'Flat roof construction'
		NUM_UT						// Keyword: ---						'Miscellaneous'
	};

	enum InsulationKind {
		IK_NotInsulated,			// Keyword: NotInsulated			'Not insulated'
		IK_InsideInsulation,		// Keyword: InsideInsulation		'Inside insulated'
		IK_CoreInsulation,			// Keyword: CoreInsulation			'Core insulation'
		IK_OutsideInsulation,		// Keyword: OutsideInsulation		'Outside insulated'
		NUM_IK						// Keyword: ---						'Not selected'
	};

	enum MaterialKind {
		MK_BrickMasonry,			// Keyword: BrickMasonry			'Brick masonry'
		MK_NaturalStoneMasonry,		// Keyword: NaturalStoneMasonry		'Natural stones'
		MK_Concrete,				// Keyword: Concrete				'Concrete'
		MK_Wood,					// Keyword: Wood					'Wood'
		MK_FrameWork,				// Keyword: FrameWork				'Frame construction'
		MK_Loam,					// Keyword: Loam					'Loam'
		NUM_MK						// Keyword: ---						'Not selected'
	};

	// *** PUBLIC MEMBER FUNCTIONS ***

	VICUS_READWRITE
	VICUS_COMPARE_WITH_ID

	/*! Checks if all referenced materials exist and if their parameters are valid. */
	bool isValid(const VICUS::Database<VICUS::Material> & materials) const;

	/*! Computes the u-value. */
	bool calculateUValue(double & UValue, const VICUS::Database<Material> & materials, double ri, double re) const;

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID of construction. */
	unsigned int					m_id = INVALID_ID;					// XML:A:required

	/*! The usage type (classification property). */
	UsageType						m_usageType = NUM_UT;				// XML:E
	/*! The type of insulation used (classification property). */
	InsulationKind					m_insulationKind = NUM_IK;			// XML:E
	/*! The main/load bearing material (classification property). */
	MaterialKind					m_materialKind = NUM_MK;			// XML:E

	/*! Display name of construction. */
	IBK::MultiLanguageString		m_displayName;						// XML:A

	/*! False color. */
	QColor							m_color;							// XML:A

	/*! Notes. */
	IBK::MultiLanguageString		m_notes;							// XML:E

	/*! Data source. */
	IBK::MultiLanguageString		m_dataSource;						// XML:E

	/*! The individual material layers. */
	std::vector<MaterialLayer>		m_materialLayers;					// XML:E
};

} // namespace VICUS


#endif // VICUS_ConstructionH

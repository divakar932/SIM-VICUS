#ifndef NANDRAD_HydraulicNetworkComponentH
#define NANDRAD_HydraulicNetworkComponentH

#include <IBK_Parameter.h>

#include "NANDRAD_CodeGenMacros.h"
#include "NANDRAD_Constants.h"


namespace NANDRAD {

/*! Contain dataset for a hydraulic component for the network.

*/
class HydraulicNetworkComponent {
public:

	/*! The various types (equations) of the hydraulic component. */
	enum modelType_t {
		MT_StaticAdiabaticPipe,				// Keyword: StaticAdiabaticPipe			'Simple pipe at stationary flow conditions without heat exchange'
		MT_StaticPipe,						// Keyword: StaticPipe					'Simple pipe at stationary flow conditions with heat exchange'
		MT_DynamicAdiabaticPipe,			// Keyword: DynamicAdiabaticPipe		'Pipe with a discretized fluid volume, without heat exchange'
		MT_DynamicPipe,						// Keyword: DynamicPipe					'Pipe with a discretized fluid volume and heat exchange'
		MT_ConstantPressurePumpModel,		// Keyword: ConstantPressurePumpModel	'Pump with constant pressure'
		MT_HeatExchanger,					// Keyword: HeatExchanger				'Simple heat exchanger with given heat flux'
		MT_HeatPump,						// Keyword: HeatPump					'Heat pump'
		MT_GasBoiler,						// Keyword: GasBoiler					'Gas boiler'
		MT_ControlValve,					// Keyword: ControlValve				'Control valve'
		MT_WaterStorage,					// Keyword: WaterStorage				'Water storage'
		MT_ComponentConditionSystem,		// Keyword: ComponentConditionSystem	'Component conditioning system is a system for heating or cooling of components'
		MT_Radiator,						// Keyword: Radiator					'Radiator'
		MT_Mixer,							// Keyword: Mixer						'Mixer component'
		MT_FMU,								// Keyword: FMU							'Flow characteristics provided by FMU'
		NUM_MT
	};

	/*! Parameters for the component. */
	enum para_t {
		P_PipeRoughness,					// Keyword: PipeRoughness						[mm]	'Roughness of pipe material.'
		P_PressureLossCoefficient,			// Keyword: PressureLossCoefficient				[-]		'Pressure loss coefficient for the component (zeta-value).'
		P_HydraulicDiameter,				// Keyword: HydraulicDiameter					[mm]	'Inside hydraulic diameter for the component.'
		P_ExternalHeatTransferCoefficient,	// Keyword: ExternalHeatTransferCoefficient		[W/m2K]	'External heat transfer coeffient for the outside boundary.'
		P_TemperatureTolerance,				// Keyword: TemperatureTolerance				[K]		'Temperature tolerance for e.g. thermostats.'
		P_PressureHead,						// Keyword: PressureHead						[Pa]	'Pressure head form a pump.'
		P_PumpEfficiency,					// Keyword: PumpEfficiency						[---]	'Pump efficiency.'
		P_MotorEfficiency,					// Keyword: MotorEfficiency						[---]	'Motor efficiency for a pump.'
		P_RatedHeatingCapacity,				// Keyword: RatedHeatingCapacity				[W]		'Rated heating capacity of the component.'
		P_RatedCoolingCapacity,				// Keyword: RatedCoolingCapacity				[W]		'Rated Cooling capacity of the component.'
		P_AuxiliaryPower,					// Keyword: AuxiliaryPower						[W]		'Auxiliary power of the component.'
		P_Volume,							// Keyword: Volume								[m3]	'Water or air volume of the component.'
		P_ExternalSurfaceArea,				// Keyword: ExternalSurfaceArea					[m2]	'External surface area of the component.'
		P_ConvectiveFraction,				// Keyword: ConvectiveFraction					[---]	'Convective fraction for heating or cooling.'
		P_COP,								// Keyword: COP									[-]		'Coefficient of performance of the component.'
		NUM_P
	};


	enum heatExchangeType_t {
		HT_HeatFluxConstant,				// Keyword: HeatFluxConstant					[-]		'Constant heat flux'
		HT_HeatFluxDataFile,				// Keyword: HeatFluxDataFile					[-]		'Heat flux from data file '
		HT_HeatExchangeWithZoneTemperature,	// Keyword: HeatExchangeWithZoneTemperature		[-]		'Heat exchange with zone'
		HT_HeatExchangeWithFMUTemperature,	// Keyword: HeatExchangeWithFMUTemperature		[-]		'Heat exchange with FMU which requires temperature and provides heat flux'
		NUM_HT
	};


//	/*! type of interface to an external model or data file */
//	enum interfaceType_t {
//		IT_HeatFlux,						// Keyword: HeatFlux							'Fixed heat flux'
//		IT_Temperature,						// Keyword: Temperature							'Fixed temperature'
//		IT_HeatExchangeGivenTemperature,	// Keyword: HeatExchangeGivenTemperature		'Coupled to external model which requires a heat flux and calculates a temperature in return'
//		IT_ThermoHydraulicGivenPinPout,		// Keyword: ThermoHydraulicGivenPinPout			'Coupled to external model which requires p_in, p_out, T_in and calculates T_out and m_dot'
//		IT_ThermoHydraulicGivenPinMdot,		// Keyword: ThermoHydraulicGivenPinMdot			'Coupled to external model which requires p_in, m_dot, T_in and calculates T_out and p_out'
//		NUM_IT
//	};


	// *** PUBLIC MEMBER FUNCTIONS ***

	NANDRAD_READWRITE
	NANDRAD_COMPARE_WITH_ID

	// *** PUBLIC MEMBER VARIABLES ***

	/*! Unique ID for this component. */
	unsigned int					m_id			= NANDRAD::INVALID_ID;				// XML:A:required

	/*! Display name. */
	std::string						m_displayName;										// XML:A

	/*! Model type. */
	modelType_t						m_modelType		= NUM_MT;							// XML:A:required

	/*! Type of interface to external data or model */
	heatExchangeType_t				m_heatExchangeType = NUM_HT;

	/*! Parameters of the flow component. */
	IBK::Parameter					m_para[NUM_P];										// XML:E

	bool operator==(const HydraulicNetworkComponent &other) const{
		for (unsigned n=0; n<NUM_P; ++n){
			if (m_para[n] != other.m_para[n])
				return false;
		}
		return (m_modelType == other.m_modelType && m_heatExchangeType == other.m_heatExchangeType);
	}


	static bool hasHeatExchange(const modelType_t modelType){
		switch (modelType) {
			case MT_StaticPipe:
			case MT_DynamicPipe:
			case MT_HeatPump:
			case MT_HeatExchanger:
			case MT_Radiator:
				return true;
			default:
				return false;
		}
	}

};

} // namespace NANDRAD

#endif // NANDRAD_HydraulicNetworkComponentH

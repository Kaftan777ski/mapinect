#include "PownConstants.h"

#include "ofxXmlSettings.h"

#define POWN_CONFIG			"PownConfig:"

namespace pown
{

	float				PownConstants::EMIT_TIME							= 1.0f;

	float				PownConstants::SPOT_BASE_RADIUS						= 0.05f;
	float				PownConstants::SPOT_ROTATION_PERIOD_TIME			= 3.0f;

	float				PownConstants::BOLT_INITIAL_SPEED					= 0.1f;
	float				PownConstants::BOLT_BASE_RADIUS						= 0.02f;
	float				PownConstants::BOLT_INTENSITY_DECREASE_FACTOR		= 0.3f;
	float				PownConstants::BOLT_BOOST_COLOR_DECREASE_FACTOR		= 150.0f;

	void PownConstants::LoadPownConstants()
	{
		ofxXmlSettings XML;
		if(XML.loadFile("Pown_Config.xml"))
		{
			EMIT_TIME						= XML.getValue(POWN_CONFIG "EMIT_TIME", EMIT_TIME);
			SPOT_BASE_RADIUS				= XML.getValue(POWN_CONFIG "SPOT_BASE_RADIUS", SPOT_BASE_RADIUS);
			SPOT_ROTATION_PERIOD_TIME		= XML.getValue(POWN_CONFIG "SPOT_ROTATION_PERIOD_TIME", SPOT_ROTATION_PERIOD_TIME);
			BOLT_INITIAL_SPEED				= XML.getValue(POWN_CONFIG "BOLT_INITIAL_SPEED", BOLT_INITIAL_SPEED);
			BOLT_BASE_RADIUS				= XML.getValue(POWN_CONFIG "BOLT_BASE_RADIUS", BOLT_BASE_RADIUS);
			BOLT_INTENSITY_DECREASE_FACTOR	= XML.getValue(POWN_CONFIG "BOLT_INTENSITY_DECREASE_FACTOR", BOLT_INTENSITY_DECREASE_FACTOR);
			BOLT_BOOST_COLOR_DECREASE_FACTOR= XML.getValue(POWN_CONFIG "BOLT_BOOST_COLOR_DECREASE_FACTOR", BOLT_BOOST_COLOR_DECREASE_FACTOR);
		}
	}

}
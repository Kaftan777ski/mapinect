#ifndef SORYSTATUS_H__
#define SORYSTATUS_H__

#include <map>
#include "ofMain.h"
#include "ofVec3f.h"

namespace story
{
	enum ofVec3fStoryStatusProperty{
		CENTROID_BURNING_HOUSE = 0,
	};
	
	enum StoryStatusProperty{
		ADDING_STREET = 0,
		ADDING_RIVER,
		ADDING_POWERPLANT,
		ADDING_WATERPLANT,
		ADDING_HOUSE,
		POWERPLANT_ACTIVE,
		WATERPLANT_ACTIVE,
		FIREMAN_FINISHED,
		ALREADY_BURNING,
		WANT_TO_BURN,
		BURNING,
	};

	enum IntStoryStatusProperty{
		ID_BURNING_HOUSE = 0,
	};


	
	class StoryStatus
	{
		private:
			static std::map<StoryStatusProperty, bool> properties;
			static std::map<IntStoryStatusProperty, int> intProperties;
			static std::map<ofVec3fStoryStatusProperty, ofVec3f> ofVec3fProperties;
		public:
			static void			setup();
			static void			setProperty(StoryStatusProperty prop, bool val);
			static void			setProperty(IntStoryStatusProperty prop, int val);
			static void			setProperty(ofVec3fStoryStatusProperty prop, ofVec3f val);
			static bool			getProperty(StoryStatusProperty prop);
			static int			getIntProperty(IntStoryStatusProperty prop);
			static ofVec3f		getofVec3fProperty(ofVec3fStoryStatusProperty prop);
	};

}

#endif	// SORYSTATUS_H__

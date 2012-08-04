#ifndef ROAD_H__
#define ROAD_H__

#include "IObject.h"
#include "ofImage.h"
#include "ofVec3f.h"

using namespace mapinect;

namespace story
{
	class Road
	{
	public:
		Road(const ofVec3f begin, const ofVec3f end);
		virtual ~Road();

		void			update(float elapsedTime);
		void			draw();

	private:
		ofImage*		texture;
		ofVec3f			begin;
		ofVec3f			end;
		ofVec3f			normal;
		
	};
}

#endif	// ROAD_H__
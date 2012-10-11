#ifndef POWN_H__
#define POWN_H__

#include "IApplication.h"

#include <map>
#include <set>
#include "Box.h"
#include "Buildings/PowerPlant.h"
#include "Buildings/WaterPlant.h"
#include "DraggableButton.h"
#include "ofVec3f.h"
#include "Road.h"
#include "Park.h"
#include "River.h"
#include "Spot.h"
#include "ofImage.h"
#include "SimpleButton.h"
#include "Menu.h"
#include "StoryStatus.h"
#include "Canvas.h"


namespace story
{


	class Story : public IApplication
	{
	public:
		Story();
		virtual ~Story();

		virtual void setup();
		virtual void update(float elapsedTime);
		virtual void draw();

		virtual void keyPressed(int key);

		virtual void debugDraw();

		virtual void objectDetected(const IObjectPtr&);
		virtual void objectUpdated(const IObjectPtr&);
		virtual void objectLost(const IObjectPtr&);
		virtual void objectMoved(const IObjectPtr&, const DataMovement&);
		virtual void objectTouched(const IObjectPtr&, const DataTouch&);
		virtual void buttonPressed(const IButtonPtr&, const DataTouch&);
		virtual void buttonReleased(const IButtonPtr&, const DataTouch&);
		virtual void pointTouched(const DataTouch&);
		virtual void armStoppedMoving();

		void setCamera(int camera);

	private:
		std::map<int, Box*>			boxes;
		std::map<int, Box*>			houses;
		PowerPlant*					power_plant;
		WaterPlant*					water_plant;
		std::map<int, DataTouch>	touchPoints;

		std::list<Road>				roads;
		std::list<Park>				parks;
		Canvas*						river;
		//River						river; //por ahora, solo un r�o
		Menu						menu;
		StoryStatus*				status;

		IObjectPtr					floor;
		ofVec3f						firstTableTouch;
		bool						firstTouchDone;
		void						touchTable(const IObjectPtr&, const DataTouch&);
		void						touchObject(const IObjectPtr&, const DataTouch&);
		
		map<int,Box*>::iterator		selectedBoxIdx;
		Spot						spot;

		enum StoryMode				previousMode;
		unsigned long				startTime;

		void						tableUpdated(const IObjectPtr& object);
	};
}

#endif	// POWN_H__

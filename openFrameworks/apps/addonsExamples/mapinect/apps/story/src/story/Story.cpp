#include "Story.h"

#include "ObjectButton.h"
#include "House.h"
#include "River.h"
#include "Road.h"
#include "StoryConstants.h"


namespace story {
	
	//--------------------------------------------------------------
	Story::Story()
	{
	}

	//--------------------------------------------------------------
	Story::~Story() {
		for (map<int, Box*>::iterator i = boxes.begin(); i != boxes.end(); i++) {
			delete i->second;
		}
		boxes.clear();
	}

	//--------------------------------------------------------------
	void Story::setup() {
		selectedBoxIdx = boxes.end();
		StoryConstants::LoadStoryConstants();
		Spot::setup();
		House::setup();
		WaterPlant::setup();
		PowerPlant::setup();
		StoryStatus::setup();

		menu.setup(btnManager);

		modeManager->disableObjectTracking();
		firstTouchDone = false;
		river = NULL;
	}

	//--------------------------------------------------------------
	void Story::debugDraw()
	{
		
	}

	//--------------------------------------------------------------
	void Story::draw()
	{
		for(map<int,Box*>::iterator it = boxes.begin(); it != boxes.end(); ++it)
			it->second->draw();
		if(spot.isActive())
			spot.draw();

		if(river != NULL)
			river->draw();

		for (map<int, DataTouch>::const_iterator it = touchPoints.begin(); it != touchPoints.end(); ++it)
		{
			if (it->second.getType() == kTouchTypeStarted)
				ofSetHexColor(0xFF0000);
			else if (it->second.getType() == kTouchTypeHolding)
				ofSetHexColor(0x00FF00);
			else
				ofSetHexColor(0x0000FF);
			ofVec3f s = it->second.getTouchPoint();
			ofCircle(s.x, s.y, s.z, 0.01);
			
		}
	}

	

	//--------------------------------------------------------------
	void Story::update(float elapsedTime) 
	{
		//spot
		if(spot.isActive())
			spot.update(elapsedTime);

		//touchpoints
		map<int, DataTouch> keep;
		for (map<int, DataTouch>::const_iterator it = touchPoints.begin(); it != touchPoints.end(); ++it)
		{
			if (it->second.getType() != kTouchTypeReleased)
				keep.insert(make_pair(it->first, it->second));
		}
		touchPoints = keep;

		//Menu
		menu.update(elapsedTime);

		//Modo
		if (StoryStatus::getProperty(ADDING_HOUSE) || StoryStatus::getProperty(ADDING_POWERPLANT) || StoryStatus::getProperty(ADDING_WATERPLANT))
		{
			modeManager->enableObjectTracking();
			modeManager->disableTouchTracking();
		}
		else if (StoryStatus::getProperty(ADDING_RIVER) || StoryStatus::getProperty(ADDING_STREET))
		{
			modeManager->disableObjectTracking();
			modeManager->enableTouchTracking();
		}

		//Buildings
		for (map<int, Box*>::const_iterator it = boxes.begin(); it != boxes.end(); ++it)
			it->second->update(elapsedTime);

	}

	//--------------------------------------------------------------
	void Story::keyPressed(int key)
	{
		cout << "key pressed" << endl;
		switch(key)
		{
			case 'c':
				setStoryMode(STORY_ACTION_MODE);
				break;
			case 'm':
				setStoryMode(STORY_MOVE_MODE);
				break;

		}
	}

	//--------------------------------------------------------------
	void Story::objectDetected(const IObjectPtr& object)
	{
		if (object->getId() == TABLE_ID)
		{
			floor = object;
			Polygon3D table = object->getPolygons().at(0)->getMathModel();
			ofVec3f tableNormal = table.getPlane().getNormal();
			ofVec3f translateCanvas = tableNormal * -0.009;
			vector<ofVec3f> oldVexs = table.getVertexs();
			vector<ofVec3f> newVexs;
			for(int i = 0; i < 4; i++)
				newVexs.push_back(oldVexs.at(i)+translateCanvas);

			table.setVertexs(newVexs);
			river = new Canvas(object->getId(),table, 300,300,ofColor(220,110,50),ofColor(75,140,250),50);
		}
		else
		{
			if (StoryStatus::getProperty(ADDING_HOUSE))
			{
				House* h  = new House(object, btnManager);
				boxes.insert(pair<int, Box*>(object->getId(), h));
				setStoryMode(STORY_ACTION_MODE);
			}
			else if (StoryStatus::getProperty(ADDING_POWERPLANT))
			{
				PowerPlant* plant  = new PowerPlant(object, btnManager);
                boxes.insert(pair<int, Box*>(object->getId(), plant));
                setStoryMode(STORY_ACTION_MODE);
			}
			else if (StoryStatus::getProperty(ADDING_WATERPLANT))
			{
				WaterPlant* plant  = new WaterPlant(object, btnManager);
                boxes.insert(pair<int, Box*>(object->getId(), plant));
                setStoryMode(STORY_ACTION_MODE);
			}
		}
	}
		
	//--------------------------------------------------------------
	void Story::objectUpdated(const IObjectPtr& object)
	{
		if (object->getId() == TABLE_ID)
			river->update(object->getPolygons().at(0)->getMathModel());
		else
		{
			map<int,Box*>::iterator box = boxes.find(object->getId());
			if(box != boxes.end())
				box->second->updateModelObject(object);
		}
		/*Box* b = new Box(object);
		boxes.insert(pair<int, Box*>(1, b));*/
	}

	//--------------------------------------------------------------
	void Story::objectLost(const IObjectPtr& object)
	{
		map<int,Box*>::iterator box = boxes.find(object->getId());
		if(box != boxes.end())
			boxes.erase(box);
	}

	//--------------------------------------------------------------
	void Story::objectMoved(const IObjectPtr& object, const DataMovement& movement)
	{

	}
	
	//--------------------------------------------------------------
	void Story::objectTouched(const IObjectPtr& object, const DataTouch& touchPoint)
	{
		if (object->getId() == TABLE_ID)
		{
			//reseteo seleccion de objetos
			cout << " no selecciono mas" << endl;
			selectedBoxIdx = boxes.end(); 
			spot.setActive(false);

			touchTable(object, touchPoint);
		}
		else 
		{
			touchObject(object, touchPoint);
		}
	}

	void Story::touchTable(const IObjectPtr& object, const DataTouch& touchPoint)
	{
		if (StoryStatus::getProperty(ADDING_RIVER))
		{
			cout << "touchRiver" << endl;
			river->touchEvent(touchPoint);
		}
		else if (StoryStatus::getProperty(ADDING_STREET))
		{
			if (firstTouchDone && firstTableTouch.distance(touchPoint.getTouchPoint())> 0.05)
			{
				
				if (StoryStatus::getProperty(ADDING_STREET))
				{
					Road road = Road(firstTableTouch, touchPoint.getTouchPoint());
					btnManager->addButton(road.button);
					roads.push_back(road);
				}
				firstTouchDone = false;
				StoryStatus::setProperty(ADDING_STREET,false);
			}
			else
			{
				firstTouchDone = true;
				this->firstTableTouch = touchPoint.getTouchPoint();
			}
		}
		else
		{
			menu.objectEvent(object, touchPoint);
		}
	}

	void Story::touchObject(const IObjectPtr& object, const DataTouch& touchPoint)
	{
		map<int,Box*>::iterator touchedIdx = boxes.find(object->getId());
		if(touchedIdx != boxes.end())
		{
			if(selectedBoxIdx == boxes.end())
			{
				cout << "seleccione: " << touchedIdx->second->getBuildType() << endl;
				selectedBoxIdx = touchedIdx;
				ofVec3f spotCenter = floor->getPolygons()[0]->getMathModel().getPlane().project(object->getCenter());
				spotCenter.y -= 0.001f;
				
				vector<ofVec3f> vexs1 = object->getPolygon(kPolygonNameSideA)->getMathModel().getVertexs();
				vector<ofVec3f> vexs2 = object->getPolygon(kPolygonNameSideB)->getMathModel().getVertexs();
				float size = max(abs((vexs1[1] - vexs1[2]).length()),abs((vexs2[1] - vexs2[2]).length()))  + 0.03;
				spot.setSize(size);
				spot.setPosition(spotCenter);
			}
			else if(selectedBoxIdx != touchedIdx)
			{
				cout << "mando evento a: " << touchedIdx->second->getBuildType() << endl;
				touchedIdx->second->objectEvent(touchPoint,selectedBoxIdx->second->getBuildType());
				selectedBoxIdx = boxes.end(); 
				spot.setActive(false);
			}
		}
	}

	void Story::buttonPressed(const IButtonPtr& btn, const DataTouch& touchPoint)
	{
		menu.buttonEvent(btn,false);
		for(map<int,Box*>::iterator it = boxes.begin(); it != boxes.end(); ++it)
			it->second->buttonEvent(btn, false);
	}

	void Story::buttonReleased(const IButtonPtr& btn, const DataTouch& touchPoint)
	{
		menu.buttonEvent(btn,true);
		for(map<int,Box*>::iterator it = boxes.begin(); it != boxes.end(); ++it)
		{
			it->second->buttonEvent(btn,true);
		}
	}

	void Story::pointTouched(const DataTouch& touchPoint)
	{
		//touchpoints
		map<int, DataTouch>::iterator it = touchPoints.find(touchPoint.getId());
		if (it == touchPoints.end())
		{
			//assert(touchPoint.getType() == kTouchTypeStarted);
			touchPoints.insert(make_pair(touchPoint.getId(), touchPoint));
		}
		else
		{
			it->second = touchPoint;
		}
	}

	void Story::setStoryMode(StoryMode mode)
	{
		switch(mode)
		{
			case STORY_ACTION_MODE:
                StoryStatus::setProperty(ADDING_POWERPLANT, false);
                StoryStatus::setProperty(ADDING_WATERPLANT, false);
				StoryStatus::setProperty(ADDING_HOUSE,false);
				StoryStatus::setProperty(ADDING_RIVER,false);
				StoryStatus::setProperty(ADDING_STREET,false);
				modeManager->disableObjectTracking();
				modeManager->enableTouchTracking();
				break;
			case STORY_MOVE_MODE:
				StoryStatus::setProperty(ADDING_POWERPLANT, false);
                StoryStatus::setProperty(ADDING_WATERPLANT, false);
				StoryStatus::setProperty(ADDING_HOUSE,false);
				StoryStatus::setProperty(ADDING_RIVER,false);
				StoryStatus::setProperty(ADDING_STREET,false);
				modeManager->enableObjectTracking();
				modeManager->disableTouchTracking();
				break;
			case STORY_MOVE_AND_ACTION_MODE:
				StoryStatus::setProperty(ADDING_POWERPLANT, false);
                StoryStatus::setProperty(ADDING_WATERPLANT, false);
				StoryStatus::setProperty(ADDING_HOUSE,false);
				StoryStatus::setProperty(ADDING_RIVER,false);
				StoryStatus::setProperty(ADDING_STREET,false);
				modeManager->enableObjectTracking();
				modeManager->enableTouchTracking();
				break;
		}
	}


}

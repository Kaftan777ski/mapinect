#include "PCM.h"

#include "Feature.h"
#include "Globals.h"
#include "log.h"
#include "pointUtils.h"
#include "utils.h"

using namespace std;

namespace mapinect {
	//--------------------------------------------------------------
	void PCM::setup() {
		CHECK_ACTIVE;

		pcmThread.setup();
		drawPC = false;
		calibratedTex.allocate(gKinect->width, gKinect->height,GL_RGB); 
	}

	//--------------------------------------------------------------
	void PCM::exit() {
		CHECK_ACTIVE;

		pcmThread.exit();
	}

	//--------------------------------------------------------------
	void PCM::update(bool isKinectFrameNew) {
		CHECK_ACTIVE;

		if (isKinectFrameNew)
		{
			pcmThread.newFrameAvailable();
			if (IsFeatureActive(FEATURE_SHOW_RGB))
			{
				calibratedTex.loadData(gKinect->getCalibratedRGBPixels(),640,480,GL_RGB);
			}
		}
	}

	//--------------------------------------------------------------
	void PCM::draw() {
		CHECK_ACTIVE;

		ofResetColor();

		if(drawPC) {
			ofPushMatrix();
			ofTranslate(420, 320);
			// we need a proper camera class
			drawPointCloud();
			ofPopMatrix();
		}
		else {
			gKinect->getDepthTextureReference().draw(0, 0, -0.001f, KINECT_DEFAULT_WIDTH, KINECT_DEFAULT_HEIGHT);

			if(IsFeatureActive(FEATURE_SHOW_RGB))
			{
				ofEnableAlphaBlending();
				ofSetColor(255,255,255,128);
				calibratedTex.draw(0,0); 
				ofDisableAlphaBlending();
			}

			ofResetColor();
			ofPushMatrix();
			{
				gModel->objectsMutex.lock();
				for (vector<ModelObjectPtr>::const_iterator it = gModel->getObjects().begin(); it != gModel->getObjects().end(); it++)
				{
					(*it)->drawObject();
				}
				gModel->objectsMutex.unlock();

			}
			{
				gModel->tableMutex.lock();
				if(gModel->getTable().get() != NULL)
					gModel->getTable()->draw();	
				gModel->tableMutex.unlock();
			}

			ofPopMatrix();

			for (map<int, DataTouch>::const_iterator t = touchPoints.begin(); t != touchPoints.end(); ++t)
			{
				ofVec3f s(getScreenCoords(t->second.getTouchPoint()));
				ofSetColor(kRGBBlue);
				ofCircle(s.x, s.y, 3, 4);
				ofSetColor(kRGBRed);
				ofDrawBitmapString(ofToString(t->first), s.x, s.y);
			}
		}
	}

	//--------------------------------------------------------------
	void PCM::drawPointCloud() {
		CHECK_ACTIVE;

		ofScale(400, 400, 400);
		int w = KINECT_DEFAULT_WIDTH;
		int h = KINECT_DEFAULT_HEIGHT;

		ofRotateY(pointCloudRotationY);
		float* distancePixels = gKinect->getDistancePixels();

		glBegin(GL_POINTS);
		int step = 2;
		for(int y = 0; y < h; y += step) {
			for(int x = 0; x < w; x += step) {
				ofPoint cur = gKinect->getWorldCoordinateFor(x, y);
				ofColor color = gKinect->getCalibratedColorAt(x,y);
				glColor3ub((unsigned char)color.r,(unsigned char)color.g,(unsigned char)color.b);
				glVertex3f(cur.x, cur.y, cur.z);
			}
		}
		glEnd();

	}

	//--------------------------------------------------------------
	void PCM::keyPressed (int key) {
		CHECK_ACTIVE;

		switch (key) {
		case ' ':
			pcmThread.startDetection();
			break;
		case'p':
			drawPC = !drawPC;
			break;
		//case 's':
		//	savePC("test.pcd");
		//	break;
		case 'd':
			pcmThread.newForcedFrameAvailable();
			break;
		case 'r':
			pcmThread.reset();
			break;
		case 'l':
			printLogFile(kLogFilePCMThread);
			printLogFile(kLogFileObjectsThread);
			break;
		case 'k':
			printLogFileToFile(kLogFilePCMThread, "pcmThread.log");
			printLogFileToFile(kLogFileObjectsThread, "objectsThread.log");
		}
		//case 't':
		//	setTransformation();
		//	break;
	}

	//--------------------------------------------------------------
	void PCM::objectTouched(const IObjectPtr& object, const DataTouch& touchPoint)
	{
		switch (touchPoint.getType())
		{
		case kTouchTypeStarted:
		case kTouchTypeHolding:
			touchPoints[touchPoint.getId()] = touchPoint;
			break;
		case kTouchTypeReleased:
			touchPoints.erase(touchPoint.getId());
			break;
		}
	}

	//--------------------------------------------------------------
	bool PCM::isActive() {
		return IsFeaturePCMActive();
	}
}

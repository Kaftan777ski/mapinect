#include "mapinectApp.h"

#include "ofxFensterManager.h"

#include "ArmController.h"
#include "EventManager.h"
#include "Feature.h"
#include "Globals.h"
#include "log.h"
#include "Model.h"
#include "ofxKinect.h"
#include "pointUtils.h"
#include "ButtonManager.h"
#include "ModeManager.h"

namespace mapinect {

	//--------------------------------------------------------------
	mapinectApp::mapinectApp(ofxFenster* window, IApplication* app, VM* vm)
		: window(window), app(app), vm(vm)
	{
		LoadFeatures();
	}

	//--------------------------------------------------------------
	mapinectApp::~mapinectApp() {
		
	}

	//--------------------------------------------------------------
	void mapinectApp::setup() {
		gTransformation = new mapinect::Transformation();

		gKinect = new ofxKinect();
		if (IsFeatureKinectActive())
		{
			gKinect->init();
			gKinect->setVerbose(true);
			gKinect->open();
			// set Kinect looking down on startup
			angle = -28;
			//gKinect->setCameraTiltAngle(angle);
		}

		gModel = new mapinect::Model();

		ButtonManager* btnManager = new ButtonManager();
		app->btnManager = btnManager;

		ofSetWindowTitle("mapinect");
		cv.setup();
		pcm.setup(btnManager);
		arduino.setup();

		app->armController = new ArmController(&arduino);
		app->modeManager = new ModeManager(&pcm);
	}

	//--------------------------------------------------------------
	void mapinectApp::exit() {
		cv.exit();
		pcm.exit();
		arduino.exit();
		app->exit();

		if (IsFeatureKinectActive())
		{
			gKinect->close();
		}
	}

	//--------------------------------------------------------------
	void mapinectApp::update() {
		bool isKinectFrameNew = false;

		if (IsFeatureKinectActive())
		{
			gKinect->update();
			isKinectFrameNew = gKinect->isFrameNew();
		}

		cv.update(isKinectFrameNew);
		pcm.update(isKinectFrameNew);
		arduino.update();

	}

	//--------------------------------------------------------------
	void mapinectApp::draw()
	{
		window->setBackgroundColor(128, 128, 128);

		cv.draw();
		pcm.draw();
		
		app->debugDraw();
		
		ofSetHexColor(0);
		stringstream reportStream;
		reportStream
			<< "           fps: " << ofGetFrameRate() << endl
			<< "    pcm thread: " << getPCMThreadStatus() << endl
			<< "objects thread: " << getObjectsThreadStatus() << endl;
		ofDrawBitmapString(reportStream.str(), 20, 520);
	}

	//--------------------------------------------------------------
	void mapinectApp::keyPressed (int key) {

		bool debug = true;
		if (debug) {
			int angleVariation = 0;
			switch (key) {
			case OF_KEY_UP:
				angleVariation++;
				break;
			case OF_KEY_DOWN:
				angleVariation--;
				break;
			}
			if (angleVariation != 0) {
				angle = ofClamp(angle + angleVariation, -30, 30);
				if (IsFeatureKinectActive())
				{
					gKinect->setCameraTiltAngle(angle);
					printf("Current Kinect tilt angle: %d\n", angle);
				}
			}
		}		
		cv.keyPressed(key);
		pcm.keyPressed(key);
		arduino.keyPressed(key);
	}

	//--------------------------------------------------------------
	void mapinectApp::keyReleased(int key)
	{
	}

	//--------------------------------------------------------------
	void mapinectApp::windowMoved(int x, int y)
	{
	}

	//--------------------------------------------------------------
	void mapinectApp::mouseMoved(int x, int y)
	{
	}

	//--------------------------------------------------------------
	void mapinectApp::mouseDragged(int x, int y, int button)
	{
	}

	//--------------------------------------------------------------
	void mapinectApp::mousePressed(int x, int y, int button)
	{
	}

	//--------------------------------------------------------------
	void mapinectApp::mouseReleased(int x, int y, int button)
	{
	}

	//--------------------------------------------------------------
	void mapinectApp::dragEvent(ofDragInfo info)
	{
	}



	//--------------------------------------------------------------
	userApp::userApp(ofxFenster* window, IApplication* app)
		: window(window), app(app)
	{
		vm = new VM();
	}

	//--------------------------------------------------------------
	void userApp::exit()
	{
		app->exit();
	}

	//--------------------------------------------------------------
	void userApp::setup()
	{
		ofxFenster* win = ofxFensterManager::get()->createFenster(0, 0, 680, 600, OF_WINDOW);
		mapinectAppPtr = new mapinectApp(win, app, vm);
		win->addListener(mapinectAppPtr);
		mapinectAppPtr->setup();

		vm->setup();

		if (IsFeatureKinectActive())
		{
			double fx_d, fy_d, fx_rgb, fy_rgb; 
			float  cx_d, cy_d, cx_rgb, cy_rgb;
			ofVec3f T_rgb;
			ofMatrix4x4 R_rgb;
			getKinectCalibData(vm->getKinectCalibFile(), fx_d, fy_d, cx_d, cy_d,
										fx_rgb, fy_rgb, cx_rgb, cy_rgb, T_rgb, R_rgb);
			gKinect->getCalibration().setCalibValues( fx_d, fy_d, cx_d, cy_d,
										fx_rgb, fy_rgb, cx_rgb, cy_rgb, T_rgb, R_rgb);
		}

		app->setup();
		EventManager::suscribe(app);
		timer.start();
	}


	//--------------------------------------------------------------
	void userApp::draw()
	{
		window->setBackgroundColor(0, 0, 0);
		ofSetColor(255);
		
		if (gTransformation->getIsWorldTransformationStable()) 
		{
			// Dibujar mesa y objetos detectados
			vm->setupView();
			vm->draw();
			app->draw();								
			((ButtonManager*)app->btnManager)->draw();	
			vm->endView();
		}

	}

	//--------------------------------------------------------------
	void userApp::update()
	{
		EventManager::fireEvents();

		vm->update();

		float elapsedTime = timer.stopResumeAndGetElapsedSeconds();
		app->update(elapsedTime);
	}

	//--------------------------------------------------------------
	void userApp::keyPressed(int key)
	{
		vm->keyPressed(key);
		app->keyPressed(key);

		switch (key)
		{
			/*********************
			  TOGGLE FULLSCREEN	 - F11
			*********************/
		case OF_KEY_F11:
				window->setFullscreen(window->getWindowMode() != OF_FULLSCREEN);
				break;
		}
	}

	//--------------------------------------------------------------
	void userApp::keyReleased(int key)
	{
		vm->keyReleased(key);
		app->keyReleased(key);
	}

	//--------------------------------------------------------------
	void userApp::windowMoved(int x, int y)
	{
		vm->windowMoved(x, y);
		app->windowMoved(x, y);
	}

	//--------------------------------------------------------------
	void userApp::mouseMoved(int x, int y)
	{
		vm->mouseMoved(x, y);
		app->mouseMoved(x, y);
	}

	//--------------------------------------------------------------
	void userApp::mouseDragged(int x, int y, int button)
	{
		vm->mouseDragged(x, y, button);
		app->mouseDragged(x, y, button);
	}

	//--------------------------------------------------------------
	void userApp::mousePressed(int x, int y, int button)
	{
		vm->mousePressed(x, y, button);
		app->mousePressed(x, y, button);
	}

	//--------------------------------------------------------------
	void userApp::mouseReleased(int x, int y, int button)
	{
		vm->mouseReleased(x, y, button);
		app->mouseReleased(x, y, button);
	}

	//--------------------------------------------------------------
	void userApp::dragEvent(ofDragInfo info)
	{
		vm->dragEvent(info);
		app->dragEvent(info);
	}



}

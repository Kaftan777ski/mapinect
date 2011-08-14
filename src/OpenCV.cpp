#include "OpenCV.h"

#include "ofGraphicsUtils.h"

//--------------------------------------------------------------
void OpenCV::setup(ofxKinect *kinect) {
	this->kinect = kinect;

	colorImg.allocate(kinect->width, kinect->height);
	grayImage.allocate(kinect->width, kinect->height);
	grayThresh.allocate(kinect->width, kinect->height);
	grayThreshFar.allocate(kinect->width, kinect->height);
	grayBg.allocate(kinect->width, kinect->height);
	grayDiff.allocate(kinect->width, kinect->height);

	nearThreshold = 233;
	farThreshold  = 208;
	bThreshWithOpenCV = true;

	threshold = 80;
}

//--------------------------------------------------------------
void OpenCV::update(bool isKinectFrameNew) {

	if(isKinectFrameNew)
	{
		ofBackground(100, 100, 100);

		grayImage.setFromPixels(kinect->getDepthPixels(), kinect->width, kinect->height);

		//we do two thresholds - one for the far plane and one for the near plane
		//we then do a cvAnd to get the pixels which are a union of the two thresholds.	
		if( bThreshWithOpenCV ) {
			grayThreshFar = grayImage;
			grayThresh = grayImage;
			grayThresh.threshold(nearThreshold, true);
			grayThreshFar.threshold(farThreshold);
			cvAnd(grayThresh.getCvImage(), grayThreshFar.getCvImage(), grayImage.getCvImage(), NULL);
		}
		else{

			//or we do it ourselves - show people how they can work with the pixels

			unsigned char * pix = grayImage.getPixels();
			int numPixels = grayImage.getWidth() * grayImage.getHeight();

			for(int i = 0; i < numPixels; i++){
				if( pix[i] < nearThreshold && pix[i] > farThreshold ){
					pix[i] = 255;
				}else{
					pix[i] = 0;
				}
			}
		}

		//update the cv image
		grayImage.flagImageChanged();

		// find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
		// also, find holes is set to true so we will get interior contours as well....
		//contourFinder.findContours(grayImage, 10, (kinect->width*kinect->height)/2, 20, false);

		//for(int i = 0 ; i < contourFinder.nBlobs; i++)
		//{
		//	cout << "min: (" << contourFinder.blobs[i].boundingRect.x << ", " << contourFinder.blobs[i].boundingRect.x + contourFinder.blobs[i].boundingRect.y << ")" << endl;
		//	cout << "max: (" << contourFinder.blobs[i].boundingRect.x + contourFinder.blobs[i].boundingRect.width << ", " << contourFinder.blobs[i].boundingRect.x + contourFinder.blobs[i].boundingRect.y + contourFinder.blobs[i].boundingRect.height<< ")" << endl;
		//}

	}
}

//--------------------------------------------------------------
void OpenCV::draw() {
	ofResetColor();

	grayImage.draw(10, 320, 400, 300);

	//grayDiff.draw(420, 10, 400, 300);
	contourFinder.draw(10, 320, 400, 300);

	ofResetColor();
	stringstream reportStream;
	reportStream << "accel is: "
		<< ofToString(kinect->getMksAccel().x, 2) << " / "
		<< ofToString(kinect->getMksAccel().y, 2) << " / " 
		<< ofToString(kinect->getMksAccel().z, 2) << endl
		<< "press p to switch between images and point cloud, rotate the point cloud with the mouse" << endl
		<< "using opencv threshold = " << bThreshWithOpenCV <<" (press spacebar)" << endl
		<< "set near threshold " << nearThreshold << " (press: + -)" << endl
		<< "set far threshold " << farThreshold << " (press: < >) num blobs found " << contourFinder.nBlobs
		<< ", fps: " << ofGetFrameRate() << endl
		<< "press c to close the connection and o to open it again, connection is: " << kinect->isConnected() << endl;
	ofDrawBitmapString(reportStream.str(),20,666);
}

//--------------------------------------------------------------
void OpenCV::keyPressed (int key) {
	switch (key) {
	case '>':
	case '.':
		farThreshold ++;
		if (farThreshold > 255) farThreshold = 255;
		break;
	case '<':		
	case ',':		
		farThreshold --;
		if (farThreshold < 0) farThreshold = 0;
		break;

	case '+':
	case '=':
		nearThreshold ++;
		if (nearThreshold > 255) nearThreshold = 255;
		break;
	case '-':		
		nearThreshold --;
		if (nearThreshold < 0) nearThreshold = 0;
		break;
	}
}

//--------------------------------------------------------------
void OpenCV::mouseMoved(int x, int y)
{
}

//--------------------------------------------------------------
void OpenCV::mouseDragged(int x, int y, int button)
{
}

//--------------------------------------------------------------
void OpenCV::mousePressed(int x, int y, int button)
{
}

//--------------------------------------------------------------
void OpenCV::windowResized(int w, int h)
{
}

//--------------------------------------------------------------
void OpenCV::mouseReleased(int x, int y, int button)
{
}

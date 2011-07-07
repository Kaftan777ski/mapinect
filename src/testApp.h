#ifndef _TEST_APP
#define _TEST_APP
#define EIGEN_DONT_VECTORIZE
#define EIGEN_DISABLE_UNALIGNED_ARRAY_ASSERT

#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxKinect.h"
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/octree/octree.h>

#define KINECT_WIDTH 640
#define KINECT_HEIGHT 480
#define CLOUD_POINTS 307200 //640x480

using namespace pcl;

class testApp : public ofBaseApp {
	public:

		void setup();
		void update();
		void draw();
		void exit();
	
		void drawPointCloud();

		void keyPressed  (int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		void saveCloud();
		void savePartialCloud(ofPoint min, ofPoint max, int id);

		void captureBlobsClouds();
		void detectPlanes(PointCloud<PointXYZ>::Ptr currentCloud);
		void processBlobsClouds();
		PointCloud<PointXYZ>::Ptr getPartialCloud(ofPoint min, ofPoint max);
		PointCloud<PointXYZ>::Ptr getCloud();
		PointCloud<PointXYZRGB>::Ptr getColorCloud();
		PointCloud<PointXYZRGB>::Ptr getPartialColorCloud();
		void setInitialPointCloud();
		PointCloud<PointXYZ>::Ptr getDifferenceIdx(const PointCloud<PointXYZ>::Ptr &cloud, int noise_filter = 7);
		void processDiferencesClouds();

		ofxKinect kinect;

		ofxCvColorImage		colorImg;

		ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayThresh;
		ofxCvGrayscaleImage 	grayThreshFar;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;

        ofxCvContourFinder 	contourFinder;

		int 				threshold;
		bool				bLearnBakground;
		
		bool				bThreshWithOpenCV;
		bool				drawPC;

		int 				nearThreshold;
		int					farThreshold;

		int					angle;
		
		int 				pointCloudRotationY;


		pcl::PointCloud<pcl::PointXYZ>::Ptr	cloud;
		octree::OctreePointCloudChangeDetector<PointXYZ> *myoctree; //Valor de resolucion sacado del ejemplo
};

#endif
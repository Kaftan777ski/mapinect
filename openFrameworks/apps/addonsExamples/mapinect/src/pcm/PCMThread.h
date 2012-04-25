#ifndef PCM_THREAD_H__
#define PCM_THREAD_H__

#include "ofThread.h"
#include "ofTypes.h"
#include "ofVec3f.h"
#include "TrackedCloud.h"
#include <list>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/surface/convex_hull.h>
#include <pcl/octree/octree.h>
#include <pcl/registration/icp.h>
#include <pcl/registration/transformation_estimation.h>
#include <pcl/registration/transformation_estimation_svd.h>
#include <pcl/filters/passthrough.h>
#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/segmentation/extract_clusters.h>
#include "PCPolyhedron.h"
#include "PotentialHand.h"

using namespace pcl;

#define CLOUD_POINTS KINECT_WIDTH * KINECT_HEIGHT
#define MAX_OBJECTS 10

namespace mapinect {
	class PCMThread : ofThread {
	public:
		void						setup();
		void						exit();
		virtual void				threadedFunction();

		void						saveCloud(const string& name);
		void						savePartialCloud(ofPoint min, ofPoint max, int id, const string& name);
		PointCloud<PointXYZ>*		loadCloud(const string& name);

		PointCloud<PointXYZ>::Ptr	getPartialCloud(ofPoint min, ofPoint max);

		PointCloud<PointXYZ>::Ptr	cloud;
		PointCloud<PointXYZ>::Ptr	currentDiffcloud;
		octree::OctreePointCloudChangeDetector<PointXYZ>	*octree;

		void						setInitialPointCloud();
		PointCloud<PointXYZ>::Ptr	getDifferenceIdx(bool &dif, int noise_filter = 7);
		void						processDiferencesClouds();
		void						reset();

		bool						baseCloudSetted;
		float						timer;

		void						updateDetectedObjects();

		bool						detectMode;

	private:
		void						setPotentialHand(PointCloud<PointXYZ>::Ptr cluster);
		void						setPotentialHand(PointCloud<PointXYZ>::Ptr cluster, ofVec3f centroid);
		ofVec3f					normalEstimation(pcl::PointCloud<pcl::PointXYZ>::Ptr plane);
		ofVec3f					normalEstimation(pcl::PointCloud<pcl::PointXYZ>::Ptr plane, pcl::PointIndices::Ptr indicesptr);
		int							getSlotForTempObj();
		PointCloud<PointXYZ>::Ptr	getTableCluster();
		int							noDifferencesCount;
		bool						findBestFit(TrackedCloud* trackedCloud, TrackedCloud*& removedCloud, bool &removed);
		std::list<TrackedCloud>		trackedClouds;
		PCPolyhedron*				table;
		float						tableClusterLastDist;
		std::vector<PotentialHand>	potentialHands;
		bool						handSetted;
		

	};
}

#endif	// PCM_THREAD_H__

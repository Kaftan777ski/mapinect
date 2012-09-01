#include "PCMThread.h"

#include "Constants.h"
#include "Globals.h"
#include "log.h"
#include "pointUtils.h"
#include "PCPolyhedron.h"
#include "Frustum.h"
#include <pcl/octree/octree.h>

using namespace std;

#define WAIT_TIME_MS		40

namespace mapinect {

	//--------------------------------------------------------------
	void ObjectsThread::reset()
	{
		trackedClouds.clear();
		gModel->resetObjects();
	}

	//--------------------------------------------------------------
	void ObjectsThread::setup()
	{
		// Start the cloud processing thread
		startThread(true, false);
	}

	//--------------------------------------------------------------
	void ObjectsThread::exit()
	{
		stopThread();
	}

	//--------------------------------------------------------------
	void ObjectsThread::setClouds(const PCPtr& cloud)
	{
		inCloudMutex.lock();
		inCloud = cloud;
		inRawCloud = cloud;
		inCloudMutex.unlock();
	}

	//--------------------------------------------------------------
	void ObjectsThread::threadedFunction()
	{
		while (isThreadRunning()) {
			if (lock()) {
				
				bool newCloudAvailable = false;
				{
					inCloudMutex.lock();
					if (inCloud.get() != NULL && inRawCloud.get() != NULL )
						newCloudAvailable = true;
					inCloudMutex.unlock();

				}

				if(newCloudAvailable)
				{
					processCloud();
					setObjectsThreadStatus("");
				}
				
				unlock();
				ofSleepMillis(WAIT_TIME_MS);
			}
		}
	}
	
	//--------------------------------------------------------------
	bool countIsLessThanZero(const TrackedCloudPtr &trackedCloud)
	{
		bool condition = trackedCloud->getCounter() <= 0;
		return condition;
	}

	//--------------------------------------------------------------
	void ObjectsThread::processCloud()
	{
		PCPtr cloud;
		{
			inCloudMutex.lock();
			cloud = PCPtr(new PC(*inCloud));
			inCloud.reset();
			inCloudMutex.unlock();
		}

		// Updating temporal detections
		for (list<TrackedCloudPtr>::iterator iter = trackedClouds.begin(); iter != trackedClouds.end(); iter++) {			
			if((*iter)->hasObject())
			{
				PCPolyhedron* polyhedron = dynamic_cast<PCPolyhedron*>((*iter)->getTrackedObject().get());
				if (polyhedron != NULL)
				{
					// Est� dentro del frustum si todos sus v�rtices lo est�n
					(*iter)->setInViewField(Frustum::IsInFrustum(polyhedron->getVertexs()));
				}
			}
			if(!(*iter)->hasObject() || (*iter)->isInViewField())
			{
				(*iter)->addCounter(-1);
			}
		}

		int size = trackedClouds.size();
		trackedClouds.remove_if(countIsLessThanZero);
		int dif = size - trackedClouds.size(); 
		
		if(cloud->empty())
		{
			updateDetectedObjects();
			return;
		}

		list<TrackedCloudPtr> newClouds;
		int debugCounter = 0;

		{
				setObjectsThreadStatus("Detecting clusters...");
				saveCloud("rawClusters.pcd", *cloud);
				std::vector<pcl::PointIndices> clusterIndices =
				findClusters(cloud, Constants::OBJECT_CLUSTER_TOLERANCE(), Constants::OBJECT_CLUSTER_MIN_SIZE());

			for (std::vector<pcl::PointIndices>::const_iterator it = clusterIndices.begin (); it != clusterIndices.end (); ++it)
			{
				PCPtr cloudCluster = getCloudFromIndices(cloud, *it);

				saveCloud("objectsCluster" + ofToString(debugCounter) + ".pcd", *cloudCluster);

				newClouds.push_back(TrackedCloudPtr (new TrackedCloud(cloudCluster)));
				debugCounter++;
			}
		}

		// Look into the new clouds for the best fit
		list<TrackedCloudPtr> cloudsToMatch;
		list<TrackedCloudPtr> cloudsToAdd;

		debugCounter = 0;
		int maxIter = 10;
		setObjectsThreadStatus("Matching clusters with existing ones...");
		do
		{
			for (list<TrackedCloudPtr>::iterator iter = newClouds.begin(); iter != newClouds.end(); iter++)
			{
				//saveCloudAsFile("clusterInTable" + ofToString(debugCounter) + ".pcd", *(*iter)->getTrackedCloud());
				TrackedCloudPtr removedCloud;
				bool removed = false;
				bool fitted = findBestFit(*iter, removedCloud, removed);

				if (removed)
					cloudsToMatch.push_back(removedCloud);	// Push back the old cloud to try again
				if (!fitted)
					cloudsToAdd.push_back(*iter);			// No matching cloud, this will be a new object
				debugCounter++;
			}
			newClouds = cloudsToMatch;
			cloudsToMatch.clear();
			maxIter--;
		}
		while (newClouds.size() > 0 && maxIter > 0);

		// Effectuate the update of the tracked cloud with the new ones
		setObjectsThreadStatus("Update existing and new data...");
		updateDetectedObjects();

		trackedClouds.insert(trackedClouds.end(), cloudsToAdd.begin(), cloudsToAdd.end());
	}

	//--------------------------------------------------------------
	bool ObjectsThread::findBestFit(const TrackedCloudPtr& trackedCloud, TrackedCloudPtr& removedCloud, bool &removed) {
		float currentDist = numeric_limits<float>::max();
		TrackedCloudPtr currentMatch;
		removed = false;
		for (list<TrackedCloudPtr>::iterator iter = trackedClouds.begin(); iter != trackedClouds.end(); iter++) {
			// Chequea el matching contra todas las trackedClouds, est�n o no dentro del frustum
			float dist = (*iter)->matchingTrackedObjects(trackedCloud);
			if (dist != -1 && dist < currentDist)
			{
				currentMatch = (*iter);
				currentDist = dist;
			}
		}
		if(currentDist != numeric_limits<float>::max())
		{
			// Solo se confirma el matching, y se hace el update cuando el objeto que se viene trackeando est� dentro del cono de visi�n
			if(currentMatch->isInViewField())
				removed = currentMatch->confirmMatch(trackedCloud,removedCloud);
			return true;
		}
		return false;
	}

	//--------------------------------------------------------------
	vector<TrackedCloudPtr> ObjectsThread::computeOcclusions(const list<TrackedCloudPtr>& potentialOcclusions)
	{
		vector<TrackedCloudPtr> occlusions;

		ofVec3f origin = PCXYZ_OFVEC3F(eyePos());

		inCloudMutex.lock();
		PCPtr cloud = PCPtr(new PC(*inRawCloud));
		inRawCloud.reset();
		inCloudMutex.unlock();

		saveCloud("rawInternal.pcd",*cloud);
		pcl::octree::OctreePointCloudVoxelCentroid<pcl::PointXYZ> octree(Constants::CLOUD_VOXEL_SIZE*2);
		pcl::octree::OctreePointCloudVoxelCentroid<pcl::PointXYZ>::AlignedPointTVector voxelList;
		if(cloud->size() > 0)
		{
			octree.setInputCloud(cloud);
			octree.defineBoundingBox();
			octree.addPointsFromInputCloud();

			gModel->objectsMutex.lock();
			for (list<TrackedCloudPtr>::const_iterator iter = potentialOcclusions.begin(); iter != potentialOcclusions.end(); iter++) 
			{
				if((*iter)->hasObject())
				{
					bool occludedPol = true;
					PCPolyhedron* polyhedron = dynamic_cast<PCPolyhedron*>((*iter)->getTrackedObject().get());
					if(polyhedron->isValid())
					{
						polyhedron->resetOccludedFaces();
						vector<IPolygon*> pols = polyhedron->getPolygons();
						int occludedFaces = 0;
						for(int i = 0; i < pols.size(); i++)
						{
							vector<ofVec3f> vexs = pols.at(i)->getMathModel().getVertexs();
							int occludedVertexs = 0;

							for(int o = 0; o < vexs.size(); o++)
							{
								bool occludedVertex = false;
								ofVec3f end = vexs.at(o);
								Eigen::Vector3f endPoint(end.x,end.y,end.z);
								Eigen::Vector3f originPoint = PCXYZ_EIGEN3F(eyePos());
								voxelList.clear();

								int voxs = octree.getApproxIntersectedVoxelCentersBySegment(originPoint,endPoint,voxelList,Constants::CLOUD_VOXEL_SIZE*2);

								for(int i = 0; i < voxelList.size(); i ++)
								{
									if(octree.isVoxelOccupiedAtPoint(voxelList.at(i)))
									{
										ofVec3f intersect (voxelList.at(i).x,voxelList.at(i).y,voxelList.at(i).z);
										if(((intersect - end).length() > Constants::CLOUD_VOXEL_SIZE*5) &&
											(intersect - origin).length() < (end - origin).length())
											occludedVertexs++;
									}
								}
							}

							if(occludedVertexs > 3)
							{
								polyhedron->setOccludedFace(pols.at(i)->getName());
								occludedFaces++;
							}
						}
						if(occludedFaces > 1)
						{
							occlusions.push_back((*iter));
							//cout << "	occluded pol " << endl;
						}
					}
					
				}
			}
			gModel->objectsMutex.unlock();

		}
		return occlusions;
	}

	void ObjectsThread::updateDetectedObjects()
	{
		vector<TrackedCloudPtr> potentialOcclusions;
		vector<TrackedCloudPtr> occlusions = computeOcclusions(trackedClouds);

		if(occlusions.size() > 0)
		{
			for(int i = 0; i < occlusions.size(); i++)
			{
				if(!occlusions.at(i)->hasMatching())
					occlusions.at(i)->addCounter(1);
			}
		}

		for (list<TrackedCloudPtr>::iterator iter = trackedClouds.begin(); iter != trackedClouds.end(); iter++) {
			if ((*iter)->hasMatching() && (*iter)->isInViewField())
			{
				saveCloud("objPreMatched.pcd",*(*iter)->getTrackedCloud());
				(*iter)->updateMatching();
				saveCloud("objPostMatched.pcd",*(*iter)->getTrackedCloud());

				if (!((*iter)->hasObject())) {
					(*iter)->addCounter(2);
				}
				else
				{
					(*iter)->addCounter(1);
				}
				(*iter)->removeMatching();
			}
			/*else 
			{
				cout << "		no matching! " << (*iter)->getCounter() << endl;
				saveCloud("noMatched.pcd",*(*iter)->getTrackedCloud());
			}*/
		}
	}
}

#include "PCHand.h"

#include <pcl/ModelCoefficients.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>

#include "PCQuadrilateral.h"
#include "pointUtils.h"
#include "utils.h"
#include "ofxVecUtils.h"
#include "PCPolyhedron.h"
#include <pcl/filters/passthrough.h>
#include <pcl/surface/convex_hull.h>

#define MAX_FINGERS		5

namespace mapinect {

	PCHand::PCHand(PointCloud<PointXYZ>::Ptr cloud, PointCloud<PointXYZ>::Ptr extendedCloud, int objId)
				: PCModelObject(cloud, extendedCloud, objId)
	{
		drawPointCloud = false; 
		ofxVec3f v;
		Eigen::Vector4f eCenter;
		pcl::compute3DCentroid(*cloud,eCenter);
		
		v.x = eCenter[0];
		v.y = eCenter[1];
		v.z = eCenter[2];

		this->setCenter(v);
	}

	
	void PCHand::detectPrimitives() {
		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_projected (new pcl::PointCloud<pcl::PointXYZ>),
			cloud_in (new pcl::PointCloud<pcl::PointXYZ>(cloud));
  
		
		cloud_projected = cloud_in;
		//Elimino la componente z
		/*for(int i = 0; i < cloud_projected->points.size(); i++)
			cloud_projected->points[i].z = 0;*/




		  //pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
		  //pcl::PointIndices::Ptr inliers (new pcl::PointIndices);
		  //// Create the segmentation object
		  //pcl::SACSegmentation<pcl::PointXYZ> seg;
		  //// Optional
		  //seg.setOptimizeCoefficients (true);
		  //// Mandatory
		  //seg.setModelType (pcl::SACMODEL_PLANE);
		  //seg.setMethodType (pcl::SAC_RANSAC);
		  //seg.setDistanceThreshold (0.01);

		  //seg.setInputCloud (cloud_in);
		  //seg.segment (*inliers, *coefficients);

		  //// Project the model inliers 
		  //pcl::ProjectInliers<pcl::PointXYZ> proj;
		  //proj.setModelType (pcl::SACMODEL_PLANE);
		  //proj.setInputCloud (cloud_in);
		  //proj.setModelCoefficients (coefficients);
		  //proj.filter (*cloud_projected);

		  // Create a Convex Hull representation of the projected inliers
		  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_hull (new pcl::PointCloud<pcl::PointXYZ>);
		  pcl::ConvexHull<pcl::PointXYZ> chull;
		  chull.setInputCloud (cloud_projected);
		  chull.reconstruct (*cloud_hull);
		  //PCDWriter writer;
		  //writer.write<pcl::PointXYZ> ("handprojected.pcd", *cloud_projected, false);
		  //writer.write<pcl::PointXYZ> ("convexHull.pcd", *cloud_hull, false);


		  fingerTips.clear();
		  for(int i = 0; i < cloud_hull->size(); i++)
		  {
			  PointXYZ pto = cloud_hull->at(i);
			  fingerTips.push_back(ofxVec3f(pto.x,pto.y,pto.z));
		  }
		  
		  unifyVertexs();
	}

	void PCHand::unifyVertexs() {
		vector<vector<ofxVec3f>> tmp;
		list<ofxVec3f> final;
		if(gModel->table != NULL)
		{
			PCPolyhedron* hedron = (PCPolyhedron*)(gModel->table);
			PCPolygon* gon = hedron->getPCPolygon(0);
			pcl::ModelCoefficients coefficients = gon->getCoefficients();
			
			for (list<ofxVec3f>::iterator iter = fingerTips.begin(); iter != fingerTips.end(); iter++) {
				if(abs(evaluatePoint(coefficients, *iter)) < 0.03)     //<---------- Chequeo de la mesa
				{
					bool unified = false;
					for(int j = 0; j < tmp.size(); j++)
					{
						vector<ofxVec3f> inTmp = tmp.at(j);
						if(inTmp.size() > 0 && 
							inTmp.at(0).distance(*iter) < MAX_UNIFYING_DISTANCE_PROJECTION)
						{
							inTmp.push_back(*iter);
							unified = true;
						}
					}
					if(!unified)
					{
						vector<ofxVec3f> inTmp;
						inTmp.push_back(*iter);
						tmp.push_back(inTmp);
					}
				}
			}

			for(int i = 0; i < tmp.size(); i++)
			{
				vector<ofxVec3f> inTmp = tmp.at(i);
				ofxVec3f avg = ofxVec3f();
				int j;
				for(j = 0; j < inTmp.size(); j++)
					avg += inTmp.at(j);
				avg /= j;
				final.push_back(avg);
			}
		}

		fingerTips = final;
	}

	void PCHand::draw() {
		int i = 1;
		for (list<ofxVec3f>::iterator iter = fingerTips.begin(); iter != fingerTips.end(); iter++) {
			ofSetColor(100,255 * i++ / 4.0f,0);
			ofxVec3f w = gKinect->getScreenCoordsFromWorldCoords(*iter);
			ofCircle(w.x,w.y,4);
		}

		ofSetColor(255,200,0);
		ofxVec3f w = gKinect->getScreenCoordsFromWorldCoords(this->getCenter());
		ofCircle(w.x,w.y,10);
	}

	void PCHand::applyTransformation()
	{
		
	}

	void PCHand::resetLod() {
		
	}

	void PCHand::increaseLod() {
		
	}
}
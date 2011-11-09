#include "PCPolyhedron.h"

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

#define MAX_FACES		3

namespace mapinect {

	PCPolyhedron::PCPolyhedron(PointCloud<PointXYZ>::Ptr cloud, PointCloud<PointXYZ>::Ptr extendedCloud, int objId)
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

	bool hasNoMatching(PCPolygon* p) {
		return !(p->hasMatching());
	}

	void PCPolyhedron::detectPrimitives() {
		sensor_msgs::PointCloud2::Ptr cloud_blob (new sensor_msgs::PointCloud2());
		sensor_msgs::PointCloud2::Ptr cloud_filtered_blob (new sensor_msgs::PointCloud2);
		//pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered (new pcl::PointCloud<pcl::PointXYZ>)
		pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_p (new pcl::PointCloud<pcl::PointXYZ>);
		pcl::StatisticalOutlierRemoval<pcl::PointXYZ> sor;
		
		pcl::PointIndices::Ptr inliers (new pcl::PointIndices ());
		pcl::SACSegmentation<pcl::PointXYZ> seg;
		pcl::ExtractIndices<pcl::PointXYZ> extract;
		std::vector<ofxVec3f> vCloudHull;


		//PCDWriter writer;
		//writer.write<pcl::PointXYZ> ("cloud_p.pcd", cloud, false);

		//Remover outliers
		PointCloud<pcl::PointXYZ>::Ptr cloudTemp (new PointCloud<PointXYZ>(cloud));
		
		//sor.setInputCloud (cloudTemp);
		//sor.setMeanK (10); //Cantidad de vecinos a analizar
		//sor.setStddevMulThresh (1.0);
		//sor.filter (*cloud_filtered);

		//writer.write<pcl::PointXYZ> ("filtered.pcd", *cloud_filtered, false);

		// Optional
		seg.setOptimizeCoefficients (true);
		// Mandatory
		seg.setModelType (pcl::SACMODEL_PLANE);
		seg.setMethodType (pcl::SAC_RANSAC);
		seg.setMaxIterations (50);
		seg.setDistanceThreshold (0.009); //original: 0.01

		// Create the filtering object
		int i = 0, nr_points = cloudTemp->points.size ();
		// mientras 10% de la nube no se haya procesado

		/*
		for (int i = 0; i < pcpolygons.size(); i++) {
			delete pcpolygons[i];
		}
		pcpolygons.clear();
		*/
		vector<PCPolygon*> nuevos;

		int numFaces = 0;
		while (cloudTemp->points.size () > 0.07 * nr_points && numFaces < MAX_FACES)
		{
			pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());
			// Segment the largest planar component from the remaining cloud
			seg.setInputCloud (cloudTemp);
			seg.segment (*inliers, *coefficients);
			if (inliers->indices.size () == 0) {
				std::cerr << "Could not estimate a planar model for the given dataset." << std::endl;
				break;
			}

			//FIX
			pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered_temp_inliers (new pcl::PointCloud<pcl::PointXYZ>());
			pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_filtered_temp_outliers (new pcl::PointCloud<pcl::PointXYZ>());
			if(inliers->indices.size() != cloudTemp->size())
			{
				// Extract the inliers
				extract.setInputCloud (cloudTemp);
				extract.setIndices (inliers);
				extract.setNegative (false);
				extract.filter (*cloud_filtered_temp_inliers);
				cloud_p = cloud_filtered_temp_inliers;
			}
			else
				cloud_p = cloudTemp;
		
			// Create the filtering object
			extract.setInputCloud (cloudTemp);
			extract.setIndices (inliers);
			extract.setNegative (true);

		
			if(cloud_p->size() != cloudTemp->size())
				extract.filter (*cloud_filtered_temp_outliers);

			cloudTemp = cloud_filtered_temp_outliers;

			vCloudHull.clear();
			for (int k = 0; k < cloud_p->size(); k++) {
				vCloudHull.push_back(POINTXYZ_OFXVEC3F(cloud_p->at(k)));
			}
			PCPolygon* pcp = new PCQuadrilateral(*coefficients);
			static int polygonId = 0;
			pcp->setId(polygonId++);
			pcp->detectPolygon(cloud_p, vCloudHull);
			//detectedPlane.avgNormal = normalEstimation(cloud_p);
			PointCloud<PointXYZ>::Ptr cloud_pTemp (new PointCloud<PointXYZ>(*cloud_p));
			pcp->setCloud(cloud_pTemp);
			nuevos.push_back(pcp);
			
			//writer.write<pcl::PointXYZ> ("cloud_pTemp" + ofToString(i) + ".pcd", *cloud_pTemp, false);
			i++;
			numFaces++;
		}

		vector<PCPolygon*> aAgregar;
		vector<PCPolygon*> aProcesar;

		int remainingIters = 10;
		do {
			for (int i = 0; i < nuevos.size(); i++) {
				PCPolygon*	removed = NULL;
				bool		wasRemoved = false;
				bool		fitted = findBestFit(nuevos[i], removed, wasRemoved);

				if (wasRemoved) {
					aProcesar.push_back(removed);
				}
				if (!fitted) {
					aAgregar.push_back(nuevos[i]);
				}
			}
			nuevos = aProcesar;
			aProcesar.clear();
			remainingIters--;
		} while (nuevos.size() > 0 && remainingIters > 0);

		vector<PCPolygon*> keep;
		for (vector<PCPolygon*>::iterator iter = pcpolygons.begin(); iter != pcpolygons.end(); iter++) {
			if (!(*iter)->hasMatching()) {
				delete *iter;
			}
			else {
				keep.push_back(*iter);
			}
		}
		pcpolygons = keep;

		updatePolygons();

		for (int i = 0; i < aAgregar.size(); i++) {
			if (indexOf(pcpolygons, aAgregar[i]) < 0) {
				pcpolygons.push_back(aAgregar[i]);
			}
		}

		aAgregar.clear();

		unifyVertexs();
	}

	void PCPolyhedron::updatePolygons() {
		for (int i = 0; i < pcpolygons.size(); i++) {
			pcpolygons[i]->updateMatching();
		}
	}

	void PCPolyhedron::unifyVertexs() {
		typedef struct {
			PCPolygon*		pcp;
			int				vertex;
		} VertexInPCPolygon;
		vector<VertexInPCPolygon> updateVertexs;

		for (vector<PCPolygon*>::iterator nextIter = pcpolygons.begin(); nextIter != pcpolygons.end();) {
			vector<PCPolygon*>::iterator iter = nextIter++;
			Polygon* polygon = (*iter)->getPolygonModelObject();

			if (polygon == NULL) {
				// No model object is available yet, quit!
				return;
			}

			for (int j = 0; j < polygon->getVertexCount(); j++) {
				updateVertexs.clear();
				VertexInPCPolygon vpp;
				vpp.pcp = *iter;
				vpp.vertex = j;
				updateVertexs.push_back(vpp);
				ofxVec3f v(polygon->getVertex(j));

				for (vector<PCPolygon*>::iterator iter2 = iter; iter2 != pcpolygons.end(); iter2++) {
					Polygon* polygon2 = (*iter2)->getPolygonModelObject();
					for (int k = 0; k < polygon2->getVertexCount(); k++) {
						ofxVec3f v2(polygon2->getVertex(k));
						if (!(v == v2)
							&& polygon->getVertex(j).distance(polygon2->getVertex(k)) <= MAX_UNIFYING_DISTANCE) {
							VertexInPCPolygon vpp2;
							vpp2.pcp = *iter2;
							vpp2.vertex = k;
							updateVertexs.push_back(vpp2);
						}
					}
				}

				if (updateVertexs.size() > 1) {
					ofxVec3f avg(0, 0, 0);
					for (int i = 0; i < updateVertexs.size(); i++) {
						avg += updateVertexs.at(i).pcp->getPolygonModelObject()->getVertex(updateVertexs.at(i).vertex);
					}
					avg /= updateVertexs.size();
					for (int i = 0; i < updateVertexs.size(); i++) {
						updateVertexs.at(i).pcp->getPolygonModelObject()->setVertex(updateVertexs.at(i).vertex, avg);
					}
				}
			}
		}
	}

	bool PCPolyhedron::findBestFit(PCPolygon* polygon, PCPolygon*& removed, bool& wasRemoved)
	{
		for (int i = 0; i < pcpolygons.size(); i++) {
			if (pcpolygons[i]->matches(polygon, removed, wasRemoved)) {
				return true;
			}
		}
		return false;
	}

	//PCPolygon* PCPolyhedron::createPCPolygon() {
	//	return new PCQuadrilateral();
	//}

	void PCPolyhedron::draw() {
		for (vector<PCPolygon*>::iterator iter = pcpolygons.begin(); iter != pcpolygons.end(); iter++) {
			(*iter)->draw();
		}
	}

	void PCPolyhedron::applyTransformation()
	{
		PCModelObject::applyTransformation();
		for(vector<PCPolygon*>::iterator iter = pcpolygons.begin(); iter != pcpolygons.end(); iter++){
			(*iter)->applyTransformation(&transformation);
		}
	}

	PCPolygon*	PCPolyhedron::getPCPolygon(int index)
	{
		return pcpolygons[index];
	}

	int PCPolyhedron::getPCPolygonSize()
	{
		return pcpolygons.size();
	}

	void PCPolyhedron::resetLod() {
		PCModelObject::resetLod();
		for (int i = 0; i < pcpolygons.size(); i++) {
			pcpolygons[i]->resetLod();
		}
	}

	void PCPolyhedron::increaseLod() {
		PCModelObject::increaseLod();
		for(vector<PCPolygon*>::iterator iter = pcpolygons.begin(); iter != pcpolygons.end(); iter++){
			PointCloud<PointXYZ>::Ptr nuCloud (new PointCloud<PointXYZ>(cloud));
			(*iter)->increaseLod(nuCloud);
		}
		unifyVertexs();
	}
}

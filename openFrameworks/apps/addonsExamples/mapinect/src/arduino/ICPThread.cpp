#include "ICPThread.h"

#include <pcl/registration/icp.h>
#include <pcl/segmentation/sac_segmentation.h>
#include "Globals.h"
#include "pointUtils.h"
#include "Constants.h"
#include "Plane3D.h"

using namespace std;

#define WAIT_TIME_MS		20

namespace mapinect {

	static unsigned long startTime; 

	ICPThread::ICPThread()	
	{
		icpMutex.lock();
		checkApplyICP = false;
		icpMutex.unlock();
	}

	void ICPThread::reset() 	
	{
		icpMutex.lock();
		checkApplyICP = false;
		icpMutex.unlock();
	}

	void ICPThread::setup() {
		reset();

		startThread(true, false);
	}

	//--------------------------------------------------------------
	void ICPThread::exit() {
		stopThread();
	}

	//--------------------------------------------------------------
	void ICPThread::threadedFunction() {
		while (isThreadRunning()) {
			if (lock()) {
				
				bool applyICP = false;
				{
					icpMutex.lock();
						if (checkApplyICP && !(cloudBeforeMoving.get() == NULL) && !(cloudAfterMoving.get() == NULL)) 
						{
							applyICP = true;
						}
					icpMutex.unlock();
				}

				if(applyICP)
				{
					processICP();
				}
				
				unlock();
				ofSleepMillis(WAIT_TIME_MS);
			}
		}
	}

	void ICPThread::applyICP(const PCPtr& cloudBefore, const PCPtr& cloudAfter, int iterations) {
		icpMutex.lock();
			checkApplyICP = true;
			cloudBeforeMoving = cloudBefore;
			cloudAfterMoving = cloudAfter;
			maxIterations = iterations;
		icpMutex.unlock();
	}

	void ICPThread::processICP() {

		int iter = 0;
		icpMutex.lock();
			checkApplyICP = false;
			pcl::PointCloud<PCXYZ>::Ptr beforeMoving (new pcl::PointCloud<PCXYZ>(*cloudBeforeMoving.get()));
			saveCloud("ICPbeforeMoving.pcd",*beforeMoving);
			pcl::PointCloud<PCXYZ>::Ptr afterMoving  (new pcl::PointCloud<PCXYZ>(*cloudAfterMoving.get()));
			saveCloud("ICPafterMoving.pcd",*afterMoving);
			iter = maxIterations;
		icpMutex.unlock();

		// Verificar si la transformaci�n calculada de forma te�rica es ya una buena estimaci�n
		float maxAngleThreshold = 2; // In degrees
		bool isTableWellEstimated = goodTableEstimation(afterMoving, maxAngleThreshold);

		if (isTableWellEstimated) {
			cout << "La transformacion estimada es buena. No aplicar ICP" << endl;
		} else {
			cout << "Se necesita mejorar la transformacion" << endl;
			cout << "Aplicando ICP..." << endl;

			// Comienza procesamiento de ICP
			pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
			icp.setInputCloud(afterMoving);
			icp.setInputTarget(beforeMoving);

			// Set the max correspondence distance (correspondences with higher distances will be ignored)
			float maxDistance = 0.20; // In mts
			icp.setMaxCorrespondenceDistance (maxDistance * maxDistance);
			// Set the maximum number of iterations (criterion 1)
			icp.setMaximumIterations (iter);
			// Set the transformation epsilon (criterion 2)
			//icp.setTransformationEpsilon (1e-8);
			// Set the euclidean distance difference epsilon (criterion 3)
			//icp.setEuclideanFitnessEpsilon (1);

			pcl::PointCloud<pcl::PointXYZ> Final;

			startTime = ofGetSystemTime();

			icp.align(Final);

			unsigned int elapsedTime = (unsigned int) (ofGetSystemTime() - startTime);
			cout << "Tiempo de ICP: " << elapsedTime << endl;

			if (icp.hasConverged())
			{
				cout << "ICP convergio con fitness score igual a: " << icp.getFitnessScore() << endl;
				Eigen::Affine3f newTransf (icp.getFinalTransformation());
				
				PCPtr nubeAfterMovingTransfICP = transformCloud(afterMoving, newTransf);
				saveCloud("nubeAfterMovingTransfICP.pcd",*nubeAfterMovingTransfICP);

				// Verificar el resultado de ICP recalculando la mesa
				cout << "Verificando resultado de ICP, recalculando mesa" << endl;
				/* M�todo para detectar mesa - PCL SACSegmentation */
				float maxToleratedAngle = 10;
				bool isICPTableWellEstimated = goodTableEstimation(nubeAfterMovingTransfICP, maxToleratedAngle); // Sino usar goodTableEstimationRedetectingTable
				if (isICPTableWellEstimated) {
					cout << "Se aplica la transformacion de ICP" << endl;
					gTransformation->setWorldTransformation(newTransf * gTransformation->getWorldTransformation());
				} else {
					cout << "No se aplica la transformaci�n de ICP" << endl;				
				}			

			} else {
				cout << "No convergio ICP" << endl;
			}

		}	
		// Una vez que se termin� de aplicar ICP y se actualiz� la matriz de transformaci�n, 
		//	libero el mutex para que puedan invocar al m�todo getCloud
		gTransformation->cloudMutex.unlock();

		// Adem�s, se debe volver a dibujar en la ventana de mapping
		gTransformation->setIsWorldTransformationStable(true);
	}

	bool ICPThread::goodTableEstimation(PCPtr newCloud, float maxAngleThreshold) {
		// http://pointclouds.org/documentation/tutorials/planar_segmentation.php#planar-segmentation
		// http://www.pcl-users.org/Ransac-Planes-td2085912.html#a2086759

		cout << "GTE: Chequeando si la mesa detectada es buena "<< endl;
		cout << "	maxAngleThreshold = " << maxAngleThreshold << endl;

		pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients);
		pcl::PointIndices::Ptr inliers (new pcl::PointIndices);

		pcl::SACSegmentation<pcl::PointXYZ> seg;  
		seg.setOptimizeCoefficients (false);  
		seg.setModelType (pcl::SACMODEL_PERPENDICULAR_PLANE);  
		seg.setMethodType (pcl::SAC_RANSAC);  
		seg.setMaxIterations (100);  
		seg.setDistanceThreshold (0.01);  //  How close a point must be to the model in order to be considered an inlier
		ofVec3f tableNormal = gModel->getTable()->getNormal();
		cout << "Buscando planos con normal = (" << tableNormal.x << ", " << tableNormal.y << ", " << tableNormal.z << ")" << endl;
		seg.setAxis(Eigen::Vector3f(tableNormal.x,tableNormal.y,tableNormal.z));  
		float maxAngleThresholdRad = ofDegToRad(maxAngleThreshold);
		seg.setEpsAngle(maxAngleThresholdRad);

		seg.setInputCloud(newCloud);
		seg.segment(*inliers, *coefficients);

		Plane3D p(*coefficients);
		ofVec3f tableCentroid = gModel->getTable()->getCenter();
		Plane3D modelTablePlane(gModel->getTable()->getCoefficients());
		float centroidDistance = p.distance(tableCentroid);
		float normalDifference = abs(p.getNormal().angle(modelTablePlane.getNormal()));

		float MAX_ANGLE = 2;
		float MAX_TOLERATED_ANGLE = 10;
		float MAX_CENTROID_DISTANCE = 0.005;
		float MAX_TOLERATED_CENTROID_DISTANCE = 0.05; 

		// A veces la normal forma casi 180 grados con la buscada, es decir apunta en el sentido opuesto, pero es la mesa
		if (abs(180 - normalDifference) < MAX_TOLERATED_ANGLE) {
			normalDifference = abs(180 - normalDifference);	
		}

		if (inliers->indices.size () != 0) {
			cout << "Se detecto una mesa" << endl;
			if (normalDifference < MAX_ANGLE && centroidDistance < MAX_CENTROID_DISTANCE) {
				cout << "La mesa fue detectada correctamente. Fin GTE" << endl;
				return true;
			} else if (normalDifference < MAX_TOLERATED_ANGLE && centroidDistance < MAX_TOLERATED_CENTROID_DISTANCE) {
				cout << "La mesa detectada no es optima" << endl;
				// Tomo como mesa lo que se detect�			
				gModel->tableMutex.lock();
				TablePtr modelTable = gModel->getTable();
				if (modelTable != NULL) {
					modelTable->SetTablePlane(p.getCoefficients());
					cout << "Se pisa la mesa con la nueva detectada" << endl;
					cout << "Nueva normal = (" << gModel->getTable()->getNormal().x;
					cout << ", " << gModel->getTable()->getNormal().y << ", " << gModel->getTable()->getNormal().z << ")" << endl;
					cout << "Fin GTE" << endl;
				}
				gModel->tableMutex.unlock();
				return true;
			} else {
				cout << "La mesa detectada no es buena. Es un problema. Fin GTE" << endl;
				// TODO: evaluar si no hay que resetear todo y volver a calcular la transformacion de cero
				// Y resetear la mesa a la inicial (hay que guardarla)
				return false;
			}
		}

		cout << "No se detecto mesa. Fin de GTE" << endl;
		return false;
		
	}

	bool ICPThread::goodTableEstimationRedetectingTable(PCPtr nubeAfterMovingTransfICP)
	{
		/* M�todo 1 para detectar mesa - Clusters, extraer el plano m�s grande */
		int CLOUD_DENSITY = 4;
		startTime = ofGetSystemTime();

		float CLOUD_VOXEL_SIZE = 0.01;
		float TABLE_CLUSTER_TOLERANCE_FACTOR = 1.5;
		float NEW_TABLE_CLUSTER_TOLERANCE	= CLOUD_VOXEL_SIZE * TABLE_CLUSTER_TOLERANCE_FACTOR;
		float CLOUD_POINTS = KINECT_DEFAULT_WIDTH * KINECT_DEFAULT_HEIGHT * (1.0f / CLOUD_DENSITY);
		float TABLE_CLUSTER_MIN_PERCENT = 0.02;
		float NEW_TABLE_CLUSTER_MIN_SIZE = CLOUD_POINTS * TABLE_CLUSTER_MIN_PERCENT;
		std::vector<pcl::PointIndices> cluster_indices =
			findClusters(nubeAfterMovingTransfICP, NEW_TABLE_CLUSTER_TOLERANCE, NEW_TABLE_CLUSTER_MIN_SIZE);
		if (cluster_indices.size() <= 0) {
			cout << "No se encontraron clusters" << endl;
		} else {
			PCPtr tableCluster;
			float minDistanceToCentroid = MAX_FLOAT;
			for (std::vector<pcl::PointIndices>::const_iterator it = cluster_indices.begin (); it != cluster_indices.end (); ++it)
			{
				PCPtr cloud_cluster = getCloudFromIndices(nubeAfterMovingTransfICP, *it);
				ofVec3f ptoCentroid = computeCentroid(cloud_cluster);
				if (ptoCentroid.squareLength() < minDistanceToCentroid)
				{
					minDistanceToCentroid = ptoCentroid.squareLength();
					tableCluster = cloud_cluster;
				}
			}
			pcl::ModelCoefficients coefficients;
			PCPtr result(new PC());
			PCPtr biggestPlane = extractBiggestPlane(tableCluster, coefficients, result, 0.009);
			if (biggestPlane->size() <= 0) {
				cout << "No se pudo encontrar el plano m�s grande" << endl;
			} else {
				cout << "Se obtuvo un cluster que parece ser mesa" << endl;
				cout << "Verificando condiciones..." << endl;
				// Aplicar criterios para determinar si es la mesa o no			
				// 1 - Calculo el promedio de la coordenada Y de la mesa del modelo, y del cluster que supuestamente es mesa
				float promedioCoordenadaYTable = 0;
				int cantPuntos = gModel->getTable()->getCloud()->size();
				for (int i=0; i < cantPuntos ; i++) {
					promedioCoordenadaYTable += gModel->getTable()->getCloud()->points[i].y;
				}
				promedioCoordenadaYTable /= cantPuntos;
				float promedioCoordenadaY = 0;
				for (int i=0; i < biggestPlane->size() ; i++) {
					promedioCoordenadaY += biggestPlane->points[i].y;
				}
				promedioCoordenadaY /= biggestPlane->size();
				// 2 - Obtengo las normales 
				ofVec3f tableNormal = gModel->getTable()->getNormal();
				ofVec3f biggestPlaneNormal = getNormal(coefficients);
				// 3 - Si la diferencia entre el promedioY es cercano a cero y las normales son paralelas
				//	entonces es una buena aproximaci�n de la mesa
				//	Dos vectores A y B son normales si el producto punto es A*B => A.B = A*B*cos(angulo) y si angulo tiende a 0 -> A*B
				float MAX_DISTANCE = 0.05; // 5 cms
				float MAX_FOR_PARALLEL = 0.07;
				float yNubeTransformada = abs(promedioCoordenadaY);
				float yMesa = abs(promedioCoordenadaYTable);
				if ( (abs(yNubeTransformada - yMesa)  < MAX_DISTANCE) && 
					(abs(tableNormal.dot(biggestPlaneNormal) - tableNormal.length()*biggestPlaneNormal.length()) <= MAX_FOR_PARALLEL) ) {
					// Se cumplen los criterios, asumo que es mesa
					cout << "El cluster encontrado es mesa" << endl;
					return true;
					// Resetear la mesa?
				} else {
					cout << "No es mesa, se cumpli� alguna condici�n" << endl;
					return false;
					// No se cumplen los criterios
					//	Puede ocurrir por que no se pudo detectar la mesa (muchos objectos encima, etc)
					//	O por que la transformaci�n que calcul� ICP no es buena
				}				
			}
		}
		unsigned int elapsedTime = (unsigned int) (ofGetSystemTime() - startTime);
		cout << "Tiempo para verificar ICP re detectando mesa: " << elapsedTime << endl;
		
		/* Fin del M�todo 1 para detectar mesa - Clusters, extraer el plano m�s grande */
	}


}
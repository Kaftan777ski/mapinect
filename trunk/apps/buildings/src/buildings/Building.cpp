#include "Building.h"

#include "ofxVecUtils.h"

namespace buildings {

	GLuint Building::buildingTexture = 0;
	GLuint Building::roofTexture = 0;

	Building::Building(int id, PCPolyhedron* polyhedron) {
		this->id = id;
		this->polyhedron = polyhedron;
		progress = 0;
	}

	Building::~Building() {

	}

	void Building::draw(const ITxManager* txManager, const Floor* floor)
	{
		if (progress < 1.0f) {
			progress += 0.002f;
		}

		for (int i = 0; i < polyhedron->getPCPolygonSize(); i++)
		{
			PCPolygon* polygon = polyhedron->getPCPolygon(i);
			if (polygon->hasObject()) 
			{
				txManager->enableTextures();
				mapinect::Polygon* q = polygon->getPolygonModelObject();

				static ofxVec3f vA,vB,vC,vD;

				vA = q->getVertex(0);
				vB = q->getVertex(1);
				vC = q->getVertex(2);
				vD = q->getVertex(3);

				ofxVec3f floorNormal = floor->getModelObject()->getNormal();
				float prod = abs(q->getNormal().dot(floorNormal));
				if (prod < 0.9)
				{
					std::vector<ofxVec3f> pts_;
			
					pts_.push_back(vA);
					pts_.push_back(vB);
					pts_.push_back(vC);
					pts_.push_back(vD);

					sort(pts_.begin(), pts_.end(), sortOn_y<ofPoint>);

					/*	__________
						|2		3|
						|		 |
						|		 |
						|1		0|
					*/

					std::vector<ofxVec3f> pts_b;
					pts_b.push_back(pts_[2]);
					pts_b.push_back(pts_[3]);

					sort(pts_b.begin(), pts_b.end(), sortOn_x<ofPoint>);

					std::vector<ofxVec3f> pts_a;
					pts_a.push_back(pts_[2]);
					pts_a.push_back(pts_[3]);

					sort(pts_a.begin(), pts_a.end(), sortOn_x<ofPoint>);

					ofxVec3f v0 = pts_b[1];
					ofxVec3f v1 = pts_b[0];
					ofxVec3f v2 = pts_a[0];
					ofxVec3f v3 = pts_a[1];

					float h12 = std::abs(v2.y - v1.y);
					float h03 = std::abs(v3.y - v0.y);			
			
					// Bind Texture
					txManager->bindTexture(Building::buildingTexture);

					ofxVec3f v3p(v3.x, v0.y + progress * h03, v3.z);
					ofxVec3f v2p(v2.x, v1.y + progress * h12, v2.z);
					ofDrawQuadTextured(v1, v0, v3p, v2p, 0, 0, progress, 0, progress, progress, 0, progress);

				}
				else {
					txManager->bindTexture(Building::roofTexture);
					ofDrawQuadTextured(vA, vB, vC, vD);
				}
				txManager->disableTextures();
			}
		}
	}

	void Building::update() {
	
	}

}
#include "utils.h"

ofxKinect			*gKinect = 0;
mapinect::Model	*gModel = 0;

float				OCTREE_RES;
int					MIN_DIFF_TO_PROCESS;
int					QUAD_HALO;
int					DIFF_THRESHOLD;
float				RES_IN_OBJ;
float				RES_IN_OBJ2;
int					DIFF_IN_OBJ;
int					TIMES_TO_CREATE_OBJ;
float				MIN_DIF_PERCENT;
int					TIMES_TO_STABILIZE;
float				MAX_Z;
float				NORMAL_ESTIMATION_PERCENT;
int					MAX_CLUSTER_SIZE;
int					MIN_CLUSTER_SIZE;
float				MAX_CLUSTER_TOLERANCE;
int					MAX_TABLE_CLUSTER_SIZE;
int					MIN_TABLE_CLUSTER_SIZE;
float				MAX_TABLE_CLUSTER_TOLERANCE;
int					CLOUD_RES;
float				TRANSLATION_DISTANCE_TOLERANCE;
int					MAX_OBJ_LOD;
int					objId;
float				MAX_UNIFYING_DISTANCE;
int					KINECT_WIDTH;
int					KINECT_HEIGHT;
int					KINECT_WIDTH_OFFSET;
int					KINECT_HEIGHT_OFFSET;
float				MAX_UNIFYING_DISTANCE_PROJECTION;

ofPolar cartesianToPolar(const ofPoint& c)
{
	ofPolar p;
	p.ro = sqrt(c.x * c.x + c.y * c.y);
	p.theta = 0;
	if (c.x != 0 || c.y != 0) {
		if (c.x == 0) {
			p.theta = c.y > 0 ? PI / 2 : - PI / 2;
		}
		else {
			p.theta = atan(c.y / c.x);
			if (c.x < 0) {
				p.theta += PI;
			}
		}
	}
	return p;
}

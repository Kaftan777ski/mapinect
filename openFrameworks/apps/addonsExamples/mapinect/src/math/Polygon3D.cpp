#include "Polygon3D.h"

namespace mapinect
{

	Polygon3D::Polygon3D(const vector<ofVec3f>& vertexs)
		: vertexs(vertexs), plane(vertexs[0],vertexs[1],vertexs[2])
	{
		assert(vertexs.size() >= 2);
		init();
	}

	Polygon3D::Polygon3D(const Plane3D& plane, const vector<ofVec3f>& vs)
		: plane(plane)
	{
		for (int i = 0; i < vs.size(); i++)
		{
			vertexs.push_back(plane.project(vs[i]));
		}
		init();
	}

	Polygon3D::Polygon3D(const Polygon3D& polygon)
		: vertexs(polygon.vertexs), vertexs2d(polygon.vertexs2d), discardCoord(polygon.discardCoord),
			plane(polygon.plane), edges(polygon.edges), edges2d(polygon.edges2d)
	{
	}

	void Polygon3D::init()
	{
		discardCoord = calculateDiscardCoordinate(vertexs);
		for (vector<ofVec3f>::const_iterator it = vertexs.begin(); it != vertexs.end(); ++it)
		{
			this->vertexs2d.push_back(discardCoordinateOfVec3f(*it, discardCoord));
		}
		for (int i = 0; i < vertexs.size(); i++)
		{
			edges.push_back(Line3D(vertexs[i], vertexs[(i + 1) % vertexs.size()]));
			edges2d.push_back(Line2D(vertexs2d[i], vertexs2d[(i + 1) % vertexs.size()]));
		}
	}

	float Polygon3D::distance(const ofVec3f& p) const
	{
		return p.distance(project(p));
	}

	ofVec3f Polygon3D::project(const ofVec3f& p) const
	{
		ofVec3f planeProjected(plane.project(p));

		if (isInPolygon(planeProjected))
			return planeProjected;

		float minDistance = MAX_FLOAT;
		ofVec3f result;
		for (int i = 0; i < edges.size(); i++)
		{
			float k = edges[i].projectedK(p);
			ofVec3f edgeProjected = edges[i].calculateValue(k);
			float distance = p.distance(edgeProjected);
			if (edges[i].isInSegment(k) && distance < minDistance)
			{
				minDistance = distance;
				result = edgeProjected;
			}
		}
		for (int i = 0; i < vertexs.size(); i++)
		{
			float distance = p.distance(vertexs[i]);
			if (distance < minDistance)
			{
				minDistance = distance;
				result = vertexs[i];
			}
		}

		return result;
	}

	bool Polygon3D::isInPolygon(const ofVec3f& p) const
	{
		if (plane.distance(p) < MATH_EPSILON)
		{
			ofVec3f projected(plane.project(p));
			PositionToLine position = edges2d[0].positionTo(discardCoordinateOfVec3f(projected, discardCoord));
			for (int i = 1; i < edges2d.size(); i++)
			{
				PositionToLine positionI = edges2d[i].positionTo(discardCoordinateOfVec3f(projected, discardCoord));
				if (positionI != position)
				{
					if (position == kPositionedInLine)
					{
						position = positionI;
					}
					else
					{
						return false;
					}
				}
			}
			return true;
		}
		return false;
	}

	float Polygon3D::calculateArea() const
	{
		// Code taken from http://softsurfer.com/Archive/algorithm_0101/algorithm_0101.htm#area3D_Polygon()
		float area = 0;
		int   i, j, k;			// loop indices

		vector<ofVec3f> auxVertexs(vertexs);
		auxVertexs.push_back(auxVertexs.at(0));
		auxVertexs.push_back(auxVertexs.at(1));
		
		// compute area of the 2D projection
		for (i = 1, j = 2, k = 0; i <= auxVertexs.size() - 2; i++, j++, k++) {
			switch (discardCoord) {
			case kDiscardCoordinateX:
				area += (auxVertexs[i].y * (auxVertexs[j].z - auxVertexs[k].z));
				continue;
			case kDiscardCoordinateY:
				area += (auxVertexs[i].x * (auxVertexs[j].z - auxVertexs[k].z));
				continue;
			case kDiscardCoordinateZ:
				area += (auxVertexs[i].x * (auxVertexs[j].y - auxVertexs[k].y));
				continue;
			}
		}

		// scale to get area before projection
		switch (discardCoord) {
		case kDiscardCoordinateX:
			area *= (1.0f / (2.0f * plane.getNormal().x));
			break;
		case kDiscardCoordinateY:
			area *= (1.0f / (2.0f * plane.getNormal().y));
			break;
		case kDiscardCoordinateZ:
			area *= (1.0f / (2.0f * plane.getNormal().z));
		}
		return abs(area);
	}

}
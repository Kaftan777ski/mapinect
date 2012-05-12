#ifndef MAPINECT_POLYGON_H__
#define MAPINECT_POLYGON_H__

#include "ModelObject.h"
#include "IPolygon.h"

namespace mapinect {

	class Polygon;

	typedef boost::shared_ptr<Polygon> PolygonPtr;

	class Polygon : public ModelObject, public IPolygon {
		public:
			Polygon() : vertexs() { }
			virtual ~Polygon() { }

			const ofVec3f&		getVertex(int vertexNum);
			void				setVertex(int vertexNum, const ofVec3f& v);
			void				addVertex(const ofVec3f&);
			inline int			getVertexCount()			{ return vertexs.size(); }
			inline const vector<ofVec3f>&	getVertexs()	{ return vertexs; }
			inline const ofVec3f&	getNormal()				{ return normal; }

			void				sortVertexs();

			float				calculateArea();
			ofVec3f				project(const ofVec3f&);
			void				resetVertex();
			virtual void		draw();
		
		private:
			vector<ofVec3f>		vertexs;
			ofVec3f				normal;
	};
}

#endif	// MAPINECT_POLYGON_H__
#ifndef MAPINECT_PC_POLYHEDRON_H__
#define MAPINECT_PC_POLYHEDRON_H__

#include "PCModelObject.h"
#include "IObject.h"

#include "Polyhedron.h"
#include "PCPolygon.h"
#include "Vertex.h"
#include <list>

namespace mapinect {

	class PCPolyhedron;

	typedef boost::shared_ptr<PCPolyhedron> PCPolyhedronPtr;

	class PCPolyhedron : public PCModelObject, public IObject {
		public:
			PCPolyhedron(const PCPtr& cloud, int objId);
			
			virtual void			draw();
			virtual void			detectPrimitives();
			const PCPolygonPtr&		getPCPolygon(int index);
			int						getPCPolygonSize();
			virtual void			resetLod();
			virtual void			increaseLod(const PCPtr& nuCloud);
			virtual void			addToModel(const PCPtr& nuCloud);
			virtual void			setAndUpdateCloud(const PCPtr& cloud);

			vector<Polygon3D>		getMathModelApproximation() const;

			inline int						getId()							{ return PCModelObject::getId(); }

			inline const ofVec3f&			getCenter()						{ return PCModelObject::getCenter(); }
			inline const ofVec3f&			getScale()						{ return PCModelObject::getScale(); }
			inline const ofVec3f&			getRotation()					{ return PCModelObject::getRotation(); }

			const IPolygon*					getPolygon(const IPolygonName&);
			inline const vector<IPolygon*>	getPolygons()					{ return polygonsCache; }
			//vector<Vertex&>					getVertexs();
		private:
			void							updatePolygons();
			bool							findBestFit(const PCPolygonPtr&, PCPolygonPtr& removed, bool& wasRemoved);
			vector<PCPolygonPtr>	detectPolygons(const PCPtr& cloudTemp, float planeTolerance = 0.01, float pointsTolerance = 4.0, bool limitFaces = true);
			vector<PCPolygonPtr>			mergePolygons(vector<PCPolygonPtr>& toMerge);
			virtual vector<PCPolygonPtr>	estimateHiddenPolygons(const vector<PCPolygonPtr>& newPolygons);
			virtual vector<PCPolygonPtr>	discardPolygonsOutOfBox(const vector<PCPolygonPtr>& toDiscard);
			virtual void					namePolygons(vector<PCPolygonPtr>& toName);
			
		protected:
			vector<PCPolygonPtr>			pcpolygons;
			vector<IPolygon*>				polygonsCache;
			//vector<Vertex&>					vertexs;
			bool							partialEstimation;
			virtual void					unifyVertexs();

	};
}

#endif	// MAPINECT_PC_POLYHEDRON_H__
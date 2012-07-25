#ifndef MAPINECT_PC_BOX_H__
#define MAPINECT_PC_BOX_H__

#include "PCPolyhedron.h"


namespace mapinect {

	class PCBox;

	typedef boost::shared_ptr<PCBox> PCBoxPtr;

	class PCBox : public PCPolyhedron{
		public:
			PCBox(const PCPtr& cloud, int objId = -1);
			
			virtual void			detectPrimitives();
			//virtual void			addToModel(const PCPtr& nuCloud);

			//BoxVertex&				getVertex(BoxVertexName v);

		private:
			virtual void					unifyVertexs();
			PCPolygonPtr					duplicatePol(const PCPolygonPtr& polygon,const vector<PCPolygonPtr>& newPolygons);
			PCPolygonPtr					duplicatePol(const PCPolygonPtr& polygon,const map<IPolygonName,PCPolygonPtr>& estimated);
			virtual vector<PCPolygonPtr>	estimateHiddenPolygons(const vector<PCPolygonPtr>& newPolygons, bool& estimationOk);
			virtual vector<PCPolygonPtr>	mergePolygons(vector<PCPolygonPtr>& toMerge);

			virtual vector<PCPolygonPtr>	discardPolygonsOutOfBox(const vector<PCPolygonPtr>& toDiscard);
			virtual vector<PCPolygonPtr>	discardPolygonsOutOfBox(const vector<PCPolygonPtr>& toDiscard, const vector<PCPolygonPtr>& inPolygon);
			list<IPolygonName>				getMissing(const vector<PCPolygonPtr>& estimated);
			list<IPolygonName>				getMissing(const map<IPolygonName,PCPolygonPtr>& estimated);
			PCPolygonPtr					getNextPolygon(IPolygonName toEstimate, const map<IPolygonName,PCPolygonPtr>& estimated);
			PCPolygonPtr					getPrevPolygon(IPolygonName toEstimate, const map<IPolygonName,PCPolygonPtr>& estimated);
			PCPolygonPtr					getOppositePolygon(IPolygonName toEstimate, const map<IPolygonName,PCPolygonPtr>& estimated);
			PCPolygonPtr					getPCPolygon(IPolygonName name, const map<IPolygonName,PCPolygonPtr>& estimated);
			PCPolygonPtr					getPCPolygon(IPolygonName name, const vector<PCPolygonPtr>& estimated);
			IPolygonName					getOppositePolygonName(IPolygonName toEstimate);
			IPolygonName					getPrevPolygonName(IPolygonName toEstimate);
			IPolygonName					getNextPolygonName(IPolygonName toEstimate);


			PCPolygonPtr					top;
			PCPolygonPtr					sideA;
			PCPolygonPtr					sideB;
			PCPolygonPtr					sideC;
			PCPolygonPtr					sideD;
			PCPolygonPtr					bottom;

			void							messureBox();
			float							width;			//Respecto a la cara A
			float							height;			//Respecto a la cara A
			float							depth;			//Respecto a la cara B
			bool							messured;

	};
}

#endif	// MAPINECT_PC_BOX_H__
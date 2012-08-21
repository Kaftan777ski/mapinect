#ifndef ICP_THREAD_H__
#define ICP_THREAD_H__

#include "mapinectTypes.h"
#include "ofThread.h"
#include "ofxMutex.h"

namespace mapinect
{
	class ICPThread : ofThread
	{
	public:
		ICPThread();

		void						reset();
		void						setup();
		void						exit();
		virtual void				threadedFunction();

		void						applyICP(const PCPtr& cloudBefore, const PCPtr& cloudAfter, int maxIterations);

		void						processICP();

	private:
		bool						checkApplyICP;
		PCPtr						cloudBeforeMoving;
		PCPtr						cloudAfterMoving;
		int							maxIterations;

		ofxMutex					icpMutex;	

		bool						findNewTablePlane(const PCPtr& cloud, float maxAngleThreshold, float maxDistance, 
											pcl::ModelCoefficients& coefficients, PCPtr& planeCloud);
		bool						icpProcessing(const PCPtr& inputCloud, const PCPtr& inputTarget, Eigen::Affine3f& newTransf, 
											float maxDistance = 0.20, int maxIterations = 30);


	};
}

#endif	// ICP_THREAD_H__
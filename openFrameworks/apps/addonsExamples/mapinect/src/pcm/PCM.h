#ifndef PCM_H__
#define PCM_H__

#include "PCMThread.h"
#include "ofGraphicsUtils.h"
#include "ofMain.h"

namespace mapinect {
	class PCM {
	public:

		virtual void		setup();
		virtual void		exit();
		virtual void		update(bool isKinectFrameNew);
		virtual void		draw();

		virtual void		keyPressed(int key);

		virtual void		objectTouched(const IObjectPtr&, const DataTouch&);

		void				drawPointCloud();

	private:
		bool				isActive();
		ofTexture			calibratedTex; 
		PCMThread			pcmThread;
		bool				drawPC;
		int 				pointCloudRotationY;
		map<int, DataTouch>	touchPoints;

	};
}

#endif	// PCM_H__

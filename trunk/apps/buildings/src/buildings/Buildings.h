#ifndef BUILDINGS_H__
#define BUILDINGS_H__

#include "IApplication.h"

#include "Floor.h"
#include "Building.h"

namespace buildings {

	class Buildings : public IApplication {
	public:
		Buildings();
		virtual ~Buildings();

		virtual void setup();
		virtual void update();
		virtual void draw();
		virtual void exit();

		virtual void keyPressed(int key);
		virtual void mouseMoved(int x, int y );
		virtual void mouseDragged(int x, int y, int button);
		virtual void mousePressed(int x, int y, int button);
		virtual void mouseReleased(int x, int y, int button);
		virtual void windowResized(int w, int h);

		virtual void debugDraw();

	private:
		std::map<int, Building*>	buildings;
		Floor*						floor;
	};
}

#endif	// BUILDINGS_H__
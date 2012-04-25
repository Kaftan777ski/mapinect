#ifndef BUILDING_H__
#define BUILDING_H__

#include "PCPolyhedron.h"
#include "ITxManager.h"
#include "Floor.h"s

using namespace mapinect;

namespace buildings {

	class Building {
	public:
		Building(int id, PCPolyhedron* polyhedron);
		virtual ~Building();

		void update();
		void draw(const ITxManager* txManager, const Floor* floor);

		static GLuint	buildingTexture;
		static GLuint	roofTexture;

	private:
		int				id;
		float			progress;
		PCPolyhedron*	polyhedron;
	};
}

#endif	// BUILDING_H__

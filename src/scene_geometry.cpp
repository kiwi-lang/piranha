#include <scene_geometry.h>

manta::SceneGeometry::SceneGeometry() {
	m_id = -1;
}

manta::SceneGeometry::~SceneGeometry() {

}

int manta::SceneGeometry::getId() {
	return m_id;
}

void manta::SceneGeometry::setId(int id) {
	m_id = id;
}


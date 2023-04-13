#pragma once

#include "../../ccb/gfx/material3d.h"

#include "../struct/feature_base.h"
#include "../struct/world_structures.h"

class CasinoSpike : public Scene
{
public:

	// construction
	CasinoSpike(CascabelBaseFeature* ccbf);
	~CasinoSpike() {  }

	// draw
	virtual void render(Camera3D &cam3D);

private:

	// render information
	CascabelBaseFeature* m_ccbf;
	uint16_t index_r3D;
	glm::vec3 mv_pos = glm::vec3(1,0,-2);
	Material3D m0,m1;

	// camera
	glm::vec3 cp_pos = glm::vec3(0,5,-5),cp_dir = glm::vec3(0,-.05f,.05f);
	float pitch = 0,yaw = 45.0f;
};
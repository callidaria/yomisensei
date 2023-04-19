#pragma once

#include "../../ccb/gfx/shader.h"
#include "../../ccb/gfx/mesh.h"
#include "../../ccb/gfx/renderer3d.h"

#include "../../ccb/fcn/buffer.h"

#include "feature_base.h"

// positioning
constexpr float CARDSYSTEM_CARD_WIDTH = 2.25f;
constexpr float CARDSYSTEM_CARD_HEIGHT = 3.5f;
constexpr float CARD_HWIDTH = CARDSYSTEM_CARD_WIDTH/2;
constexpr float CARD_HHEIGHT = CARDSYSTEM_CARD_HEIGHT/2;

// upload capacity
constexpr uint8_t CARDSYSTEM_UPLOAD_REPEAT = 9;
constexpr uint8_t CARDSYSTEM_INDEX_REPEAT = 12;

class PlayingCards : public ShadowGeometry
{
public:

	// construction
	PlayingCards() {  }
	PlayingCards(CascabelBaseFeature* ccbf,StageSetup* set_rigs);
	~PlayingCards() {  }

	// draw
	virtual void render_shadow();
	void render();

public:

	// render
	std::vector<float> rqueue;
	uint16_t vertex_count;

private:

	// cascabel
	Buffer bfr = Buffer();
	Shader sdr = Shader();
	CascabelBaseFeature* m_ccbf;
	StageSetup* m_setRigs;

	// render
	GLuint tex;
};
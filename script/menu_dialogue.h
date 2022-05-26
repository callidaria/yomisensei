#include <iostream>
#include <vector>

#include <glm/glm.hpp>

#include "../ccb/gfx/shader.h"
#include "../ccb/gfx/renderer2d.h"

#include "../ccb/mat/camera2d.h"

#include "../ccb/fcn/font.h"
#include "../ccb/fcn/text.h"

class MenuDialogue
{
public:
	MenuDialogue();
	MenuDialogue(glm::vec2 pos,float width,float height,Renderer2D* r2d,Camera2D* cam2d,
		const char* head,std::vector<const char*> paths,float ewidth,float eheight);
	~MenuDialogue();

	// visuals
	void render(uint8_t &index);

	// dialogue state modification
	uint8_t stall_input(std::vector<bool*> trg_stall,bool* conf,bool* back);
	void open_dialogue();
	void close();
	uint8_t hit_dialogue(bool confirm);
private:
	// essentials
	unsigned int bvao,bvbo;		// vertex array & vertex buffer for background
	Shader m_sh;				// shader
	Renderer2D* m_r2d;			// 2d renderer reference

	// dialogue state variables
	uint8_t sstate = 0;			// selection state of players choosing
	bool open = false;			// dialogue is opened

	// text
	Text thead;					// headings text

	// selection entities
	uint32_t irnd;				// render index
	uint8_t srnd;				// size read from point of index
};

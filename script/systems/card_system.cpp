#include "card_system.h"

/*
	TODO
*/
CardSystem::CardSystem()
{
	// card visualization setup
	const float hwdt = CARDSYSTEM_CARD_WIDTH/2,hhgt = CARDSYSTEM_CARD_HEIGHT/2;
	float cverts[] = {

		// card front
		-hwdt,0,-hhgt,0,0,0, -hwdt,0,hhgt,0,1,0, hwdt,0,hhgt,1,1,0,
		hwdt,0,hhgt,1,1,0, hwdt,0,-hhgt,1,0,0, -hwdt,0,-hhgt,0,0,0,

		// card back
		hwdt,0,-hhgt,0,0,1, hwdt,0,hhgt,0,1,1, -hwdt,0,hhgt,1,1,1,
		-hwdt,0,hhgt,1,1,1, -hwdt,0,-hhgt,1,0,1, hwdt,0,-hhgt,0,0,1,
	};
	bfr.bind();
	bfr.upload_vertices(cverts,sizeof(cverts));
	sdr.compile("./shader/vertex_cards.shader","./shader/fragment_cards.shader");
	sdr.def_attributeF("position",3,0,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.def_attributeF("texCoords",2,3,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.def_attributeF("texID",1,5,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.upload_camera(cam3D);

	// load card game texture
	glGenTextures(1,&tex);
	Toolbox::load_texture(tex,"./res/kopfuber_atlas.png",-1.5f);

	// card instancing: single draw call for all playing cards
	bfr.add_buffer();
	bfr.bind_index();
	sdr.def_indexF(bfr.get_indices(),"tofs",3,0,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.get_indices(),"i_tex",2,3,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.get_indices(),"rotation_sin",3,5,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.get_indices(),"rotation_cos",3,8,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.get_indices(),"deckID",1,11,CARDSYSTEM_INDEX_REPEAT);

	// spawn cards into space
	for (uint8_t i=0;i<40;i++) create_card(glm::vec2(i%10,(uint8_t)(i/10)),i>19);
	for (uint8_t i=0;i<36;i++) {
		glm::vec2 tex_id = glm::vec2(i%9,4+(uint8_t)(i/9));
		create_card(tex_id,0);
		create_card(tex_id,1);
	}

	// shuffle deck & place
	create_pile(glm::vec2(2.5f,0));
	shuffle_all();
}

/*
	TODO
*/
void CardSystem::shuffle_all()
{
	// setup allocation list
	std::vector<uint8_t> loose_list;
	for (uint8_t i=0;i<112;i++) loose_list.push_back(i);

	// process all cards
	DeckPile dpile;
	while (loose_list.size()) {

		// pick a random, uninitialized card & put it into pile
		uint8_t rcard = rand()%loose_list.size();
		move_to_pile(0,loose_list[rcard]);

		// face down card
		set_rotation(loose_list[rcard],glm::vec3(0,0,glm::radians(180.0f)));

		// un-loosen card status
		loose_list.erase(loose_list.begin()+rcard,loose_list.begin()+rcard+1);
	}
}

/*
	TODO
*/
void CardSystem::deal_card(uint8_t pid)
{
	// move card to hand
	uint8_t tmp = dpiles[pid].cards.back();
	hand.push_back(tmp);
	dpiles[pid].cards.erase(dpiles[pid].cards.end()-1,dpiles[pid].cards.end());

	// update card position & rotation
	update_hand_position();
}

/*
	TODO
*/
void CardSystem::hand_to_pile(uint8_t pid,uint8_t idx)
{
	move_to_pile(pid,hand[idx]);
	reset_rotation(hand[idx]);
	hand.erase(hand.begin()+idx,hand.begin()+idx+1);
	update_hand_position();
}

/*
	TODO
*/
void CardSystem::move_to_pile(uint8_t pid,uint8_t cid)
{
	dpiles[pid].cards.push_back(cid);
	set_position(cid,glm::vec3(dpiles[pid].pos.x,dpiles[pid].cards.size()*.017f,dpiles[pid].pos.y));
}

/*
	TODO
*/
void CardSystem::create_pile(glm::vec2 pos)
{ dpiles.push_back({ {},pos }); }

/*
	TODO
*/
void CardSystem::render()
{
	// gl enable features
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// setup
	sdr.enable();
	bfr.bind();
	bfr.bind_index();
	bfr.upload_indices(icpos);

	// draw
	glBindTexture(GL_TEXTURE_2D,tex);
	glDrawArraysInstanced(GL_TRIANGLES,0,12,112);

	// gl disable features
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
}

/*
	TODO
*/
void CardSystem::set_position(uint8_t id,glm::vec3 pos)
{
	uint16_t rid = id*CARDSYSTEM_INDEX_REPEAT;
	icpos[rid] = pos.x;icpos[rid+1] = pos.y;icpos[rid+2] = pos.z;
}

/*
	TODO
*/
void CardSystem::set_rotation(uint8_t id,glm::vec3 rot)
{
	// rasterize id jumps
	uint16_t rid = id*CARDSYSTEM_INDEX_REPEAT;

	// precalculate sine & cosine for rotation matrix in GPU
	icpos[rid+5] = glm::sin(rot.x);icpos[rid+6] = glm::sin(rot.y);icpos[rid+7] = glm::sin(rot.z);
	icpos[rid+8] = glm::cos(rot.x);icpos[rid+9] = glm::cos(rot.y);icpos[rid+10] = glm::cos(rot.z);
}

/*
	TODO
*/
void CardSystem::reset_rotation(uint8_t id)
{
	// rasterize id jumps
	uint16_t rid = id*CARDSYSTEM_INDEX_REPEAT;

	// precalculate sine & cosine for rotation matrix in GPU
	icpos[rid+5] = 0;icpos[rid+6] = 0;icpos[rid+7] = 0;
	icpos[rid+8] = 1;icpos[rid+9] = 1;icpos[rid+10] = 1;
}

/*
	TODO
*/
void CardSystem::create_card(glm::vec2 tex_id,bool deck_id)
{
	icpos.push_back(0);icpos.push_back(0);icpos.push_back(0);	// position modification
	icpos.push_back(tex_id.x);icpos.push_back(tex_id.y);		// texture atlas index
	icpos.insert(icpos.end(),{ 0,0,0,1,1,1 });					// rotation sine & cosine
	icpos.push_back(deck_id);									// deck identification
}

/*
	TODO
*/
void CardSystem::update_hand_position()
{
	for (uint8_t i=0;i<hand.size();i++) {
		set_position(hand[i],
				glm::vec3(-4-1*(hand.size()/112.0f)+((float)i/hand.size()*5),0.001f*i,7));
		set_rotation(hand[i],glm::vec3(glm::radians(-80.0f),0,0));
	}
}
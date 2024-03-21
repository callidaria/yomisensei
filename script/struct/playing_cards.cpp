#include "playing_cards.h"

/*
	constructor(CascabelBaseFeature*,StageSetup*)
	purpose: create aunt susann's kopfüber playing cards
	\param ccbf: most basic cascabel features
	\param set_rigs: stage setup
	\param sorg: light origin position, from where the shadow is projected
*/
PlayingCards::PlayingCards(CascabelBaseFeature* ccbf,StageSetup* set_rigs,glm::vec3 sorg)
	: m_ccbf(ccbf),m_setRigs(set_rigs),shadow_dir(-glm::normalize(sorg))
{
	// card visualization setup
	/*float cverts[] = {

		// card front
		-CARD_HWIDTH,0,-CARD_HHEIGHT,0,0,0,1,0,0, -CARD_HWIDTH,0,CARD_HHEIGHT,0,1,0,1,0,0,
		CARD_HWIDTH,0,CARD_HHEIGHT,1,1,0,1,0,0, CARD_HWIDTH,0,CARD_HHEIGHT,1,1,0,1,0,0,
		CARD_HWIDTH,0,-CARD_HHEIGHT,1,0,0,1,0,0, -CARD_HWIDTH,0,-CARD_HHEIGHT,0,0,0,1,0,0,

		// card back
		CARD_HWIDTH,0,-CARD_HHEIGHT,0,0,0,1,0,1, CARD_HWIDTH,0,CARD_HHEIGHT,0,1,0,1,0,1,
		-CARD_HWIDTH,0,CARD_HHEIGHT,1,1,0,1,0,1, -CARD_HWIDTH,0,CARD_HHEIGHT,1,1,0,1,0,1,
		-CARD_HWIDTH,0,-CARD_HHEIGHT,1,0,0,1,0,1, CARD_HWIDTH,0,-CARD_HHEIGHT,0,0,0,1,0,1,
	};*/

	// load card vertices & mirror for backsides
	std::vector<float> overts,cverts;
	Toolbox::load_object("./res/card.obj",overts,glm::vec3(0),1,glm::vec3(0));
	for (uint32_t i=0;i<overts.size();i+=TOOLBOX_OBJECT_LOAD_REPEAT) {
		cverts.push_back(overts[i]);cverts.push_back(overts[i+1]);cverts.push_back(overts[i+2]);
		cverts.push_back(overts[i+3]);cverts.push_back(-overts[i+4]);
		cverts.push_back(overts[i+5]);cverts.push_back(overts[i+6]);cverts.push_back(overts[i+7]);
		cverts.push_back(0);
	} for (uint32_t i=0;i<overts.size();i+=TOOLBOX_OBJECT_LOAD_REPEAT) {
		cverts.push_back(-overts[i]);cverts.push_back(overts[i+1]);cverts.push_back(overts[i+2]);
		cverts.push_back(overts[i+3]);cverts.push_back(overts[i+4]);
		cverts.push_back(overts[i+5]);cverts.push_back(overts[i+6]);cverts.push_back(overts[i+7]);
		cverts.push_back(1);
	}

	// upload card vertices
	bfr.bind();
	bfr.upload_vertices(cverts);
	sdr.compile("./shader/obj/card.vs","./shader/obj/card.fs");
	sdr.def_attributeF("position",3,0,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.def_attributeF("texCoords",2,3,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.def_attributeF("normals",3,5,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.def_attributeF("texID",1,8,CARDSYSTEM_UPLOAD_REPEAT);
	sdr.upload_camera(set_rigs->cam3D[3]);

	// load card game texture
	glGenTextures(1,&tex);
	Toolbox::load_texture(tex,"./res/kopfuber_atlas.png",-1.2f,true);
	sdr.upload_int("tex",0);

	// card instancing: single draw call for all playing cards
	bfr.add_buffer();
	bfr.bind_index();
	sdr.def_indexF(bfr.iebo,"tofs",3,0,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.iebo,"i_tex",2,3,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.iebo,"rotation_sin",3,5,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.iebo,"rotation_cos",3,8,CARDSYSTEM_INDEX_REPEAT);
	sdr.def_indexF(bfr.iebo,"deckID",1,11,CARDSYSTEM_INDEX_REPEAT);
}
// TODO: compare performance between card object and cutouts with render queue

/*
	render_shadow() -> void (virtual)
	purpose: handling of geometry shadow by 3D renderer
*/
void PlayingCards::render_shadow()
{
	// draw card shadow
	sdr.enable();
	bfr.bind();
	bfr.upload_indices(rqueue);
	sdr.upload_matrix("view",m_ccbf->r3d->shadow_view);
	sdr.upload_matrix("proj",m_ccbf->r3d->shadow_proj);
	sdr.upload_vec3("shadow_dir",shadow_dir);
	glDrawArraysInstanced(GL_TRIANGLES,0,12,112);
	sdr.upload_vec3("shadow_dir",glm::vec3(.0f));
}

/*
	render() -> void
	purpose: draw instanced playing cards
*/
void PlayingCards::render()
{
	// setup cards
	sdr.enable();
	bfr.bind();
	bfr.upload_indices(rqueue);
	sdr.upload_matrix("light_trans",m_ccbf->r3d->scam_projection);
	sdr.upload_camera(m_setRigs->cam3D[3]);

	// draw cards
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,tex);
	glDrawArraysInstanced(GL_TRIANGLES,0,12,112);
}
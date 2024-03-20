#include "jaeger_jet.h"

/*
	constructor(CascabelBaseFeature*)
	ccbf: collection of most important engine tools
	purpose: setting up the player object to be able to control and visualize pc later
*/
JaegerJet::JaegerJet(CascabelBaseFeature* ccbf)
	: m_ccbf(ccbf)
{
	// setup player hitbox indicator
	index_r2D = ccbf->r2d->add(glm::vec2(0,0),10,10,"./res/hitbox_def.png");

	// setup player character visualization
	index_r3D = ccbf->r3d->add("./res/flyfighter.obj","./res/flyfighter_tex.png","./res/none.png",
			"./res/dnormal.png","./res/none.png",glm::vec3(0,0,0),18,glm::vec3(-90,0,0));

	// add pc projectiles to bullet system
	ccbf->bSys->add_cluster(15,15,4096,"./res/hntblt.png",1,1,1,30);
	treg[2] = 9;

	// adressing indexable interaction methods
	rng_flib.push_back(&jet_wait);
	rng_flib.push_back(&jet_wide);
	rng_flib.push_back(&jet_focus);
	rng_flib.push_back(&jet_scientific);
}

/*
	update() -> void (virtual)
	purpose: update the players state and render position, visualization & health bar
*/
void JaegerJet::update()
{
	// movement speed and direction based on shot and focus mode
	glm::vec2 mvdir = m_ccbf->iMap->move_dir;
	float mvspeed = !ddur*(4+(3*!(
		m_ccbf->iMap->input_val[InputID::FOCUS]||m_ccbf->iMap->input_val[InputID::CHANGE])));

	// change position of player based on calculated movement direction
	position += glm::vec3(m_ccbf->iMap->move_dir.x*mvspeed,m_ccbf->iMap->move_dir.y*mvspeed,0)
			* glm::vec3(m_ccbf->frame->time_delta);

	// force player in-bounds
	uint16_t x_full_cap = 1280-JET_BORDER_WIDTH;
	uint16_t y_full_cap = 720-JET_BORDER_HEIGHT;
	position += glm::vec3((position.x<JET_BORDER_WIDTH)*(JET_BORDER_WIDTH-position.x)
			- (position.x>x_full_cap)*(position.x-x_full_cap),
			(position.y<JET_BORDER_HEIGHT)*(JET_BORDER_HEIGHT-position.y)
			- (position.y>y_full_cap)*(position.y-y_full_cap),position.z);

	// update bullet position by current direction & write position to register
	m_ccbf->bSys->delta_fDir(0);
	treg[0] = position.x,treg[1] = position.y;

	// run requested shot type or idle
	uint8_t sidx = ((m_ccbf->iMap->input_val[InputID::WIDE]&&!m_ccbf->iMap->input_val[InputID::FOCUS])+2
			* m_ccbf->iMap->input_val[InputID::FOCUS])*(m_ccbf->frame->time_delta>.1f);
	rng_flib[sidx](m_ccbf->bSys,treg);

	// TODO: bombs
	// TODO: shot modes and spawn
	// TODO: close quarters

	// calculate player jet tilt
	bool abs_right = m_ccbf->iMap->input_val[InputID::RIGHT];
	bool abs_left = m_ccbf->iMap->input_val[InputID::LEFT];
	tilt += (abs_right*5*(tilt<30)-abs_left*5*(tilt>-30))*m_ccbf->frame->time_delta;
	tilt += (((tilt<0)-(tilt>0))*5*(!abs_left&&!abs_right))*m_ccbf->frame->time_delta;
	glm::mat4 mdrot = glm::rotate(glm::mat4(1.0f),glm::radians(tilt),glm::vec3(0,1,0));

	// render and move player character
	m_ccbf->r3d->prepare();
	m_ccbf->r3d->s3d.upload_matrix("model",glm::translate(glm::mat4(1.0f),position)*mdrot);
	m_ccbf->r3d->render_mesh(index_r3D,index_r3D+1);

	// render player hitbox indicator
	m_ccbf->r2d->prepare();
	m_ccbf->r2d->sl[index_r2D].model = glm::translate(glm::mat4(1.0f),position-glm::vec3(5,5,0));
	m_ccbf->r2d->render_sprite(index_r2D,index_r2D+1);

	// render health bar
	/*hbar.register_damage(pDmg);
	hbar.render();*/
}

/*
	jet_wait(BulletSystem*,int32_t*) -> void (private,static)
	conforming to: rng_flib
	purpose: handle the jet flight movement idle state
*/
void JaegerJet::jet_wait(BulletSystem* bsys,int32_t* treg)
{
	// TODO: cool waiting animation and muzzle smoke particles
}

/*
	jet_wide(BulletSystem*,int32_t*) -> void (private,static)
	conforming to: rng_flib
	purpose: handle wideshot for fighter jet
*/
void JaegerJet::jet_wide(BulletSystem* bsys,int32_t* treg)
{
	for (int i=-10+50*(treg[2]<5);i<11;i++) {
		float rot = i*.075f;
		glm::vec4 rVec = glm::vec4(0,1,0,0)*glm::rotate(glm::mat4(1.0f),rot,glm::vec3(0,0,1));
		bsys->spwn_blt(0,glm::vec2(treg[0]-7,treg[1]+10),glm::vec2(rVec.x,rVec.y)*glm::vec2(12),rot);
	} treg[2]--;
	treg[2] += (treg[2]<1)*9;
}

/*
	jet_focus(BulletSystem*,int32_t*) -> void (private,static)
	conforming to: rng_flib
	purpose: handle focus shot for fighter jet
*/
void JaegerJet::jet_focus(BulletSystem* bsys,int32_t* treg)
{
	for (int i=-10;i<11;i++)
		bsys->spwn_blt(0,glm::vec2(treg[0]-7+i*7,treg[1]+10),glm::vec2(0,1)*glm::vec2(12),0);
}

/*
	jet_scientific(BulletSystem*,int32_t*) -> void (private,static)
	conforming to: rng_flib
	purpose: handle chosen secondary shot for fighter jet
*/
void JaegerJet::jet_scientific(BulletSystem* bsys,int32_t* treg)
{
	// TODO: second shot type
}
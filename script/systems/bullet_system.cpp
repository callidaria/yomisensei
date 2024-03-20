#include "bullet_system.h"

/*
	constructor(Frame*,RendererI*)
	rI: points to index renderer the visualized bullets will be handled by
	purpose: setting up the bullet system object
*/
BulletSystem::BulletSystem(Frame* frame,RendererI* rI)
	: m_frame(frame),m_rI(rI) {  }

/*
	add_cluster(uint16_t,uint16_t,const uint32_t,const char*) -> uint16_t
	width: width of bullets in cluster
	height: height of bullets in cluster
	caps: maximum capacity of bullets handled in relation with the defined cluster
	tPath: path to bullet texture (and also animation, when #50 is closed)
	purpose: adding bullet cluster to spawn bullets with later
	returns: index to reference added cluster
*/
uint16_t BulletSystem::add_cluster(uint16_t width,uint16_t height,
		const uint32_t caps,const char* tPath,uint8_t rows,uint8_t cols,uint8_t itn,uint8_t f)
{
	// setting bullet parameter lists
	m_rI->add(glm::vec2(0),width,height,tPath,rows,cols,itn,f);		// adding bullets
	std::vector<glm::vec2> t_dirs(caps);	// creating temporary bullet direction list
	std::vector<int32_t> t_ts(caps);		// creating temporary bullet tick counter list

	// moving all instanced objects outside orthogonal view
	for (int i=0;i<caps;i++) m_rI->set_aOffset(bCount.size(),i,glm::vec2(10000));
	// FIXME: set at construction

	// save bullet width and height of cluster
	c_width.push_back(width);
	c_height.push_back(height);

	// setting initial values
	bCount.push_back(0);		// add value of already spawned bullets, set to 0
	countCaps.push_back(caps);	// save capacity value to capacity value list
	dirs.push_back(t_dirs);		// add bullet direction list
	ts.push_back(t_ts);			// add bullet tick counter list
	return bCount.size()-1;		// return the index the cluster is to be referenced by
}
// TODO: find a way to index cluster attributes and independent renderer draws alike

/*
	PARAMETER DEFINITIONS:
	func(uint8_t,uint32_t,[*]) -> [*]
	cluster: bullet cluster index as reference to working cluster
	index: bullet index within cluster, referring to a specific projectile
*/

/*
	spwn_blt(uint8_t,glm::vec2,glm::vec2) -> void
	nPos: position to spawn bullet at
	nDir: initial direction bullet should head towards
	purpose: spawns bullet at given position and saves a directional parameter for later usage
*/
void BulletSystem::spwn_blt(uint8_t cluster,glm::vec2 nPos,glm::vec2 nDir)
{
	bCount[cluster] %= countCaps[cluster];  			// resetting bullet count if cap reached
	m_rI->set_aOffset(cluster,bCount[cluster],nPos);	// move bullets from original position
	m_rI->reset_anim_tick(cluster,bCount[cluster]);		// reset animation tick counter
	set_bltDir(cluster,bCount[cluster],nDir);			// setting bullet direction parameter
	reset_tick(cluster,bCount[cluster]);				// resetting bullet time ticks to 0
	bCount[cluster]++;									// increment bullet spawn counter
}

/*
	spwn_blt(uint8_t,glm::vec2,glm::vec2,float) -> void
	overloads: previous spwn_blt()
	r: starting rotation of created bullet index in upload list
	purpose: runs previous spwn_blt() and uploads a rotation value immediately afterwards
*/
void BulletSystem::spwn_blt(uint8_t cluster,glm::vec2 nPos,glm::vec2 nDir,float r)
{
	spwn_blt(cluster,nPos,nDir);
	m_rI->set_aRotation(cluster,bCount[cluster]-1,r);
}

/*
	delta_bltPos(uint8_t,uint32_t,glm::vec2) -> void
	dPos: the direction and speed the bullet should move according to
	purpose: move specific bullet according to outsidely precalculated direction and speed
*/
void BulletSystem::delta_bltPos(uint8_t cluster,uint32_t index,glm::vec2 dPos)
{ m_rI->add_aOffset(cluster,index,dPos*(float)m_frame->time_delta); }

/*
	delta_fDir(uint8_t) -> void
	purpose: update all bullets in a specified cluster according to their current movement directions
*/
void BulletSystem::delta_fDir(uint8_t cluster)
{
	// FIXME: static update loop counter
	for (int i=0;i<countCaps[cluster];i++)
		m_rI->add_aOffset(cluster,i,dirs[cluster][i]*(float)m_frame->time_delta);
}

/*
	inc_tick(uint8_t) -> void
	purpose: tick all bullets in a cluster
*/
void BulletSystem::inc_tick(uint8_t cluster)
{
	for(int i=0;i<bCount[cluster];i++)
		ts[cluster][i]++;
}

/*
	get_pHit(uint8_t,glm::vec2,float,float) -> uint8_t
	pos: center position of the object, the bullets from the cluster are in hostile relation to
	hr: object or character radius to define range of vulnerability for the hostile object
	br: bullet radius to define effective collision range (should be slightly smaller than visuals)
	returns: amount of bullets hitting the hostile object at the current moment
		the collision check is done by calculating the distance between both centers of the colliders,
		if the distance is smaller than the combined reach of both collider radii, collision happened
			O------------------------O		=> d'<(r1+r2) | for r[i] is radius of sphere
			v1		d'=|v2-v1|		v2						with center v[i]
*/
uint8_t BulletSystem::get_pHit(uint8_t cluster,glm::vec2 pos,float hr,float br)
{
	uint8_t out = 0;
	for (int i=0;i<countCaps[cluster];i++) {

		// get centered bullet position
		glm::vec2 cPos = m_rI->get_aOffset(cluster,i)
				+ glm::vec2(c_width[cluster]/2.0f,c_height[cluster]/2.0f);

		// calculate if object got hit & "despawn"
		bool hit = glm::length(cPos-pos)<=hr+br;
		m_rI->add_aOffset(cluster,i,glm::vec2(10000)*glm::vec2((int)hit));
		out += hit;
	}

	return out;
}
// TODO: squared hitboxes
// TODO: maybe compare hitboxes by chunk, disable, when player is in different chunk? performance?
// TODO: disable check for bullets outside of the frame

/*
	render() -> void
	purpose: render all spawned bullets (active or not) from all clusters belonging to this system
*/
void BulletSystem::render()
{
	m_rI->prepare(m_frame->time_delta);
	for (int i=0;i<bCount.size();i++) m_rI->render_anim(i,countCaps[i]);
}
// TODO: ??maybe add some sort of bullet cleaning system for oos bullets and reschedule (if not too perf. hoggy)
// TODO: add a reset method
// TODO: add projectile rendering rotation
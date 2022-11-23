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
	purpose: adding bullet cluster to spawn bullets with later.
		return index to reference added cluster.
*/
uint16_t BulletSystem::add_cluster(uint16_t width,uint16_t height,const uint32_t caps,const char* tPath)
{
	// setting bullet parameter lists
	m_rI->add(glm::vec2(0,0),width,height,tPath);	// adding bullets in instance renderer
	std::vector<glm::vec2> t_dirs(caps);			// creating temporary bullet direction list
	std::vector<int32_t> t_ts(caps);				// creating temporary bullet tick counter list

	// moving all instanced objects outside orthogonal view
	for (int i=0;i<caps;i++) m_rI->set_offset(bCount.size(),i,glm::vec2(-10000));

	// save bullet width and height of cluster
	c_width.push_back(width);
	c_height.push_back(height);

	// setting initial values
	bCount.push_back(0);		// add value of already spawned bullets set to 0
	countCaps.push_back(caps);	// save capacity value to capacity value list
	dirs.push_back(t_dirs);		// add bullet direction list
	ts.push_back(t_ts);			// add bullet tick counter list
	return bCount.size()-1;		// return the index, the cluster is to be referenced by
}
// FIXME: add hitbox parameters when adding cluster

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
	bCount.at(cluster) %= countCaps.at(cluster);  // resetting bullet count if cap has been reached
	// ??how performant is modulo spamming in this context in contrast to branching
	m_rI->set_offset(cluster,bCount.at(cluster),nPos);	// move bullets from original position
	set_bltDir(cluster,bCount.at(cluster),nDir);		// setting bullet direction parameter
	reset_tick(cluster,bCount.at(cluster));				// resetting bullet time ticks to 0
	bCount.at(cluster)++;  // increment bullet spawn counter
}

/*
	set_bltPos(uint8_t,uint32_t,glm::vec2) -> void
	nPos: new position of bullet to be set by this method
	purpose: change position of bullet to position pointed towards by given vector
*/
void BulletSystem::set_bltPos(uint8_t cluster,uint32_t index,glm::vec2 nPos)
{
	m_rI->set_offset(cluster,index,nPos);
}

/*
	set_bltDir(uint8_t,uint32_t,glm::vec2) -> void
	nDir: new direction the projectile should move towards or in relation to
	purpose: change direction of bullet to vector direction
*/
void BulletSystem::set_bltDir(uint8_t cluster,uint32_t index,glm::vec2 nDir)
{
	dirs.at(cluster)[index] = nDir;
}

/*
	delta_bltPos(uint8_t,uint32_t,glm::vec2) -> void
	dPos: the direction and speed the bullet should move according to
	purpose: move specific bullet according to outsidely precalculated direction and speed
*/
void BulletSystem::delta_bltPos(uint8_t cluster,uint32_t index,glm::vec2 dPos)
{
	m_rI->il.at(cluster).o[index] += dPos*m_frame->get_time_delta();
}

/*
	delta_fDir(uint8_t) -> void
	purpose: update all bullets in a specified cluster according to their current movement directions
*/
void BulletSystem::delta_fDir(uint8_t cluster)
{
	// FIXME: static update loop counter
	for (int i=0;i<countCaps.at(cluster);i++)
		m_rI->il.at(cluster).o[i] += dirs.at(cluster)[i]*m_frame->get_time_delta();
}

/*
	inc_tick(uint8_t) -> void
	purpose: tick all bullets in a cluster
*/
void BulletSystem::inc_tick(uint8_t cluster)
{
	for(int i=0;i<bCount.at(cluster);i++)
		ts.at(cluster).at(i)++;
}

/*
	reset_tick(uint8_t,uint32_t) -> void
	purpose: resetting tick counter of a specific bullet in a specific cluster
*/
void BulletSystem::reset_tick(uint8_t cluster,uint32_t index)
{
	ts.at(cluster).at(index) = 0;
}
// FIXME: deviation & naming

/*
	get_bltPos(uint8_t,uint32_t) -> vec2
	returns: position of a specific bullet in a specific cluster
*/
glm::vec2 BulletSystem::get_bltPos(uint8_t cluster,uint32_t index)
{
	return m_rI->il.at(cluster).o[index];
}

/*
	get_bltDir(uint8_t,uint32_t) -> vec2
	returns: movement direction and speed of a specific bullet in a specific cluster
*/
glm::vec2 BulletSystem::get_bltDir(uint8_t cluster,uint32_t index)
{
	return dirs.at(cluster)[index];
}

/*
	get_bCount(uint8_t) -> uint16_t
	returns: the amount of active bullets in a cluster
*/
uint16_t BulletSystem::get_bCount(uint8_t cluster)
{
	return bCount.at(cluster);
}

/*
	get_ts(uint8_t,uint32_t) -> int32_t
	returns: get tick counter of a specific bullet in a specific cluster
*/
int32_t BulletSystem::get_ts(uint8_t cluster,uint32_t index)
{
	return ts.at(cluster).at(index);
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
		glm::vec2 cPos = m_rI->il[cluster].o[i]
				+glm::vec2(c_width[cluster]/2.0f,c_height[cluster]/2.0f);

		// calculate if object got hit
		bool hit = glm::length(cPos-pos)<=hr+br;  // check collision
		m_rI->il[cluster].o[i] += glm::vec2(-10000,-10000)*glm::vec2((int)hit);  // "despawn"
		out += hit;  // increment hit-meter
	}

	return out;
}

/*
	render() -> void
	purpose: render all spawned bullets (active or not) from all clusters belonging to this system
*/
void BulletSystem::render()
{
	m_rI->prepare();
	for (int i=0;i<bCount.size();i++) m_rI->render(i,countCaps.at(i));
	// FIXME: find a solution to the bullets rendering at origin because of capping before setting
}
// TODO: ??maybe add some sort of bullet cleaning system for oos bullets and reschedule (if not too perf. hoggy)
// TODO: add a reset method
// TODO: add projectile rendering rotation
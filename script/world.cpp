#include "world.h"

/*
	constructor(EngineReference)
	eref: references to all relevant engine objects
	purpose: create a world handling entity for a better loop structure
*/
World::World(CascabelBaseFeature* eref)
	: m_ccbf(eref)
{
	game_fb = FrameBuffer(eref->frame->w_res,eref->frame->h_res,
			"./shader/fbv_menu.shader","./shader/fbf_menu.shader",false);
}

/*
	add_<element>(<element>*) -> void
	argument[0]: element to add to element list it belongs to
	purpose: create an element in the world that will be handled according to the world's logic
*/
void World::add_ui(UI* ui)
{ ui_master.push_back(ui); }
void World::add_scene(Scene* scene)
{ scene_master.push_back(scene); }
void World::add_playable(Player* player)
{ player_master.push_back(player); }
void World::add_boss(Boss* boss)
{ boss_master.push_back(boss); }

/*
	TODO
*/
void World::add_camera(Camera2D cam2D)
{ cam2D_master.push_back(cam2D); }
void World::add_camera(Camera3D cam3D)
{ cam3D_master.push_back(cam3D); }

/*
	TODO
*/
void World::free_memory()
{
	for (auto ui : ui_master) delete ui;
	for (auto scene : scene_master) delete scene;
	for (auto player : player_master) delete player;
	for (auto boss : boss_master) delete boss;
}

/*
	remove_<element>(uint8_t) -> void
	argument[0]: list index of element to remove from world
	purpose: remove an element from the world by it's list id
*/
void World::remove_ui(uint8_t ui_id)
{
	delete ui_master[ui_id];
	ui_master.erase(ui_master.begin()+ui_id);
}
void World::remove_scene(uint8_t scene_id)
{
	delete scene_master[scene_id];
	scene_master.erase(scene_master.begin()+scene_id);
}
void World::remove_playable(uint8_t player_id)
{
	delete player_master[player_id];
	player_master.erase(player_master.begin()+player_id);
}
void World::remove_boss(uint8_t boss_id)
{
	delete boss_master[boss_id];
	boss_master.erase(boss_master.begin()+boss_id);
}

/*
	TODO
*/
void World::load_geometry()
{
	m_ccbf->r2d->load(&cam2D_master[active_cam2D]);
	m_ccbf->rI->load();
	m_ccbf->r3d->load(cam3D_master[active_cam3D]);
}

/*
	render(uint32_t&,bool&) -> void
	running: runstate of the program, used to store progression & if program is still running
	reboot: holds if program should reboot after it has been closed
	purpose: render & handle world entites
*/
void World::render(uint32_t &running,bool &reboot)
{
	// bind scene framebuffer
	game_fb.bind();
	m_ccbf->frame->clear(.1f,.1f,.1f);

	// handle environments, bosses & player
	for (auto scene : scene_master) scene->render();
	for (auto boss : boss_master) boss->update(glm::vec2(100));
	for (auto player : player_master) player->update();

	// render bullets
	m_ccbf->bSys->render();

	// render ui
	game_fb.close();
	ui_master[active_daui]->render(&game_fb,running,reboot);
}
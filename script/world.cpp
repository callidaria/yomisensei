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
	deferred_fb = FrameBuffer(eref->frame->w_res,eref->frame->h_res,
			"./shader/fbv_standard.shader","./shader/gbf_lighting.shader",false);
	deferred_fb.s.upload_int("gbuffer_colour",0);
	deferred_fb.s.upload_int("gbuffer_position",1);
	deferred_fb.s.upload_int("gbuffer_normals",2);
	deferred_fb.s.upload_int("shadow_tex",3);
	m_ccbf->r3d->create_shadow(glm::vec3(100,50,150),glm::vec3(0),100,100,10,4096);
	set_rigs.lighting.add_sunlight({ glm::vec3(100,150,150),glm::vec3(1),.2f });
	set_rigs.lighting.add_pointlight({ glm::vec3(0,2,0),glm::vec3(1),1,1,.1f,1 });
	set_rigs.lighting.add_pointlight({ glm::vec3(3,2,-3),glm::vec3(1),1,1,.1f,1 });
	for (uint8_t i=0;i<4;i++)
		set_rigs.lighting.add_pointlight({ glm::vec3(7,lraise[i],-7+(i*4)),glm::vec3(1),1,1,.1f,1 });
	set_rigs.lighting.add_pointlight({ glm::vec3(-4.5f,1,4),glm::vec3(1),1,1,.1f,1 });
	set_rigs.lighting.add_pointlight({ glm::vec3(-2.5f,4,-2),glm::vec3(1),1,1,.1f,1 });
	set_rigs.lighting.add_pointlight({ glm::vec3(-2.3f,5,-1.3f),glm::vec3(1),1,1,.1f,1 });
	set_rigs.lighting.add_spotlight({ glm::vec3(0,2,0),glm::vec3(1),glm::vec3(0,-1,0),.5f,.2f });
	set_rigs.lighting.upload(&deferred_fb.s);
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
	add_camera(Camera2D||Camera3D) -> void
	argument[0]: camera to upload to list of cameras
	purpose: create a camera, that can be switched to as graphical receptor
*/
void World::add_camera(Camera2D cam2D)
{ set_rigs.cam2D.push_back(cam2D); }
void World::add_camera(Camera3D cam3D)
{ set_rigs.cam3D.push_back(cam3D); }

/*
	free_memory() -> void
	purpose: clear memory of all dynamically created objects that still exist
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
	load_geometry() -> void
	purpose: upload 2D and 3D geometry of added objects based on the active cameras
*/
void World::load_geometry()
{
	m_ccbf->r2d->load(&set_rigs.cam2D[active_cam2D]);
	m_ccbf->rI->load();
	m_ccbf->r3d->load(set_rigs.cam3D[active_cam3D]);
}

/*
	render(uint32_t&,bool&) -> void
	running: runstate of the program, used to store progression & if program is still running
	reboot: holds if program should reboot after it has been closed
	purpose: render & handle world entites
*/
void World::render(uint32_t &running,bool &reboot)
{
	// shadow processing
	m_ccbf->r3d->prepare_shadow();
	m_ccbf->r3d->render_mesh_shadow();
	m_ccbf->r3d->render_instance_shadow();
	m_ccbf->r3d->render_geometry_shadow();
	m_ccbf->r3d->close_shadow(m_ccbf->frame->w_res,m_ccbf->frame->h_res);

	// start geometry pass deferred scene
	glDisable(GL_BLEND);
	gbuffer.bind();
	m_ccbf->frame->clear(.1f,.1f,.1f);

	// handle environments, bosses & player
	m_ccbf->r3d->prepare(set_rigs.cam3D[active_cam3D]);
	m_ccbf->r3d->upload_shadow();
	for (auto scene : scene_master) scene->render(set_rigs.cam3D[active_cam3D]);
	for (auto boss : boss_master) boss->update(glm::vec2(100));
	for (auto player : player_master) player->update();

	// end geometry pass deferred scene
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glEnable(GL_BLEND);

	// render bullets
	m_ccbf->bSys->render();

	// deferred light shading
	game_fb.bind();
	deferred_fb.prepare();
	glBindTexture(GL_TEXTURE_2D,gbuffer.get_colour());
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,gbuffer.get_position());
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,gbuffer.get_normals());

	deferred_fb.s.upload_vec3("view_pos",set_rigs.cam3D[active_cam3D].pos);
	deferred_fb.draw();

	// render ui
	game_fb.close();
	ui_master[active_daui]->render(&game_fb,running,reboot);
}
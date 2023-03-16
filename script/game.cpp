#include "game.h"

Game::Game(Frame* f,Renderer2D* r2d,Renderer3D* r3d,RendererI* rI,Camera2D* cam2d)
	: m_frame(f),m_r2d(r2d),m_r3d(r3d),m_rI(rI),m_cam2d(cam2d)
{ ccbf = { m_frame,m_r2d,m_r3d,m_rI,&m_bSys,&m_player }; }

void Game::run(uint32_t &rstate,CCBManager* ccbm)
{
	m_bgenv.load(rstate,ccbm);

	// TODO: reduce
	uint32_t stg_idx2d;
	int32_t fwd_treg[16] = { 0 };
	glm::vec2 ePos = glm::vec2(615,600);
	stg_ld.at(rstate)(&ccbf,stg_idx2d,fwd_treg);

	// add spike to character list
	CharacterManager cmanager = CharacterManager();
	cmanager.add_character(0,"04","./res/mood/04.png",5,glm::vec4(1,.7f,0,1));
	cmanager.add_character(2,"Spike, Master of Cards","./res/mood/spike.png",
			6,glm::vec4(.4f,.7f,.2f,1));
	cmanager.add_character(3,"Curlhead Cross","./res/mood/cross.png",1,glm::vec4(.5f,0,0,1));

	// conversation test
	Conversation cnv_test = Conversation(m_frame,m_r2d,&cmanager,"./dat/casino_coup.mm");
	std::vector<bool> cnv_condition = { true,true,true,true,true,true,true,true,true };
	cnv_test.engage("brother_spying/card_game_spike/",cnv_condition);

	// vertex & texture load
	Camera3D cam3d = Camera3D(1280.0f,720.0f);
	m_r2d->load(m_cam2d);
	m_rI->load(m_cam2d);
	m_r3d->load(&cam3d);

	// ortho 3D element lighting
	Light3D l3d_ortho = Light3D(m_r3d,0,glm::vec3(640,360,10000),glm::vec3(1,1,1),1);
	l3d_ortho.set_amnt(1);
	l3d_ortho.upload();

	// ui
	Healthbar hbar = Healthbar(glm::vec2(440,690),790,15,{ 3,4 },
			{ 10000,5000,10000,10000,5000,5000,10000 },"The Dancing Pilot");

	// lightweight action menu
	ActionMenu lgt_menu = ActionMenu(m_frame);

	// update until exit condition
	uint32_t running = rstate+1;
	while (running) {  // ??maybe kill check if flush with static func ref

		// frame
		m_frame->print_fps();
		m_frame->calc_time_delta();
		m_frame->input(running,false);
		m_frame->clear(.1f,.1f,.1f);

		// action menu update
		lgt_menu.update(&m_player,running);
		lgt_menu.bind();

		// stage
		m_bgenv.update(rstate);
		stg_upd.at(rstate)(&ccbf,stg_idx2d,ePos,fwd_treg);

		// player
		m_player.update(rstate,fwd_treg[11]);

		// bullet system
		m_bSys.render();

		// healthbar
		hbar.register_damage(fwd_treg[10]);
		hbar.render();

		// conversation scene components & input
		cnv_test.input(m_frame->kb.ka[SDL_SCANCODE_J],
				m_frame->kb.ka[SDL_SCANCODE_UP],m_frame->kb.ka[SDL_SCANCODE_DOWN]);
		cnv_test.render_to_scene();

		// action menu render
		lgt_menu.render();

		// conversation render
		cnv_test.render(lgt_menu.get_scene_texture());

		// swap
		m_frame->update();
	} rstate = 0;  // TODO: choose between menu &|| desktop skip
}
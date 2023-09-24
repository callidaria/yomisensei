#include "main_menu.h"

/*
	constructor(CCBManager*,CascabelBaseFeature*,World*,float&,float)
	purpose: setup menu environment, populate with lists and define input possibilities
	\param ccbm: cascabel manager used to load environment and level loader files
	\param ccbf: collection of basic cascabel features
	TODO: extend documentation
*/
MainMenu::MainMenu(CCBManager* ccbm,CascabelBaseFeature* ccbf,World* world,
		float &progress,float pseq)
	: m_ccbm(ccbm),m_ccbf(ccbf),m_world(world)
{
	// asset load
	index_ranim = ccbf->r2d->al.size();
	index_rsprite = ccbm->add_lv("lvload/main_menu.ccb");

	// selection splash setup
	std::vector<float> sverts;
	create_splash(sverts,glm::vec2(SPLICE_TITLE_LOWER_START,0),glm::vec2(SPLICE_TITLE_UPPER_START,720),
			glm::vec3(.5f,0,0));
	create_splash(sverts,glm::vec2(0,SPLICE_HEAD_LOWER_START),glm::vec2(1280,SPLICE_HEAD_UPPER_START),
			SPLICE_HEAD_COLOUR);
	create_splash(sverts,glm::vec2(0),glm::vec2(0,720),SPLICE_SELECTION_COLOUR);
	sh_buffer.bind();
	sh_buffer.upload_vertices(sverts);
	sh_shader.compile("./shader/main_menu/vsplash.shader","./shader/main_menu/fsplash.shader");
	sh_shader.def_attributeF("position",2,0,SPLICE_VERTEX_FLOAT_COUNT);
	sh_shader.def_attributeF("colour",3,2,SPLICE_VERTEX_FLOAT_COUNT);
	sh_shader.def_attributeF("edge_id",1,5,SPLICE_VERTEX_FLOAT_COUNT);
	sh_shader.upload_camera(Camera2D(1280.f,720.f));

	// version annotation text setup
	std::string vmessage = "yomisensei by callidaria. danmaku v"
			+ std::to_string(INFO_VERSION_RELEASE)+'.'+std::to_string(INFO_VERSION_SUBRELEASE)+'.'
			+ std::to_string(INFO_VERSION_DEVSTEP)+INFO_VERSION_MODE_SUFFIX
			+ " - running on cascabel base (OpenGL)";
	tx_version.add(vmessage.c_str(),TEXT_VERSION_POSITION);
	tx_version.load();
	tcap_version = vmessage.length();

	// menu options text
	for (uint8_t i=0;i<MENU_MAIN_OPTION_COUNT;i++) {
		uint32_t wwidth = fnt_mopts.calc_wordwidth(main_options[i]);
		mo_twidth[i] = wwidth,mo_hwidth[i] = wwidth*.5f;
		mo_prog.x -= wwidth;
	} mo_prog /= glm::vec2(MENU_MAIN_OPTION_COUNT);
	glm::vec2 mo_cursor = MENU_OPTIONS_CLEFT+glm::vec2(mo_hwidth[0],0);
	for (uint8_t i=0;i<MENU_MAIN_OPTION_COUNT;i++) {
		tx_mopts[i].add(main_options[i],glm::vec2(-mo_hwidth[i],0)),tx_mopts[i].load();
		mo_cposition[i] = mo_cursor;
		mo_cursor += mo_prog+glm::vec2(mo_twidth[i],0);
	}

	// peripheral sensitive input request annotations
	update_peripheral_annotations();

	// buffers
	msaa = MSAA("./shader/fbv_standard.shader","./shader/main_menu/fbf_splash.shader",
			m_ccbf->frame->w_res,m_ccbf->frame->h_res,8);
	msaa.sfb.upload_int("menu_fb",1);
	fb_menu = FrameBuffer(m_ccbf->frame->w_res,m_ccbf->frame->h_res,
			"./shader/fbv_standard.shader","./shader/main_menu/fbf_mainmenu.shader");
}

/*
	render(FrameBuffer*,bool&,bool&) -> void (virtual) !O(1)
	purpose: render the main menu, calculate geometry and process interactions
	\param game_fb: deferred shaded game scene framebuffer
	\param running: is application still running?
	\param reboot: will be rebooting with new settings after game closed?
*/
void MainMenu::render(FrameBuffer* game_fb,bool &running,bool &reboot)
{
	// input
	bool plmb = m_ccbf->frame->mouse.mb[0]&&!trg_lmb,prmb = m_ccbf->frame->mouse.mb[2]&&!trg_rmb;
	bool hit_a = (m_ccbf->iMap->get_input_triggered(IMP_REQPAUSE)&&!menu_action)
			|| m_ccbf->iMap->get_input_triggered(IMP_REQFOCUS)||plmb
			|| m_ccbf->iMap->get_input_triggered(IMP_REQCONFIRM),
		hit_b = (m_ccbf->iMap->get_input_triggered(IMP_REQPAUSE)&&menu_action)
			|| m_ccbf->iMap->get_input_triggered(IMP_REQBOMB)||prmb;
	int8_t lrmv = ((m_ccbf->iMap->get_input_triggered(IMP_REQRIGHT)&&vselect<MENU_MAIN_OPTION_CAP)
			- (m_ccbf->iMap->get_input_triggered(IMP_REQLEFT)&&vselect>0))*menu_action;
	trg_lmb = m_ccbf->frame->mouse.mb[0],trg_rmb = m_ccbf->frame->mouse.mb[2];

	// timing
	bool anim_go = anim_timing>ANIMATION_UPDATE_TIMEOUT;
	anim_timing += m_ccbf->frame->time_delta;
	dt_tshiftdown += m_ccbf->frame->time_delta*speedup,
			dt_tnormalize += m_ccbf->frame->time_delta*!speedup;

	// menu transition
	bool req_transition = hit_a&&!menu_action;
	menu_action = (menu_action||hit_a)&&!hit_b;
	mtransition += (menu_action-!menu_action)*TRANSITION_SPEED*m_ccbf->frame->time_delta;
	mtransition = (mtransition<.0f) ? .0f : (mtransition>1.f) ? 1.f : mtransition;
	/*uint8_t tmin = (mtransition<.0f),tmax = (mtransition>1.f);
	mtransition = mtransition-(mtransition-1.f)*tmax+abs(mtransition)*tmin;*/
	// TODO: compare linear transition with sinespeed transition implementation
	float inv_mtransition = 1.f-mtransition;

	// processing selection input
	bool ch_select = lrmv!=0;
	vselect += lrmv;
	vselect = (m_ccbf->frame->mpref_peripheral)
			? get_selected_main_option(m_ccbf->frame->mouse.mxfr,ch_select) : vselect;

	// title rattle animation
	uint8_t rattle_mobility = RATTLE_THRESHOLD+RATTLE_THRESHOLD_RAGEADDR*menu_action,
		rattle_countermove = rattle_mobility/2;
	glm::vec3 title_action = glm::vec3((rand()%rattle_mobility-rattle_countermove)*anim_go,
			(rand()%rattle_mobility-rattle_countermove)*anim_go,0);

	// title shiftdown animation
	dt_tshiftdown *= menu_action,dt_tnormalize *= menu_action,speedup = speedup||!menu_action;
	float tshift = 1.f+SHIFTDOWN_ZOOM_INCREASE*((speedup) ? sqrt(sin(dt_tshiftdown*MATH_OCTAPI))
			: 1.f-sqrt(dt_tnormalize));

	// title animation
	glm::vec3 vrt_position = VRT_TITLE_START+VRT_TITLE_TRANSITION*mtransition+title_action,
		hrz_position = HRZ_TITLE_START+HRZ_TITLE_TRANSITION*mtransition+title_action;
	glm::mat4 vrt_scale = glm::translate(glm::scale(glm::translate(glm::mat4(1),
			-VRT_TITLE_SCALESET),glm::vec3(tshift)),VRT_TITLE_SCALESET),
		hrz_scale = glm::translate(glm::scale(glm::translate(glm::mat4(1),
			-HRZ_TITLE_SCALESET),glm::vec3(tshift)),HRZ_TITLE_SCALESET);
	m_ccbf->r2d->al[index_ranim].model = glm::translate(glm::mat4(1),vrt_position)*vrt_scale;
	m_ccbf->r2d->al[index_ranim+1].model = glm::translate(glm::mat4(1),hrz_position)*hrz_scale;

	// selection splash update calculations
	if (ch_select||req_transition) {

		// selector dimensions
		vrt_lwidth = rand()%(uint16_t)mo_hwidth[vselect],
			vrt_uwidth = rand()%(uint16_t)mo_hwidth[vselect];
		glm::vec2 vrt_cpos = mo_cposition[vselect]+glm::vec2(mo_hwidth[vselect],0);
		vrt_lpos = glm::vec2((vrt_cpos.x-MENU_HALFSCREEN_UI)*SPLICE_OFFCENTER_MV+MENU_HALFSCREEN_UI,0);
		glm::vec2 vrt_dir = vrt_cpos-vrt_lpos;
		float vrt_extend_II = (720.f-vrt_cpos.y)/vrt_dir.y;
		vrt_upos = glm::vec2(vrt_cpos.x+vrt_dir.x*vrt_extend_II,0);

		// menu option text
		//st_rot = glm::radians((float)(rand()%MENU_OPTIONS_RDEG_THRES)*-((rand()%2)*2-1));
		st_rot = 0;
		// disabled due to pseudoshadow implementation testing simplification
	}

	// peripheral switch for input request annotation
	if (cpref_peripheral!=m_ccbf->frame->cpref_peripheral) update_peripheral_annotations();

	// START MULTISAMPLED RENDER
	msaa.bind();
	Frame::clear();

	// splash render
	sh_buffer.bind();
	sh_shader.enable();

	// head splash upload & render
	modify_splash(glm::vec2(0),glm::vec2(0),SPLICE_HEAD_LOWER_WIDTH*mtransition,
			SPLICE_HEAD_UPPER_WIDTH*mtransition,true);
	glDrawArrays(GL_TRIANGLES,6,6);

	// selection splash upload & render
	modify_splash(vrt_lpos,vrt_upos,vrt_lwidth*mtransition,vrt_uwidth*mtransition,false);
	glDrawArrays(GL_TRIANGLES,12,6);
	// FIXME: splash dimensions to prevent aesthetically unfortunate proportions or too thin selectors

	// title splash upload & render
	float tlpos = SPLICE_TITLE_LOWER_MOD*mtransition,tupos = SPLICE_TITLE_UPPER_MOD*mtransition;
	float tlext = SPLICE_TITLE_LOWER_SWIDTH+SPLICE_TITLE_LWIDTH_MOD*mtransition,
		tuext = SPLICE_TITLE_UPPER_SWIDTH+SPLICE_TITLE_UWIDTH_MOD*mtransition;
	modify_splash(glm::vec2(tlpos,0),glm::vec2(tupos,0),tlext,tuext,false);
	glDrawArrays(GL_TRIANGLES,0,6);

	// START RENDER MENU BUFFER
	msaa.blit();
	fb_menu.bind();
	Frame::clear();

	// render text
	tx_dare.prepare();
	tx_dare.set_scroll(glm::vec2(title_action));
	tx_dare.render(tcap_dare*inv_mtransition,TEXT_DARE_COLOUR);
	tx_version.prepare();
	tx_version.render(tcap_version*inv_mtransition,TEXT_VERSION_COLOUR);

	// render main options
	for (uint8_t i=0;i<MENU_MAIN_OPTION_COUNT;i++) {
		tx_mopts[i].prepare();
		glm::mat4 opt_trans = glm::translate(glm::mat4(1.f),
				glm::vec3(mo_cposition[i].x+mo_hwidth[i],mo_cposition[i].y,0));
		glm::vec4 opt_colour = glm::vec4(.5f,1.f,.5f,mtransition);
		if (i==vselect) {
			opt_trans = glm::scale(opt_trans,glm::vec3(MENU_OPTIONS_SCALE_THRES));
			opt_trans = glm::rotate(opt_trans,st_rot,glm::vec3(0,0,1));
			opt_colour.z = 1.f;
		} tx_mopts[i].set_scroll(opt_trans);
		tx_mopts[i].render(strlen(main_options[i]),opt_colour);
	}

	// render titles
	m_ccbf->r2d->prepare();
	m_ccbf->r2d->render_state(index_ranim,glm::vec2(3,0));
	m_ccbf->r2d->render_state(index_ranim+1,glm::vec2(0,0));

	// STOP RENDER MENU BUFFER
	FrameBuffer::close();

	// render menu
	fb_menu.render(mtransition);
	msaa.prepare();
	msaa.sfb.upload_float("mtrans",mtransition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,fb_menu.tex);
	MSAA::render();
	glActiveTexture(GL_TEXTURE0);
	// FIXME: remove special treatment and transfer to a more controllable implementation

	// finishing
	bool shiftdown_over = dt_tshiftdown>TITLE_SHIFTDOWN_TIMEOUT,
			normalize_over = dt_tnormalize>TITLE_NORMALIZATION_TIMEOUT;
	anim_timing -= ANIMATION_UPDATE_TIMEOUT*anim_go;
	dt_tshiftdown -= TITLE_SHIFTDOWN_TIMEOUT*shiftdown_over,
			dt_tnormalize -= TITLE_NORMALIZATION_TIMEOUT*normalize_over;
	speedup = (speedup&&!shiftdown_over)||normalize_over;
}

/*
	TODO
*/
uint8_t MainMenu::get_selected_main_option(float mx,bool &ch_select)
{
	float tsmx = mx*1280.f;
	uint8_t out_id = vselect;
	while (tsmx<(mo_cposition[out_id].x-mo_prog.x)&&out_id>0) out_id--;
	while (tsmx>(mo_cposition[out_id].x+mo_twidth[out_id]+mo_prog.x)&&out_id<MENU_MAIN_OPTION_CAP)
		out_id++;
	ch_select = ch_select||(out_id!=vselect);
	return out_id;
}

/*
	TODO
*/
void MainMenu::create_splash(std::vector<float> &sverts,glm::vec2 l,glm::vec2 u,glm::vec3 c)
{
	std::vector<float> verts = {
		l.x,l.y,c.x,c.y,c.z,0, u.x,u.y,c.x,c.y,c.z,3, u.x,u.y,c.x,c.y,c.z,2,
		u.x,u.y,c.x,c.y,c.z,3, l.x,l.y,c.x,c.y,c.z,0, l.x,l.y,c.x,c.y,c.z,1
	}; sverts.insert(sverts.end(),verts.begin(),verts.end());
}

/*
	TODO
	NOTE: selection shader has to be enabled before calling this function
*/
void MainMenu::modify_splash(glm::vec2 lp,glm::vec2 up,float le,float ue,bool hrz)
{
	sh_shader.upload_vec2("lupos[0]",lp),sh_shader.upload_vec2("lupos[1]",up);
	sh_shader.upload_float("luext[0]",le),sh_shader.upload_float("luext[1]",ue);
	sh_shader.upload_int("is_hrz",hrz);
}

/*
	TODO
*/
void MainMenu::update_peripheral_annotations()
{
	// update shown preferred peripheral
	cpref_peripheral = m_ccbf->frame->cpref_peripheral;

	// title screen dare message
	tx_dare.clear();
	std::string reqbutton = (cpref_peripheral) ? m_ccbf->iMap->cnt_name[IMP_REQPAUSE]
			: m_ccbf->iMap->key_name[IMP_REQCONFIRM];
	std::string dmessage = "press ["+reqbutton+"] if you DARE";
	tx_dare.add(dmessage.c_str(),TEXT_DARE_POSITION);
	tcap_dare = dmessage.length();
	tx_dare.load();
}

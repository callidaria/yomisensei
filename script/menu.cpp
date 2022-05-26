#include "menu.h"

Menu::Menu(CCBManager* ccbm,Frame* f,Renderer2D* r2d,Renderer3D* r3d,RendererI* rI,Camera2D* cam2d)
	: m_ccbm(ccbm),m_frame(f),m_r2d(r2d),m_rI(rI),m_cam2d(cam2d)
{
	const char* GVERSION = "0.0.2";

	msindex = ccbm->add_lv("lvload/menu.ccb");

	Font fnt = Font("res/fonts/nimbus_roman.fnt","res/fonts/nimbus_roman.png",25,25);
	Font vfnt = Font("res/fonts/nimbus_roman.fnt","res/fonts/nimbus_roman.png",15,15);
	tft = Text(fnt);vtft = Text(vfnt);
	tft.add("press START if you DARE",glm::vec2(450,250));
	vtft.add("yomisensei by callidaria. danmaku version 0.0.3D - running on cascabel 1.3.1vE (OpenGL)",
			glm::vec2(630,20)); // FIXME: hardcoded version number
	tft.load_wcam(cam2d);vtft.load_wcam(cam2d);

	mls[0]=MenuList();
	mls[1]=MenuList(r2d,cam2d,"lvload/ml_mopt");
	mls[2]=MenuList(r2d,cam2d,"lvload/ml_stages");
	mls[3]=MenuList(r2d,cam2d,"lvload/ml_arcades");
	mls[4]=MenuList();
	mls[5]=MenuList();
	mls[6]=MenuList();
	mls[7]=MenuList(r2d,cam2d,"lvload/ml_optfrm");
	mls[8]=MenuList(r2d,cam2d,"lvload/ml_optaud");
	mls[9]=MenuList(r2d,cam2d,"lvload/ml_optgfx");
	mls[10]=MenuList(r2d,cam2d,"lvload/ml_optgm");
	mls[11]=MenuList(r2d,cam2d,"lvload/ml_optext");
	// FIXME: mess

	glGenVertexArrays(1,&svao);
	glGenBuffers(1,&svbo);
	sshd = Shader();

	glm::vec3 s_rgb = glm::vec3(.245f,.606f,.564f);
	glm::vec3 h_rgb = glm::vec3(.341f,.341f,.129f);
	glm::vec3 l_rgb = glm::vec3(1,0,0);
	glm::vec3 r_rgb = glm::vec3(0,0,1);
	float sverts[] = { // TODO: remove hardcoded secondary segment coords
		// title splash
		-25,0,25,0,.5f,0,0,0, 420,720,-25,720,.5f,0,0,1, 600,720,25,720,.5f,0,0,2,
		600,720,25,720,.5f,0,0,2, 50,0,160,0,.5f,0,0,3, -25,0,25,0,.5f,0,0,0,

		// head splash
		0,500,0,500,h_rgb.x,h_rgb.y,h_rgb.z,0, 0,500,0,550,h_rgb.x,h_rgb.y,h_rgb.z,1,
		0,500,1280,600,h_rgb.x,h_rgb.y,h_rgb.z,2, 0,500,1280,600,h_rgb.x,h_rgb.y,h_rgb.z,2,
		0,500,1280,470,h_rgb.x,h_rgb.y,h_rgb.z,3, 0,500,0,500,h_rgb.x,h_rgb.y,h_rgb.z,0,

		1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,4, 1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,5,
		1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,6, 1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,6,
		1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,7, 1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,4,

		// select splash
		630,0,630,0,s_rgb.x,s_rgb.y,s_rgb.z,0, 630,0,0,720,s_rgb.x,s_rgb.y,s_rgb.z,1,
		650,0,0,720,s_rgb.x,s_rgb.y,s_rgb.z,2, 650,0,0,720,s_rgb.x,s_rgb.y,s_rgb.z,2,
		650,0,650,0,s_rgb.x,s_rgb.y,s_rgb.z,3, 630,0,630,0,s_rgb.x,s_rgb.y,s_rgb.z,0,

		// crossselect splash
		1280,500,1280,500,l_rgb.x,l_rgb.y,l_rgb.z,0, 1280,500,1280,550,l_rgb.x,l_rgb.y,l_rgb.z,1,
		1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,2, 1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,2,
		1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,3, 1280,500,1280,500,l_rgb.x,l_rgb.y,l_rgb.z,0,

		1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,3, 1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,2,
		1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,4, 1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,4,
		1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,5, 1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,3,

		1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,5, 1280,500,1280,550,h_rgb.x,h_rgb.y,h_rgb.z,4,
		1280,500,1280,550,r_rgb.x,r_rgb.y,r_rgb.z,6, 1280,500,1280,550,r_rgb.x,r_rgb.y,r_rgb.z,6,
		1280,500,1280,500,r_rgb.x,r_rgb.y,r_rgb.z,7, 1280,500,1280,500,h_rgb.x,h_rgb.y,h_rgb.z,5
	}; // ??clockwise rotation triangle hardcoded replace
	// FIXME: mess

	glBindVertexArray(svao); // §§??
	glBindBuffer(GL_ARRAY_BUFFER,svbo);
	glBufferData(GL_ARRAY_BUFFER,sizeof(sverts),sverts,GL_STATIC_DRAW);
	sshd.compile_vCols("shader/fbv_select.shader","shader/fbf_select.shader");
	sshd.upload_matrix("view",cam2d->view2D);
	sshd.upload_matrix("proj",cam2d->proj2D);

	fb=FrameBuffer(f->w_res,f->h_res,"shader/fbv_menu.shader","shader/fbf_menu.shader",false);
	splash_fb=FrameBuffer(f->w_res,f->h_res,"shader/fbv_standard.shader","shader/fbf_standard.shader",false);
	title_fb=FrameBuffer(f->w_res,f->h_res,"shader/fbv_standard.shader","shader/fbf_standard.shader",false);
	select_fb=FrameBuffer(f->w_res,f->h_res,"shader/fbv_standard.shader","shader/fbf_standard.shader",false);
	cross_fb=FrameBuffer(f->w_res,f->h_res,"shader/fbv_standard.shader","shader/fbf_standard.shader",false);

	for (int i=0;i<4;i++) m_r2d->sl.at(msindex+14+i).scale_arbit(1,0);

	// setting up menu dialogues
	std::vector<const char*> pth_diff = {
		"res/diffs/normal_diff.png","res/diffs/master_diff.png",
		"res/diffs/gmaster_diff.png","res/diffs/rhack_diff.png"
	};
	md_diff = MenuDialogue(glm::vec2(200,100),880,520,m_r2d,m_cam2d,"select difficulty",pth_diff,150,450);
	std::vector<const char*> pth_conf = { "res/ok.png","res/no.png" };
	md_conf = MenuDialogue(glm::vec2(200,250),250,150,m_r2d,m_cam2d,"confirm changes?",pth_conf,75,75);

	if (f->m_gc.size()>0) { // TODO: include all axis and common intuitive input systems
		cnt_b = &f->xb.at(0).xbb[SDL_CONTROLLER_BUTTON_B];
		cnt_start = &f->xb.at(0).xbb[SDL_CONTROLLER_BUTTON_A];
		cnt_lft = &f->xb.at(0).xbb[SDL_CONTROLLER_BUTTON_DPAD_LEFT];
		cnt_rgt = &f->xb.at(0).xbb[SDL_CONTROLLER_BUTTON_DPAD_RIGHT];
		cnt_dwn = &f->xb.at(0).xbb[SDL_CONTROLLER_BUTTON_DPAD_DOWN];
		cnt_up = &f->xb.at(0).xbb[SDL_CONTROLLER_BUTTON_DPAD_UP];
	} else {
		cnt_b = &f->kb.ka[SDL_SCANCODE_Q];
		cnt_start = &f->kb.ka[SDL_SCANCODE_RETURN];
		cnt_lft = &f->kb.ka[SDL_SCANCODE_LEFT];
		cnt_rgt = &f->kb.ka[SDL_SCANCODE_RIGHT];
		cnt_dwn = &f->kb.ka[SDL_SCANCODE_DOWN];
		cnt_up = &f->kb.ka[SDL_SCANCODE_UP];
	}
} Menu::~Menu() {  }

/*
	render(uint32_t&)
	running: is application still running, and if so which lvindex?
	reboot: will be rebooting with new settings after game closed?
	purpose: render the main menu, calculate geometry and process interactions
*/
void Menu::render(uint32_t &running,bool &reboot)
{
	bool is_shift;
	uint8_t tmm;

	bool hit_a=*cnt_start&&!trg_start,hit_b=*cnt_b&&!trg_b;
	bool hit_lft=*cnt_lft&&!trg_lft,hit_rgt=*cnt_rgt&&!trg_rgt;

	int ml_delta = 0;
	uint8_t thrzt,tvrtt;
	int8_t mv_dlta = 0;

	uint8_t i_ml = mselect-2+opt_index;
	if (edge_sel&&hit_a) mls[i_ml].write_tempID(lselect);
	uint8_t dlgmod = hit_rgt-hit_lft;
	// FIXME: remove branch from loop

	std::vector<bool*> stall_trg = { &trg_start,&trg_b,&trg_up,&trg_dwn,&trg_lft,&trg_rgt };
	uint8_t diff_can = md_diff.stall_input(stall_trg,cnt_start,cnt_b);
	if (diff_can==1) mm = MenuMode::MENU_LISTING;
	else if (diff_can>1) difflv = diff_can-1;

	uint8_t conf_can = md_conf.stall_input(stall_trg,cnt_start,cnt_b);
	if (conf_can==1||(conf_can==2&&dsi_conf==2)) {
		for (int i=7;i<11;i++) mls[i].reset();
		mm = MenuMode::MENU_SELECTION;
	} else if (conf_can==2) {
		std::ofstream kill("config.ini");
		kill.close();
		for (int i=7;i<11;i++) mls[i].save();
		reboot = true;
		running = 0;
		return;
	}

	bool changed;
	switch (mm) {
	case MenuMode::MENU_TITLE:
		mm = (MenuMode)(MenuMode::MENU_SELECTION*(*cnt_start&&!trg_start));
		break;
	case MenuMode::MENU_SELECTION:
		tmm = 1;
		tmm += 3*(*cnt_start&&!trg_start);				// j listing
		tmm -= 2*(*cnt_start&&!trg_start&&mselect==7); 			// j start
		tmm = tmm*!(*cnt_b&&!trg_b);					// j title
		running = !(*cnt_start&&!trg_start&&mselect==2);		// exit
		mm = (MenuMode)tmm;
		lselect=0;lscroll=0; // FIXME: reduce one set call. idc which one
		is_shift = (tmm<5&&tmm>2)||(dtrans>.01f); // ??breakdown logic | determine when to shift title
		mve=(glm::vec2(360-(m_r2d->sl.at(msindex+mselect*2-4).sclx/2),650)
			-m_r2d->sl.at(msindex+mselect*2-4).pos)*glm::vec2(is_shift); // setting title destination
		mvj=(glm::vec2(50,50)-m_r2d->sl.at(msindex+mselect*2-3).pos)*glm::vec2(is_shift);
		mv_dlta = *cnt_rgt*(mselect<8&&!trg_rgt)-*cnt_lft*(mselect>2&&!trg_lft);
		mselect += mv_dlta; // update selection marker
		thrzt = hrz_title;
		tvrtt = vrt_title;
		hrz_title = rand()%4*(!!mv_dlta)+hrz_title*(!mv_dlta);
		vrt_title = rand()%4*(!!mv_dlta)+vrt_title*(!mv_dlta);
		hrz_title = (hrz_title+(hrz_title==thrzt&&!!mv_dlta))%4;
		vrt_title = (vrt_title+(vrt_title==tvrtt&&!!mv_dlta))%4;
		break;
	case MenuMode::MENU_DIFFS:  // TODO: remove this state and control through keystalling
		tmm = 3;
		tmm -= difflv>0;//*cnt_start&&!trg_start;
		tmm += *cnt_b&&!trg_b;
		mm = (MenuMode)tmm;
		md_diff.open_dialogue();
		break;
	case MenuMode::MENU_LISTING:
		tmm = 4;
		tmm += *cnt_start&&!trg_start&&mselect==3;
		tmm -= *cnt_start&&!trg_start&&mselect!=3;
		changed = false;
		for (int i=7;i<11;i++) changed = mls[i].was_changed()||changed;	 // FIXME: maybe a little heavy
		if (*cnt_b&&!trg_b&&mselect==3&&changed) md_conf.open_dialogue();
		else tmm -= 3*(*cnt_b&&!trg_b);
		mm = (MenuMode)tmm;
		opt_index = (6+lselect)*(tmm==5);
		lselect += (*cnt_dwn&&!trg_dwn&&lselect<(mls[mselect-2].esize-1))-(*cnt_up&&!trg_up&&lselect>0);
		lselect *= tmm!=5;
		break;
	case MenuMode::MENU_SUBLIST:
		tmm = 5;
		tmm -= *cnt_b&&!trg_b&&!edge_sel;
		mm = (MenuMode)tmm;
		lselect += (*cnt_dwn&&!trg_dwn&&lselect<(mls[mselect-2+opt_index].esize-1)&&!edge_sel)
			-(*cnt_up&&!trg_up&&lselect>0&&!edge_sel); // FIXME: reduce to one increment calculation
		opt_index *= tmm==5; // FIXME: doubled logical can be broken down in MENU_LISTING
		lselect *= tmm!=4;
		edge_sel = !edge_sel*(hit_a||(edge_sel&&hit_b))+edge_sel*(!hit_a&&!(edge_sel&&hit_b));
		ml_delta = edge_sel*((*cnt_dwn&&!trg_dwn)-(*cnt_up&&!trg_up));
		ml_delta = edge_sel*ml_delta+!edge_sel*(*cnt_rgt-*cnt_lft);
		break;
	default:running=lselect;game.run(running,m_ccbm); // running game at position
	} trg_start=*cnt_start;trg_b=*cnt_b;trg_lft=*cnt_lft;trg_rgt=*cnt_rgt;trg_dwn=*cnt_dwn;trg_up=*cnt_up;
	// FIXME: break branch with static function pointer list
	// FIXME: a button naming as cnt_start reference is off and should be changed

	// move non-used out of view
	for (int i=1;i<8;i++) { // FIXME: i will regret this tomorrow ...just a test
		float tval = m_r2d->sl.at(msindex+i*2-1).pos.x+250;
		m_r2d->sl.at(msindex+i*2-1).model=glm::translate(glm::mat4(1.0f),dtrans*glm::vec3(-tval,0,0));
		tval = m_r2d->sl.at(msindex+i*2-2).pos.x+250;
		m_r2d->sl.at(msindex+i*2-2).model=glm::translate(glm::mat4(1.0f),dtrans*glm::vec3(-tval,0,0));
	}

	// animate movement within title positions
	ptrans+=.1f*(mm==1&&ptrans<1);ptrans-=.1f*(mm==0&&ptrans>.01f); // !!use an epsilon, pretty please
	dtrans+=.1f*(mm>2&&dtrans<1);dtrans-=.1f*(mm<2&&dtrans>.01f); // FIXME: reduce this
	// strans+=.1f*(mm==3&&strans<1);strans-=.1f*(mm!=3&&strans>.01f);
	m_r2d->sl.at(msindex+mselect*2-4).model=glm::translate(glm::mat4(1.0f),dtrans*glm::vec3(mve.x,mve.y,0));
	m_r2d->sl.at(msindex+mselect*2-3).model=glm::translate(glm::mat4(1.0f),dtrans*glm::vec3(mvj.x,mvj.y,0));
	pos_title = glm::translate(glm::mat4(1.0f),TITLE_START+title_dir*ptrans+dtrans*glm::vec3(-140,0,0));
	pos_entitle = glm::translate(glm::mat4(1.0f),ENTITLE_START+entitle_dir*(ptrans-dtrans));
	for (int i=0;i<4;i++) m_r2d->sl.at(msindex+14+i).scale_arbit(1,strans);

	// menu list selection and scrolling
	lscroll += (lselect>lbounds)-(lselect<(lbounds-7)); // FIXME: kill the extra scrolling calculation
	bool scddiff = lselect>lbounds;
	bool scudiff = lselect<(lbounds-7);
	lbounds = (!scddiff&&!scudiff)*lbounds+scddiff*lselect+scudiff*(lselect+7);
	int lcscroll = 45*(lselect+7-lbounds);
	for (int i=0;i<8;i++) sbar[i] = (diffsel!=lselect)*(rand()%30-15)+(diffsel==lselect)*sbar[i]; // !!range

	// modulation coordinates title splash
	/*
		INITIAL		DESTINATION	VERTEX COLOUR		INDEX
		-25,0,		25,0,		.5f,0,0,		0,
		420,720,	-25,720,	.5f,0,0,		1,
		600,720,	25,720,		.5f,0,0,		2,
		50,0,		160,0,		.5f,0,0,		3,
	*/

	// container square parameters
	/*
	md_diff = MenuDialogue(glm::vec2(200,100),880,520,m_r2d,r3d,m_cam2d,"select difficulty",pth_diff,150,450);
	md_conf = MenuDialogue(glm::vec2(200,250),250,150,m_r2d,r3d,m_cam2d,"confirm changes?",pth_conf,75,75);
	*/

	// render the title splash
	sshd.enable();
	glBindVertexArray(svao);

	//glm::vec2 dopen = glm::vec2(dsi_diff||dsi_conf);
	//glm::mat4 dlgrot = glm::rotate(glm::mat4(1.0f),glm::radians(dlgrot_val),glm::vec3(0,0,1));
	//dlgrot_val += 5-(dlgrot_val>360)*360;
	//glm::vec2 disp = glm::vec2(235,155)*glm::vec2(dsi_diff>0)+glm::vec2(257,285)*glm::vec2(dsi_conf>0);
	//disp.x += (dsi_diff-(dsi_diff>0))*220+(dsi_conf-(dsi_conf>0))*125;

	sshd.upload_float("ptrans",ptrans);
	/*sshd.upload_vec2("idx_mod[0]",(glm::vec2(dlgrot*(glm::vec4(glm::vec2(-50,-25)+disp))))*dopen);
	sshd.upload_vec2("idx_mod[1]",(glm::vec2(dlgrot*(glm::vec4(glm::vec2(0,-695)+disp))))*dopen);
	sshd.upload_vec2("idx_mod[2]",(glm::vec2(dlgrot*(glm::vec4(glm::vec2(0,-695)+disp))))*dopen);
	sshd.upload_vec2("idx_mod[3]",(glm::vec2(dlgrot*(glm::vec4(glm::vec2(-135,-25)+disp))))*dopen);*/
	sshd.upload_vec2("idx_mod[0]",glm::vec2(0));
	sshd.upload_vec2("idx_mod[1]",glm::vec2(0));
	sshd.upload_vec2("idx_mod[2]",glm::vec2(0));
	sshd.upload_vec2("idx_mod[3]",glm::vec2(0));

	splash_fb.bind();
	m_frame->clear(0,0,0);
	glDrawArrays(GL_TRIANGLES,0,6);
	splash_fb.close();

	// render the horizontal selection splash
	int rnd_edge[4];
	for (int i=0;i<4;i++) rnd_edge[i] = (rand()%10-5);
	int32_t xscr0 = -700,xscr1 = -400;

	sshd.upload_vec2("idx_mod[0]",glm::vec2(0,-15*(mm>2)-lcscroll+sbar[0]+(rand()%10-5)));
	sshd.upload_vec2("idx_mod[1]",glm::vec2(0,-40*(mm>2)-lcscroll+sbar[1]+(rand()%10-5)));
	sshd.upload_vec2("idx_mod[2]",glm::vec2(xscr0*(mm==5),-90*(mm>2)-lcscroll+sbar[2]+rnd_edge[0]));
	sshd.upload_vec2("idx_mod[3]",glm::vec2(xscr0*(mm==5),15*(mm>2)-lcscroll+sbar[3]+rnd_edge[1]));
	sshd.upload_vec2("idx_mod[4]",glm::vec2(xscr1*(mm==5),-15*(mm>2)-lcscroll+sbar[4]+rnd_edge[2]));
	sshd.upload_vec2("idx_mod[5]",glm::vec2(xscr1*(mm==5),-40*(mm>2)-lcscroll+sbar[5]+rnd_edge[3]));
	sshd.upload_vec2("idx_mod[6]",glm::vec2(1280,-40*(mm>2)-lcscroll+sbar[6]+(rand()%10-5)));
	sshd.upload_vec2("idx_mod[7]",glm::vec2(1280,-15*(mm>2)-lcscroll+sbar[7]+(rand()%10-5)));

	title_fb.bind();
	m_frame->clear(0,0,0);
	glDrawArrays(GL_TRIANGLES,6,12);
	title_fb.close();

	// modulation coordinates for selection splash
	/*
		INITIAL		DESTINATION	VERTEX COLOUR	INDEX
		630,0,		630,0,		s_rgb,		0,
		630,0,		0,720,		s_rgb,		1,
		650,0,		0,720,		s_rgb,		2,
		650,0,		650,0,		s_rgb,		3,

	*/

	// render the vertical selection splash
	sshd.upload_vec2("idx_mod[0]",glm::vec2(300*dtrans,0));
	sshd.upload_vec2("idx_mod[1]",glm::vec2(SELTRANS[(mselect-1)*2]*(1-dtrans)+SELTRANS[0]*dtrans,0));
	sshd.upload_vec2("idx_mod[2]",glm::vec2(SELTRANS[(mselect-1)*2+1]*(1-dtrans)+SELTRANS[1]*dtrans,0));
	sshd.upload_vec2("idx_mod[3]",glm::vec2(325*dtrans,0));
	select_fb.bind();
	m_frame->clear(0,0,0);
	glDrawArrays(GL_TRIANGLES,18,6);
	select_fb.close();

	// render the list splash for prism list selection
	bool af = edge_mod==-1;
	int32_t xsll = 5+290*edge_mod,xslr = -5-290*(1-edge_mod); // ??reset and immediate usage, the hell?
	int32_t yslu = (md_disp)*edge_sel+12*(!!edge_sel),ysld = -(md_disp)*edge_sel-12*(!!edge_sel);
	sshd.upload_vec2("idx_mod[0]",glm::vec2(xscr0,-15-lcscroll+sbar[3]+rnd_edge[1]));
	sshd.upload_vec2("idx_mod[1]",glm::vec2(xscr0,-40-lcscroll+sbar[2]+rnd_edge[0]));
	sshd.upload_vec2("idx_mod[2]",glm::vec2(xscr0+xsll*!af+yslu*.1f,
			-40-lcscroll+(sbar[2]+rnd_edge[0])*af-10*!af+yslu));
	sshd.upload_vec2("idx_mod[3]",glm::vec2(xscr0+xsll*!af+yslu*.1f,
			-15-lcscroll+(sbar[3]+rnd_edge[1])*af+10*!af+ysld));
	sshd.upload_vec2("idx_mod[4]",glm::vec2(xscr1+xslr*!af-yslu*.1f,
			-40-lcscroll+(sbar[5]+rnd_edge[3])*af-10*!af+yslu));
	sshd.upload_vec2("idx_mod[5]",glm::vec2(xscr1+xslr*!af-yslu*.1f,
			-15-lcscroll+(sbar[4]+rnd_edge[2])*af+10*!af+ysld));
	sshd.upload_vec2("idx_mod[6]",glm::vec2(xscr1,-40-lcscroll+sbar[5]+rnd_edge[3]));
	sshd.upload_vec2("idx_mod[7]",glm::vec2(xscr1,-15-lcscroll+sbar[4]+rnd_edge[2]));
	cross_fb.bind();
	m_frame->clear(0,0,0);
	glDrawArrays(GL_TRIANGLES,24,18*(mm==5));
	cross_fb.close();
	diffsel = lselect;

	// render menu
	fb.bind();
	m_frame->clear(0,0,0);
	m_r2d->prepare();
	m_r2d->s2d.upload_float("ptrans",ptrans);

	// transform titles
	glm::mat4 tr_model = glm::scale(pos_title,glm::vec3(val_vscl,val_vscl,0));
	tr_model = glm::rotate(tr_model,glm::radians(val_vrot),glm::vec3(0,0,1));
	m_r2d->al.at(0).model = tr_model;	// model matrix for vertical title

	tr_model = glm::scale(pos_entitle,glm::vec3(val_hscl,val_hscl,0));
	tr_model = glm::rotate(tr_model,glm::radians(val_hrot),glm::vec3(0,0,1));
	m_r2d->al.at(1).model = tr_model;	// model matrix for horizontal title

	// render titles
	m_r2d->render_state(0,glm::vec2(vrt_title,0));
	m_r2d->render_state(1,glm::vec2(0,hrz_title));

	// animate title speedup effect
	neg_vscl += (val_vscl>1.1f&&!neg_vscl)-(val_vscl<.9f&&neg_vscl);
	val_vscl += ((rand()%6+6)*.0001f)*(1-2*neg_vscl);	// vertical scale anim
	neg_hscl += (val_hscl>1.1f&&!neg_hscl)-(val_hscl<.9f&&neg_hscl);
	val_hscl += ((rand()%6+6)*.0001f)*(1-2*neg_hscl);	// horizontal scale anim
	neg_vrot += (val_vrot>2&&!neg_vrot)-(val_vrot<-2&&neg_vrot);
	val_vrot += ((rand()%5+10)*.001f)*(1-2*neg_vrot);	// vertical rotation
	neg_hrot += (val_hrot>2&&!neg_hrot)-(val_hrot<-2&&neg_hrot);
	val_hrot += ((rand()%5+10)*.001f)*(1-2*neg_hrot);	// horizontal rotation
	// FIXME: breakdown the rhythm

	// write dare message & version notification
	m_r2d->reset_shader();
	m_r2d->render_sprite(msindex,msindex+14*(mm>0));

	tft.prepare();
	tft.render(50*(1-ptrans),glm::vec4(1,0,0,1));

	vtft.prepare();
	vtft.render(75,glm::vec4(0,0,.5f,1));
	fb.close();

	// render splash overlay
	m_frame->clear(0,0,0);
	fb.render_wOverlay(splash_fb.get_tex(),title_fb.get_tex(),select_fb.get_tex(),cross_fb.get_tex(),ptrans);

	// render menu lists and sublists
	m_r2d->prepare();
	// m_r2d->render_sprite(msindex+14,msindex+18);
	mls[i_ml].render(dtrans,lscroll,lselect,edge_mod,ml_delta,edge_sel,md_disp);

	// render menu dialogue base selector
	m_r2d->prepare();
	bool dopen = dsi_diff||dsi_conf;
	glm::vec3 disp = glm::vec3(235,135,0)*glm::vec3(dsi_diff>0)+glm::vec3(257,285,0)*glm::vec3(dsi_conf>0);
	disp.x += (dsi_diff-(dsi_diff>0))*220+(dsi_conf-(dsi_conf>0))*125;
	glm::mat4 disptrans = glm::translate(glm::mat4(1.0f),disp+glm::vec3(-5,15,0));
	glm::mat4 disprot = glm::rotate(glm::mat4(1.0f),glm::radians(dlgrot_cnt),glm::vec3(0,0,1));
	m_r2d->sl[msindex+18].model = disptrans*disprot;
	m_r2d->render_sprite(msindex+18,msindex+19*dopen);
	dlgrot_cnt -= 7-(dlgrot_cnt<-360)*360;

	// render menu dialogue overlays
	dsi_diff += dlgmod;
	dsi_conf += dlgmod;
	md_diff.render(dsi_diff);
	md_conf.render(dsi_conf);

	// render menu dialogue focused selector
	disptrans = glm::translate(glm::mat4(1.0f),disp);
	disprot = glm::rotate(glm::mat4(1.0f),glm::radians(dlgrot_val),glm::vec3(0,0,1));
	m_r2d->sl[msindex+19].model = disptrans*disprot;
	m_r2d->render_sprite(msindex+19,msindex+20*dopen);
	dlgrot_val += 2-(dlgrot_val>360)*360;
}

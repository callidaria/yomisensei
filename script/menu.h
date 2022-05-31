#pragma once

#include <iostream>

#include "../ccb/frm/frame.h"
#include "../ccb/fcn/ccb_linpr.h"
#include "../ccb/fcn/ccb_manager.h"
#include "../ccb/gfx/renderer2d.h"
#include "../ccb/gfx/renderer3d.h"
#include "../ccb/gfx/rendereri.h"
#include "../ccb/mat/camera2d.h"
#include "../ccb/frm/framebuffer.h"
#include "../ccb/fcn/text.h"
#include "../ccb/fcn/buffer.h"

#include "menu_list.h"
#include "menu_dialogue.h"
#include "game.h"

enum MenuMode
{
	MENU_TITLE,			// rv = 0
	MENU_SELECTION,		// rv = 1
	MENU_START,			// rv = 2
	MENU_DIFFS,			// rv = 3
	MENU_LISTING,		// rv = 4
	MENU_SUBLIST		// rv = 5
};

class Menu
{
public:
	Menu(CCBManager* ccbm,Frame* f,Renderer2D* r2d,Renderer3D* r3d,RendererI* rI,Camera2D* cam2d);
	~Menu();
	void render(uint32_t &running,bool &reboot);
private:
	void fill_dialogue();
private:
	Buffer buffer;
	CCBManager* m_ccbm;
	Frame* m_frame;
	Renderer2D* m_r2d;
	RendererI* m_rI;
	Camera2D* m_cam2d;
	Shader sshd;
	FrameBuffer fb,splash_fb,title_fb,select_fb,cross_fb;
	Text tft,vtft;
	MenuMode mm = MenuMode::MENU_TITLE;
	int32_t lselect = 0,lbounds = 7,lscroll = 0,diffsel = 0,opt_index = 0;
	int32_t sbar[8];
	MenuList mls[12];

	// input definition
	bool *cnt_b,*cnt_start,*cnt_lft,*cnt_rgt,*cnt_dwn,*cnt_up;
	bool trg_start=false,trg_b=false,trg_lft=false,trg_rgt=false,trg_dwn=false,trg_up=false;

	// animation values
	const glm::vec3 TITLE_START = glm::vec3(300,300,0);
	const glm::vec3 ENTITLE_START = glm::vec3(590,800,0);
	const glm::vec3 TITLE_MENU = glm::vec3(100,250,0);
	const glm::vec3 ENTITLE_MENU = glm::vec3(590,642,0);
	uint32_t SELTRANS[16] = { 900,2500, 34,170, 221,380, 419,609, 644,810, 871,997, 1027,1258, 1212,1498 };
	glm::mat4 pos_title,pos_entitle;
	float ptrans=0,dtrans=0,strans=0;
	glm::vec3 title_dir = TITLE_MENU-TITLE_START;
	glm::vec3 entitle_dir = ENTITLE_MENU-ENTITLE_START;  // FIXME: at compile time
	uint32_t msindex;
	uint8_t mselect = 7;
	glm::vec2 mve,mvj;
	float edge_mod;
	bool edge_sel = false;
	uint8_t md_disp = 0;
	uint8_t hrz_title = 0,vrt_title = 3;
	float dlgrot_val = 0,dlgrot_cnt = 0;

	// menu title speedup animation
	bool neg_vscl,neg_hscl,neg_vrot,neg_hrot;
	float val_vscl=1,val_hscl=1,val_vrot=0,val_hrot=0;

	// dialogue relevant variables
	std::vector<const char*> pth_diff = {
		"res/diffs/normal_diff.png","res/diffs/master_diff.png",
		"res/diffs/gmaster_diff.png","res/diffs/rhack_diff.png"
	};  // FIXME: remove those after path has been found
	std::vector<const char*> pth_conf = { "res/ok.png","res/no.png" };
	MenuDialogue md_diff = MenuDialogue(glm::vec2(200,100),880,520,m_r2d,m_cam2d,"select difficulty",pth_diff,150,450);
	MenuDialogue md_conf = MenuDialogue(glm::vec2(200,250),250,150,m_r2d,m_cam2d,"confirm changes?",pth_conf,75,75);
	uint8_t dsi_diff,dsi_conf;

	Game game = Game(m_frame,m_r2d,m_rI,m_cam2d);
	uint8_t difflv = 0;
};
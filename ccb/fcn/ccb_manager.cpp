#include "ccb_manager.h"

/*
	constructor(Frame*,Renderer2D*,Camera2D*)
	frame: frame entity, handling the relevant input for dev console
	r2d: renderer to handle loaded & added 2D entities
	cam2d: reference to 2D camera defining the coordinate system and perspective for renderer
	purpose: build a manager instance, for later usage of developer console and level loading
*/
CCBManager::CCBManager(Frame* frame,Renderer2D* r2d,Camera2D* cam2d)
	: m_frame(frame),m_r2d(r2d),m_cam2d(cam2d)
{
	// create developer mode label
	m_r2d->add(glm::vec2(1230,690),40,20,"res/dev.png");

	// create developer console greetings and initial setup
	ct.add("Welcome to the CASCABEL shell",glm::vec2(750,console_y+20));
	int prog = ct.add('>',glm::vec2(750,console_y));

	// add line error message
	cl.add("lineerr",glm::vec2(700+prog,30));

	// load text instances
	ct.load();cl.load();
}

/*
	add_lv(const char*) -> uint16_t
	path: path to level environment file, specifying object loading
	purpose: loads a cascabel level environment file
	returns: start address of environment objects within 2D renderer memory
*/
uint16_t CCBManager::add_lv(const char* path)
{
	// interpret level environment file
	CCBLInterpreter proc = CCBLInterpreter(m_r2d,path);
	int out = proc.load_level();

	// save to interpreter & renderer memory index list
	linpr.push_back(proc);
	index.push_back(out);
	return out;
}

/*
	rm_lv(uint16_t) -> void
	id: id of the level that shall be removed
	purpose: remove a level from manager by id
*/
void CCBManager::rm_lv(uint16_t id)
{ linpr[id].delete_level(); }

/*
	vanish() -> void
	purpose: close ccb manager and clean memory
*/
void CCBManager::vanish()
{ for (uint16_t i=0;i<linpr.size();i++) rm_lv(i); }

/*
	dev_console(uint32_t&,bool&) -> void
	running: reference to runstate variable to possibly end program from console by setting it to 0
	dactive: reference to boolean holding if developer console should be activated & shown
	purpose: renders and breaths life into developer console
*/
void CCBManager::dev_console(bool &running,bool &dactive)
{
	// draw developer mode label
	m_r2d->prepare();
	m_r2d->render_sprite(0,1);

	// open or close console with comma if not opened or closed last frame
	if (m_frame->kb.ka[SDL_SCANCODE_COMMA]&&!activeonsc) {
		activeonsc = true;	// TODO: initially set this to false and remove later trigger call

		// process
		if (dactive) { m_frame->input_stop();m_frame->tline.pop_back(); }
		else m_frame->input_start();

		// negate console's state of openness
		dactive = !dactive;
	}

	// set if open was requested for next frame
	activeonsc = m_frame->kb.ka[SDL_SCANCODE_COMMA];

	// object movement through user input
	if (mv) {

		// break movement at confirmation
		if (m_frame->kb.ka[SDL_SCANCODE_RETURN]&&!activeonentr) mv = false;
		activeonentr = m_frame->kb.ka[SDL_SCANCODE_RETURN];

		// move sprite according to mouse vector delta
		glm::vec2 deltamv = glm::vec2(m_frame->mouse.mxfr-mlf.x,m_frame->mouse.myfr-mlf.y);
		int ti = index[i_lv]+i_rf;
		m_r2d->sl[ti].model
				= glm::translate(m_r2d->sl[ti].model,glm::vec3(deltamv.x,deltamv.y,0));
		linpr[i_lv].m_pos[i_rf] += deltamv;
		mlf = glm::vec2(m_frame->mouse.mxfr,m_frame->mouse.myfr);
	}

	// object scaling throught user input
	else if (scl) {

		// break scaling through user input
		if (m_frame->kb.ka[SDL_SCANCODE_RETURN]&&!activeonentr) scl = false;
		activeonentr = m_frame->kb.ka[SDL_SCANCODE_RETURN];
		// FIXME: duplicate code. reduce!

		// scale sprite according to mouse movement vector length
		deltascl += (m_frame->mouse.myfr-mlf.y)*0.001f;
		int ti = index[i_lv]+i_rf;
		m_r2d->sl[ti].scale_arbit(deltascl,deltascl);
		linpr[i_lv].m_width[i_rf]=tmp_wscale*deltascl;
		linpr[i_lv].m_height[i_rf]=tmp_hscale*deltascl;
		mlf = glm::vec2(m_frame->mouse.mxfr,m_frame->mouse.myfr);
	}

	// developer console is active and normal due to exclusion of the above cases
	else if (dactive) {

		// input confirmation and processing
		if (m_frame->kb.ka[SDL_SCANCODE_RETURN]&&!activeonentr) {

			// reset confirmation frame trigger
			activeonentr = true;  // FIXME: not really necessary, is it?

			// add command to passive console text lines & scroll
			ct.add(m_frame->tline.c_str(),glm::vec2(770,console_y));
			console_y -= 20;

			// split command & arguments in input string
			std::vector<std::string> args;
			std::string tmp = "";
			for (int i=0;i<m_frame->tline.length();i++) {
				if (m_frame->tline[i]==' ') { 
					args.push_back(tmp);
					tmp = "";
				} else tmp.push_back(m_frame->tline[i]);
			} args.push_back(tmp);

			// program termination recognition
			if (args[0]=="exit") running = false;

			// sprite movement recognition
			else if (args[0]=="mv_sprite") {

				// check if valid argument list size
				if (args.size()!=3)
					ct.add("invalid number of arguments; should be 2",glm::vec2(750,console_y));
				else {

					// enable sprite position change mode
					mv = true;

					// save argument values & origin mouse coordinates to calculate delta from
					mlf = glm::vec2(m_frame->mouse.mxfr,m_frame->mouse.myfr);
					i_lv = std::stoi(args[1]);i_rf = std::stoi(args[2]);
				}
			}

			// sprite scaling recognition
			else if (args[0]=="scl_sprite") {

				// check if valid argument list size
				if (args.size()!=3)
					ct.add("invalid number of arguments; should be 2",glm::vec2(750,console_y));
				else {

					// enable sprite scaling mode
					scl = true;

					// save delta scaling variables & input arguments
					mlf = glm::vec2(m_frame->mouse.mxfr,m_frame->mouse.myfr);
					i_lv = std::stoi(args[1]);i_rf = std::stoi(args[2]);
					tmp_wscale = linpr[i_lv].m_width[i_rf];
					tmp_hscale = linpr[i_lv].m_height[i_rf];
				}
			}

			// request to save changes to .ccb files
			else if (args[0]=="write_changes") {
				for (int i=0;i<linpr.size();i++) linpr[i].write_level();
				ct.add("changes written",glm::vec2(750,console_y));
			}

			// identify all other commands as invalid
			else {
				std::string errline = "invalid command: \""+args[0]+"\"";
				ct.add(errline.c_str(),glm::vec2(750,console_y));
			}  // FIXME: make a dynamically changable command list with function jmp not branching

			// remove input line contents & autoscroll terminal
			m_frame->tline = "";
			console_y -= 20;
			ct.add('>',glm::vec2(750,console_y));
			cscroll = glm::translate(cscroll,glm::vec3(0,40,0));
		}

		// prevent consecutive frame confirmation input
		activeonentr = m_frame->kb.ka[SDL_SCANCODE_RETURN];

		// if mouse button is pressen while console active
		if (m_frame->mouse.mb[0]&&!activeonmcl) {

			// catch mouse coordinates and print them
			std::string mouse_out
				= std::to_string(m_frame->mouse.mxfr)+'x'+std::to_string(m_frame->mouse.myfr);
			ct.add(mouse_out.c_str(),glm::vec2(770,console_y));

			// autoscroll after output
			console_y -= 20;
			cscroll = glm::translate(cscroll,glm::vec3(0,20,0));
		}

		// prevent consecutive frame mouse click printout
		activeonmcl = m_frame->mouse.mb[0];

		// rewrite console input line
		cl.clear(); // ??maybe directly add and remove chars from text instead of rewrite
		cl.add(m_frame->tline.c_str(),glm::vec2(770,30));

		// draw console text above input line & upload scroll
		ct.prepare();
		ct.set_scroll(cscroll);
		ct.render(1024,glm::vec4(0,.7f,0,1));

		// draw console input line
		cl.prepare();
		cl.render(m_frame->tline.size(),glm::vec4(.7f,.7f,.2f,1));
	}
}

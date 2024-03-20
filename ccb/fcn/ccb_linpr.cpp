#include "ccb_linpr.h"

/*
	constructor(Renderer2D*,const char*)
	r2d: points to 2D renderer handling all sprite & anim creation requests
	path: path to environment cascabel file holding all creation requests & parameters
	purpose: saves path & renderer for later usage
*/
CCBLInterpreter::CCBLInterpreter(Renderer2D* r2d,const char* path)
	: m_r2d(r2d),lvpath(path) {  }

/*
	load_level() -> uint16_t
	purpose: process all creation requests held by the cascabel environment file
	returns: starting draw address for 2D renderer
*/
uint16_t CCBLInterpreter::load_level()
{
	// setup
	int out = m_r2d->sl.size();
	std::ifstream lvfile(lvpath,std::ios::in);
	std::vector<std::string> lines;
	std::string tline;

	// get raw level file information
	while (getline(lvfile,tline)) lines.push_back(tline);
	// FIXME: why do i do this again?

	// interpret level file
	for (auto line : lines) {
		std::stringstream ssline(line);
		std::vector<std::string> args;
		std::string arg;
		while (std::getline(ssline,arg,' ')) args.push_back(arg);

		// pattern for spritedef: lh position_x position_y width height texpath
		if (args[0]=="sprite:") {

			// extract values
			glm::vec2 pos = glm::vec2(stoi(args[1]),stoi(args[2]));
			float width = stoi(args[3]),height = stoi(args[4]);
			char* tex_path = new char[args[5].length()+1];
			strcpy(tex_path,args[5].c_str());

			// save values & create sprite
			m_pos.push_back(pos);
			m_width.push_back(width);
			m_height.push_back(height);
			m_tex.push_back(tex_path);
			m_r2d->add(pos,width,height,tex_path);
		}

		// pattern for animdef: lh position_x position_y width height texpath rows columns
		//		frames timesplit
		else if (args[0]=="anim:") {

			// extract values
			glm::vec2 pos = glm::vec2(stoi(args[1]),stoi(args[2]));
			float width = stoi(args[3]),height = stoi(args[4]);
			char* tex_path = new char[args[5].length()+1];
			strcpy(tex_path,args[5].c_str());
			uint8_t r = stoi(args[6]),c = stoi(args[7]),f = stoi(args[8]),t = stoi(args[9]);

			// save values & create animation
			a_pos.push_back(pos);
			a_width.push_back(width);
			a_height.push_back(height);
			a_tex.push_back(tex_path);
			a_row.push_back(r);a_column.push_back(c);
			a_frames.push_back(f);a_ts.push_back(t);
			m_r2d->add(pos,width,height,tex_path,r,c,f,t);
		}
	} return out;
}
// FIXME: duplicate code

/*
	write_level() -> void
	purpose: used by the developer console; writes changes to the creation list
*/
void CCBLInterpreter::write_level()
{
	// write mesh positions
	std::ofstream lvfile(lvpath,std::ios::out);
	for (int i=0;i<m_pos.size();i++) {
		std::stringstream lvbuff;
		lvbuff << "sprite: " << m_pos[i].x << ' ' << m_pos[i].y << ' '
				<< m_width[i] << ' ' << m_height[i] << ' ' << m_tex[i] << '\n';
		lvfile << lvbuff.str();
	}

	// write animation positions
	for (int i=0;i<a_pos.size();i++) {
		std::stringstream lvbuff;
		lvbuff << "anim: " << a_pos[i].x << ' ' << a_pos[i].y << ' '
				<< a_width[i] << ' ' << a_height[i] << ' ' << a_tex[i] << ' '
				<< a_row[i] << ' ' << a_column[i] << ' ' << a_frames[i] << ' ' << a_ts[i] << '\n';
		lvfile << lvbuff.str();
	}
	lvfile.close();
}

/*
	delete_level() -> void
	purpose: manually remove all texture paths from allocated memory
*/
void CCBLInterpreter::delete_level()
{
	for (const char* tex : m_tex) delete[] tex;
	for (const char* tex : a_tex) delete[] tex;
}
// TODO: completely unload all of the level from memory
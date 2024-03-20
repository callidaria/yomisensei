#include "renderer2d.h"

/*
	constructor()
	purpose: create renderer object to subsequently add 2D objects to and draw them
*/
Renderer2D::Renderer2D()
{
	buffer.add_buffer();
} Renderer2D::~Renderer2D() {  }

/*
	add(vec2,float,float,const char*) -> uint16_t
	p: origin position of added sprite
	w: width of added sprite
	h: height of added sprite
	t: path to file containing sprite texture
	purpose: add new sprite to renderer to draw later
	returns: memory index the sprite is to be referenced by when drawing
*/
uint16_t Renderer2D::add(glm::vec2 p,float w,float h,const char* t)
{
	uint16_t out = sl.size();
	sl.push_back(Sprite(p,w,h,t));
	return out;
}

/*
	add(vec2,float,float,const char*,uint8_t,uint8_t,uint8_t,uint8_t) -> uint16_t
	p: origin position of added animation
	w: width of added animation
	h: height of added animation
	t: path to file containing animation spritesheet
	r: rows of spritesheet located by [t]
	c: columns of spritesheet located by [t]
	itn: number of textures held by spritesheet
	f: frames the animation takes to fully iterate through all textures
	purpose: add new animation to renderer to draw later
	returns: memory index the animation is to be referenced by when drawing
*/
uint16_t Renderer2D::add(glm::vec2 p,float w,float h,const char* t,uint8_t r,uint8_t c,
		uint8_t itn,uint8_t f)
{
	uint16_t out = al.size();
	al.push_back(Anim(p,w,h,t,r,c,itn,f));
	return out;
}

/*
	load() -> void
	purpose: combine vertex and texture loading and compile shader program in between
*/
void Renderer2D::load(float &progress,float pseq)
{
	// setup progress bar
	uint32_t ptarget = (pseq*.5f)/(sl.size()+al.size());

	// write sprite vertex values to upload list
	for (int i=0;i<sl.size();i++) {
		std::vector<float> pv = Toolbox::create_sprite_canvas(sl[i].pos,sl[i].sclx,sl[i].scly);
		vertices.insert(vertices.end(),pv.begin(),pv.end());
		Toolbox::generate_elements(i,elements);
		progress += ptarget;
	}

	// write animation vertex values to upload list
	for(int i=0;i<al.size();i++) {
		std::vector<float> pv = Toolbox::create_sprite_canvas(al[i].pos,al[i].sclx,al[i].scly);
		vertices.insert(vertices.end(),pv.begin(),pv.end());
		Toolbox::generate_elements(i+sl.size(),elements);
		progress += ptarget;
	}

	// upload to buffers
	buffer.bind();
	buffer.upload_vertices(vertices),buffer.upload_elements(elements);

	// compile shader
	s2d.compile2d("shader/vertex2d.shader","shader/fragment2d.shader");

	// load textures
	for (int i=0;i<sl.size();i++) sl[i].texture(), progress += ptarget;
	for (int i=0;i<al.size();i++) al[i].texture(), progress += ptarget;
	s2d.upload_int("tex",0);

	// coordinate system
	s2d.upload_camera(Camera2D(1280.0f,720.0f));
}
// TODO: add sensible memory management features

/*
	prepare() -> void
	purpose: prepare shader, buffer & gl settings to render sprites and animations
*/
void Renderer2D::prepare()
{
	s2d.enable();
	reset_shader();
	buffer.bind();
}

/*
	render_sprite(uint16_t,uint16_t) -> void
	b: sprite index to begin the drawing at
	e: sprite index to end the drawing at (sprite with this index is excluded from drawing)
	purpose: draw all sprites within index range, excluding range maximum e:
		the indices are given by the renderer as a return at creation
		add(...) -> [index]
*/
void Renderer2D::render_sprite(uint16_t b,uint16_t e)
{
	for (int i=b;i<e;i++) {
		s2d.upload_matrix("model",sl[i].model);
		glBindTexture(GL_TEXTURE_2D,sl[i].tex);
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(i*6*sizeof(int)));
	}
}

/*
	render_sprite(uint16_t,uint16_t,GLuint) -> void
	b: sprite index to begin the drawing at
	e: sprite index to end the drawing at (sprite with this index is excluded from drawing)
	tex: texture, replacing the loaded texture before render
	purpose: same basic functionality as the normal render_sprite but with previous texture change
*/
void Renderer2D::render_sprite(uint16_t b,uint16_t e,GLuint tex)
{
	for (int i=b;i<e;i++) {
		s2d.upload_matrix("model",sl[i].model);
		glBindTexture(GL_TEXTURE_2D,tex);
		glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(i*6*sizeof(int)));
	}
}
// FIXME: change last parameter from automatically declared unsigned int

/*
	render_state(uint16_t,vec2) -> void
	s: index of the animation previously returned at creation
	i: index of animation frame to permanently render
	purpose: permanently render a specific frame of an animation
*/
void Renderer2D::render_state(uint16_t s,glm::vec2 i)
{
	// load texture data
	glBindTexture(GL_TEXTURE_2D,al[s].tex);

	// upload spritesheet specifications
	s2d.upload_int("row",al[s].r);
	s2d.upload_int("col",al[s].c);
	s2d.upload_vec2("i_tex",i);

	// transform and draw
	s2d.upload_matrix("model",al[s].model);
	glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)((s+sl.size())*6*sizeof(int)));
}

/*
	render_anim(uint16_t) -> void
	i: animation index previously returned at creation
	purpose: render animation and automatically iterate through all textures on the spritesheet
*/
void Renderer2D::render_anim(uint16_t i)
{
	// get rows, columns and current index
	int index = al[i].setup();
	int r = al[i].r;
	int c = al[i].c;

	// get 2D index from address reference integer at current frame
	glm::vec2 ind = glm::vec2((int)(index%c),(int)(index/c));

	// upload spritesheet specifications
	s2d.upload_int("row",r);;
	s2d.upload_int("col",c);
	s2d.upload_vec2("i_tex",ind);

	// transform and draw
	s2d.upload_matrix("model",al[i].model);
	glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)((i+sl.size())*6*sizeof(int)));
}

/*
	reset_shader() -> void
	purpose: reset all animation relevant uniform variables in shader program
*/
void Renderer2D::reset_shader()
{
	s2d.upload_int("row",1);
	s2d.upload_int("col",1);
	s2d.upload_vec2("i_tex",glm::vec2(0,0));
}

#include "instanced_anim.h"

/*
	constructor(vec2,float,float,const char*,uint8_t,uint8_t,uint8_t,uint8_t)
	p: origin position of instance animation source object
	w: standard width of instance animation source object
	h: standard height of instance animation source object
	t: path to texture holding animation or state spritesheet
	row: number of subtexture rows contained by linked spritesheet
	col: number of subtexture columns contained by linked spritesheet
	itn: total number of subtextures contained by linked spritesheet
	f: ticks a total iterations through all subtextures on the spritesheet should take
*/
InstancedAnim::InstancedAnim(glm::vec2 p,float w,float h,const char* t,uint8_t irow,uint8_t icol,
		uint8_t itn,uint8_t f)
	: tpath(t),row(irow),col(icol)
{
	// create canvas & generate texture
	v = Toolbox::create_sprite_canvas_triangled(p,w,h);
	// fill index
	for (int idx=0;idx<IANIMATION_VALUES;idx+=IANIMATION_REPEAT) {
		i[idx] = 0;
		i[idx+1] = 0;
		i[idx+2] = 0;
		i[idx+3] = 1;
	}
	glGenTextures(1,&tex);
}

/*
	texture() -> void
	purpose: textures the instanced animation canvas once the shader has been compiled and bound
*/
void InstancedAnim::texture()
{
	Toolbox::load_texture(tex,tpath);
}

/*
	setup(Shader*) -> void
	shader: shader program to upload saved uniform variables to
	purpose: takes all actions necessary to render instances from source object
*/
void InstancedAnim::setup(Shader* shader)
{
	// upload texture
	glBindTexture(GL_TEXTURE_2D,tex);

	// upload uniform variables
	shader->upload_int("row",row);
	shader->upload_int("col",col);
}
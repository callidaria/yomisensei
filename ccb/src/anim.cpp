#include "../gfx/anim.h"

/*
	constructor(vec2,float,float,const char*,uint8_t,uint8_t,uint8_t,uint8_t)
	[...]
*/
Anim::Anim(glm::vec2 p,float w,float h,const char* t,uint8_t row,uint8_t col,uint8_t itn,uint8_t f)
		: pos(p),sclx(w),scly(h),tp(t),r(row),c(col),fl(f),tn(itn)
{
	// triangle 1
	v[0] = p.x;
	v[1] = p.y+h;
	v[2] = 0.0f;
	v[3] = 0.0f;
	v[4] = p.x+w;
	v[5] = p.y+h;
	v[6] = 1.0f;
	v[7] = 0.0f;

	// triangle 2
	v[8] = p.x+w;
	v[9] = p.y;
	v[10] = 1.0f;
	v[11] = 1.0f;
	v[12] = p.x;
	v[13] = p.y;
	v[14] = 0.0;
	v[15] = 1.0f;  // FIXME: reocurring usage

	// texture generation
	glGenTextures(1,&tex);
}

/*
	texture() -> void
	[...]
*/
void Anim::texture()
{
	glBindTexture(GL_TEXTURE_2D,tex);
	int width,height;

	// load texture data from source
#ifdef __WIN32__

	unsigned char* image = stbi_load(tp,&width,&height,0,STBI_rgb_alpha);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
	stbi_image_free(image);

#else

	unsigned char* image = SOIL_load_image(tp, &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);

#endif

	// texture specification and mipmap
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

/*
	setup() -> uint8_t
	[...]
*/
uint8_t Anim::setup()
{
	glBindTexture(GL_TEXTURE_2D,tex);

	// iterate animation raster
	ft++;
	if(ft>=30)
		ft=0;

	// calculate animation substage
	return ft/(fl/tn);
}
// FIXME: kill the branch. this is loopcode!

/*
	transform(vec2,vec2,float) -> mat4
	[...]
*/
glm::mat4 Anim::transform(glm::vec2 tp,glm::vec2 ts,float tr)
{
	glm::mat4 trans = glm::translate(glm::mat4(1.0f),glm::vec3(pos.x+sclx/2+tp.x,pos.y+scly/2+tp.y,0));
	trans = glm::rotate(trans,glm::radians(tr),glm::vec3(0,0,1));
	trans = glm::scale(trans,glm::vec3(ts.x,ts.y,1));
	trans = glm::translate(trans,glm::vec3(-(pos.x+sclx/2),-(pos.y+scly/2),0));
	return trans;
}
// FIXME: why return transformation and multiply outside if a model matrix member exists

/*
	transform(vec2,vec2,float,vec2) -> mat4
	[...]
*/
glm::mat4 Anim::transform(glm::vec2 tp,glm::vec2 ts,float tr,glm::vec2 a)
{
	float ax=a.x-(pos.x+sclx/2);float ay=a.y-(pos.y+scly/2);
	glm::mat4 trans = glm::translate(glm::mat4(1.0f),
			glm::vec3(pos.x+sclx/2+tp.x+ax,pos.y+scly/2+tp.y+ay,0));
		trans = glm::rotate(trans,glm::radians(tr),glm::vec3(0,0,1));
		trans = glm::scale(trans,glm::vec3(ts.x,ts.y,1));
		trans = glm::translate(trans,glm::vec3(-(pos.x+sclx/2+ax),-(pos.y+scly/2+ay),0));
		return trans;
}

/*
	scale(float,float) -> void
	[...]
*/
void Anim::scale(float wscale,float hscale)
{
	model[0][0] = wscale;
	model[1][1] = hscale;
}
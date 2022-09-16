#include "mesh.h"

/*
	constructor(const char*,const char*,const char*,const char*,const char*,
			vec3,float,vec3,uint16_t*)
	m: path to .obj file to load the mesh vertices from
	t: path to main texture file
	sm: path to specular map file
	nm: path to normal map file
	em: path to emission map file
	ip: the mesh objects origin position
	is: the mesh objects initial scaling
	ir: the mesh objects initial rotation
	mofs: memory offset, describing at which position in vertex memory the created mesh starts
	purpose: create components, read vertex data from source file and calculate normals
*/
Mesh::Mesh(const char* m,const char* t,const char* sm,const char* nm,const char* em,
		glm::vec3 ip,float is,glm::vec3 ir,unsigned int* mofs)
	: texpath(t),smpath(sm),nmpath(nm),empath(em),pos(ip),scl(is),rot(ir),ofs(*mofs)
{
	// generate textures
	glGenTextures(1,&tex);		// object texture
	glGenTextures(1,&specmap);	// specular map
	glGenTextures(1,&normap);	// normal map
	glGenTextures(1,&emitmap);	// light emission map
	// FIXME: disgrace

	// setup vertex information lists
	std::vector<unsigned int> ovi,oui,oni;	// raw reads from source: vertex,uv,normals
	std::vector<glm::vec3> verts,norm;		// position vertices & normals
	std::vector<glm::vec2> uv;				// texture coordinates

	// read source file
	FILE* file = fopen(m,"r");
	while(true) {

		// read next line if exists
		char lh[128];
		int res = fscanf(file,"%s",lh);
		if (res==EOF) break;
		else {

			// check value prefix
			if (strcmp(lh,"v")==0) {			// vertex prefix
				glm::vec3 p;
				fscanf(file,"%f %f %f\n",&p.x,&p.y,&p.z);
				verts.push_back(p);
			} else if (strcmp(lh,"vt")==0) {	// texture coordinate prefix
				glm::vec2 p;
				fscanf(file,"%f %f\n",&p.x,&p.y);
				uv.push_back(p);
			} else if (strcmp(lh,"vn")==0) {	// normals prefix
				glm::vec3 p;
				fscanf(file,"%f %f %f\n",&p.x,&p.y,&p.z);
				norm.push_back(p);
			} else if(strcmp(lh,"f")==0) {		// element index prefix

				// read element node for current triangle
				unsigned int vi[3],ui[3],ni[3];
				fscanf(file,"%d/%d/%d %d/%d/%d %d/%d/%d\n",
						&vi[0],&ui[0],&ni[0],&vi[1],&ui[1],&ni[1],&vi[2],&ui[2],&ni[2]);
				glm::vec3 pproc = glm::vec3(vi[0],vi[1],vi[2]);
				glm::vec3 nproc = glm::vec3(ni[0],ni[1],ni[2]);

				// translate triangle
				transform(pproc,pos,scl,rot);
				rotate(nproc,rot);

				// save values position vertices
				ovi.push_back(vi[0]);ovi.push_back(vi[1]);ovi.push_back(vi[2]);		// position
				oui.push_back(ui[0]);oui.push_back(ui[1]);oui.push_back(ui[2]);		// tex coords
				oni.push_back(ni[0]);oni.push_back(ni[1]);oni.push_back(ni[2]);		// normals
			}
		}
	}

	// translate vertex data to object vertices
	glm::vec3 tg(1.0f),btg(1.0f);
	for(int i=0;i<ovi.size();i++) {

		// precalculations for normal mapping
		if (i%3==0&&ovi.size()) {
			glm::vec3 e1 = verts[ovi[i+1]-1]-verts[ovi[i]-1];
			glm::vec3 e2 = verts[ovi[i+2]-1]-verts[ovi[i]-1];
			glm::vec2 duv1 = uv[oui[i+1]-1]-uv[oui[i]-1];
			glm::vec2 duv2 = uv[oui[i+2]-1]-uv[oui[i]-1];

			// calculate tangent
			float ff = 1.0f/(duv1.x*duv2.y-duv2.x*duv1.y);
			tg.x = ff*(duv2.y*e1.x-duv1.y*e2.x);
			tg.y = ff*(duv2.y*e1.y-duv1.y*e2.y);
			tg.z = ff*(duv2.y*e1.z-duv1.y*e2.z);
			tg = glm::normalize(tg);

			// calculate bitangent
			btg.x = ff*(-duv2.x*e1.x+duv1.x*e2.x);
			btg.y = ff*(-duv2.x*e1.y+duv1.x*e2.y);
			btg.z = ff*(-duv2.x*e1.z+duv1.x*e2.z);
			btg = glm::normalize(btg);
			// FIXME: using a matrix calculation might be significantly faster
		}  // FIXME: remove branch from multiupload

		// get read vertices to process and save
		unsigned int tvi = ovi[i],tui = oui[i],tni = oni[i];
		glm::vec3 tv = verts[tvi-1],tn = norm[tni-1];
		glm::vec2 tu = uv[tui-1];

		// translate vertices & normals
		transform(tv,pos,scl,rot);
		rotate(tn,rot);

		// save data to buffer vector
		v.push_back(tv.x);v.push_back(tv.y);v.push_back(tv.z);		// vertex positions
		v.push_back(tu.x);v.push_back(tu.y);						// texture coordinates
		v.push_back(tn.x);v.push_back(tn.y);v.push_back(tn.z);		// normals
		v.push_back(tg.x);v.push_back(tg.y);v.push_back(tg.z);		// tangents
		v.push_back(btg.x);v.push_back(btg.y);v.push_back(btg.z);	// bitangents
	}

	// save and increase offset for mesh render index
	size = v.size()/14;
	*mofs += size;
} Mesh::~Mesh() {  }

/*
	texture() -> void
	purpose: upload texture data to shader program to be used as sampler2D
*/
void Mesh::texture()
{
	Toolbox::load_texture(tex,texpath);
	Toolbox::load_texture(specmap,smpath);
	Toolbox::load_texture(normap,nmpath);
	Toolbox::load_texture(emitmap,empath);
}

/*
	transform(vec3&,vec3,float,vec3) -> void
	o: vector offset to transform
	p: position vector to add to origin [o] vector position
	s: relative scaling of [o] vector
	r: relative rotation of [o] vector
	helping: constructor(...)
	purpose: transform offset vectors while reading .obj file
*/
void Mesh::transform(glm::vec3 &o,glm::vec3 p,float s,glm::vec3 r)
{
	rotate(o,r);
	o *= s;
	o += p;
}

/*
	rotate(vec3&,vec3) -> void
	o: vector offset to rotate
	r: representative rotation vector around all axis
	helping: constructor(...)
	purpose: rotate offset vectors while reading .obj file
*/
void Mesh::rotate(glm::vec3 &o,glm::vec3 r)
{
	o = glm::rotate(o,glm::radians(r.x),glm::vec3(1,0,0));
	o = glm::rotate(o,glm::radians(r.y),glm::vec3(0,1,0));
	o = glm::rotate(o,glm::radians(r.z),glm::vec3(0,0,1));
}
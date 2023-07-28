#include "mesh_anim.h"

/*
	TODO
*/
MeshAnimation::MeshAnimation(const char* path,const char* itex_path,uint32_t &mofs)
	: tex_path(itex_path)
{

	// texture generation
	glGenTextures(1,&tex);

#ifdef LIGHT_SELFIMPLEMENTATION_COLLADA_LOAD

	// iterate collada file
	std::vector<std::vector<float>> farrs;
	std::vector<float> weight_count,weight_elements;
	std::ifstream file(path);
	std::string line;
	uint8_t stride = 0;
	uint8_t farri_weights;
	bool stride_increment = false;
	while(std::getline(file,line)) {

		// split lines attribute & accumulate stride value
		std::vector<std::string> raw_data = parameters_from_line(line);
		stride += stride_increment;

		// read node definitions
		if (raw_data[0]=="visual_scene") jroot.children.push_back(rc_assemble_joint_hierarchy(file));

		// check bracket type information for float array & save its data
		else if (raw_data[0]=="float_array") farrs.push_back(extract_array_data(raw_data));

		// check for vertex element list
		else if (raw_data[0][0]=='p'&&raw_data[0][1]=='>') {
			vaddress = extract_array_data(raw_data);

			// switch increment & correct stride value
			stride_increment = false;
			stride--;
		}

		// extract weight linking information
		else if (raw_data[0][0]=='v'&&raw_data[0][1]=='c')
			weight_count = extract_array_data(raw_data);
		else if (raw_data[0][0]=='v'&&raw_data[0][1]=='>')
			weight_elements = extract_array_data(raw_data);

		// check for triangle element input definition start
		else if (raw_data[0]=="triangles") stride_increment = true;

		// saving float array indices
		else if (raw_data[0]=="library_controllers>") farri_weights = farrs.size();
	} file.close();

	// insert float array information into vertex array
	uint32_t eprog = 0;
	std::cout << vaddress.size() << "    " << weight_count.size() << '\n';
	for (uint32_t i=0;i<vaddress.size();i+=stride) {

		// basic geometry information
		verts.push_back(farrs[0][vaddress[i]*3]),verts.push_back(farrs[0][vaddress[i]*3+1]),
			verts.push_back(farrs[0][vaddress[i]*3+2]);
		verts.push_back(farrs[2][vaddress[i+2]*2]),verts.push_back(farrs[2][vaddress[i+2]*2+1]);
		verts.push_back(farrs[1][vaddress[i+1]*3]),verts.push_back(farrs[1][vaddress[i+1]*3+1]),
			verts.push_back(farrs[1][vaddress[i+1]*3+2]);

		// joint weights colouring
		const uint32_t joint_count = weight_count[size];
		uint16_t wjoint[joint_count];
		float jweight[joint_count];
		for (uint16_t i=0;i<joint_count;i++) {

			// write joint index & get weight value from element index, also increment progression
			/*wjoint[i] = weight_elements[eprog*2];
			jweight[i] = farrs[farri_weights+1][weight_elements[eprog*2+1]];*/
			eprog++;
		} std::cout << eprog << '\n';

		// increment vertex size
		size++;
	}

	// vertex size & offset
	ofs = mofs;
	mofs += size;

#else

	// load collada file
	Assimp::Importer importer;
	const aiScene* dae_file = importer.ReadFile(path,aiProcess_CalcTangentSpace|aiProcess_Triangulate
			|aiProcess_JoinIdenticalVertices|aiProcess_SortByPType);
	aiMesh* cmesh = dae_file->mMeshes[0];

	// extract animation nodes
	jroot = rc_assemble_joint_hierarchy(dae_file->mRootNode);

	// extract bone armature offset
	bool aofound = false;
	uint16_t armature_offset = rc_get_joint_id(cmesh->mBones[0]->mName.C_Str(),jroot,aofound);
	bone_offsets = std::vector<glm::mat4>(armature_offset,glm::mat4());
	std::cout << "armature bone offset:  " << armature_offset << '\n';
	// TODO: find out how much of a good idea this is in reality

	// assemble bone influence weights
	uint8_t veindex[cmesh->mNumVertices] = { 0 };
	float vbindex[cmesh->mNumVertices][BONE_INFLUENCE_STACK_RANGE] = { 0 };
	float vweight[cmesh->mNumVertices][BONE_INFLUENCE_STACK_RANGE] = { 0 };
	for (uint16_t i=0;i<cmesh->mNumBones;i++) {
		aiBone* cbone = cmesh->mBones[i];
		bone_offsets.push_back(glmify(cbone->mOffsetMatrix));

		// map bone weights onto vertices
		for (uint32_t j=0;j<cbone->mNumWeights;j++) {
			aiVertexWeight cweight = cbone->mWeights[j];

			// store indices & weights until overflow
			if (veindex[cweight.mVertexId]<BONE_INFLUENCE_STACK_RANGE) {
				uint8_t cindex = veindex[cweight.mVertexId]++;
				vbindex[cweight.mVertexId][cindex] = i+armature_offset;
				vweight[cweight.mVertexId][cindex] = cweight.mWeight;

			// handle weight overflow
			} else {
				uint8_t cwidx = 0;
				float cwproc = vweight[cweight.mVertexId][0];
				for (uint8_t ci=1;ci<4;ci++) {
					if (cwproc>vweight[cweight.mVertexId][ci]) {
						cwproc = vweight[cweight.mVertexId][ci];
						cwidx = ci;
					}
				} if (cweight.mWeight>vweight[cweight.mVertexId][cwidx])
					vweight[cweight.mVertexId][cwidx] = cweight.mWeight;
			}
		}

	// assemble vertex array
	} for (uint32_t i=0;i<cmesh->mNumVertices;i++) {

		// extract vertex positions
		aiVector3D position = cmesh->mVertices[i];
		verts.push_back(position.x),verts.push_back(position.y),verts.push_back(position.z);

		// extract texture coordinates
		aiVector3D tex_coords = cmesh->mTextureCoords[0][i];
		verts.push_back(tex_coords.x),verts.push_back(tex_coords.y);

		// extract normals
		aiVector3D normals = cmesh->mNormals[i];
		verts.push_back(normals.x),verts.push_back(normals.y),verts.push_back(normals.z);

		// correct weight array after simplification
		glm::vec4 vrip_weight = glm::vec4(vweight[i][0],vweight[i][1],vweight[i][2],vweight[i][3]);
		glm::normalize(vrip_weight);

		// insert influencing bone indices & relating weights
		verts.push_back(vbindex[i][0]),verts.push_back(vbindex[i][1]),
				verts.push_back(vbindex[i][2]),verts.push_back(vbindex[i][3]);
		verts.push_back(vrip_weight.x),verts.push_back(vrip_weight.y),
				verts.push_back(vrip_weight.z),verts.push_back(vrip_weight.w);

	// assemble element array
	} for (uint32_t i=0;i<cmesh->mNumFaces;i++) {
		aiFace face = cmesh->mFaces[i];
		for (uint32_t j=0;j<face.mNumIndices;j++) elems.push_back(face.mIndices[j]);

	// extract animations
	} for (uint32_t i=0;i<dae_file->mNumAnimations;i++) {
		aiAnimation* canim = dae_file->mAnimations[i];
		ColladaAnimationData proc;
		proc.duration = canim->mDuration;
		proc.delta_ticks = canim->mTicksPerSecond;
		std::cout << proc.duration << ' ' << proc.delta_ticks << '\n';

		// process all related bone keys
		for (uint32_t j=0;j<dae_file->mAnimations[i]->mNumChannels;j++) {
			aiNodeAnim* cnanim = canim->mChannels[j];
			JointKeys cjkey;

			// correlate bone string id with tree location
			bool found = false;
			cjkey.joint_id = rc_get_joint_id(cnanim->mNodeName.C_Str(),jroot,found);

			// add key information
			for (uint32_t k=0;k<cnanim->mNumPositionKeys;k++) {
				cjkey.dur_positions.push_back(cnanim->mPositionKeys[k].mTime);
				cjkey.key_positions.push_back(glmify(cnanim->mPositionKeys[k].mValue));
			} for (uint32_t k=0;k<cnanim->mNumScalingKeys;k++) {
				cjkey.dur_scales.push_back(cnanim->mScalingKeys[k].mTime);
				cjkey.key_scales.push_back(glmify(cnanim->mScalingKeys[k].mValue));
			} for (uint32_t k=0;k<cnanim->mNumRotationKeys;k++) {
				cjkey.dur_rotations.push_back(cnanim->mRotationKeys[k].mTime);
				cjkey.key_rotations.push_back(glmify(cnanim->mRotationKeys[k].mValue));
			}

			// add joint to animation & add animation to list
			proc.joints.push_back(cjkey);
		} anims.push_back(proc);
	}

	// vertex size & offset
	size = elems.size();
	ofs = mofs;
	mofs += size;

#endif

}

/*
	TODO
*/
void MeshAnimation::texture()
{ Toolbox::load_texture_repeat(tex,tex_path,true); }

/*
	TODO
*/
void MeshAnimation::set_animation(uint16_t anim_id)
{
	current_anim = anim_id;
	anim_progression = 0;
}

/*
	TODO
*/
void MeshAnimation::interpolate(Shader* shader,float dt)
{
	// interpolation delta
	avx += dt;
	avx = fmod(avx,anims[current_anim].duration);
	float aprog = (avx/anims[current_anim].duration);

	// iterate all joints for local transformations
	for (auto joint : anims[current_anim].joints) {
		uint16_t iteration_id = 0;
		ColladaJoint* rel_joint = rc_get_joint_object(&jroot,joint.joint_id,iteration_id);

		// determine transformation keyframes
		float tac = (joint.key_positions.size()-1)*aprog;
		float rac = (joint.key_rotations.size()-1)*aprog;
		float sac = (joint.key_scales.size()-1)*aprog;

		// smooth interpolation between keyframes
		glm::vec3 tip = glm::mix(joint.key_positions[tac],joint.key_positions[tac+1],fmod(tac,1.0f));
		glm::quat rip = glm::slerp(joint.key_rotations[rac],joint.key_rotations[rac+1],
				fmod(rac,1.0f));
		glm::vec3 sip = glm::mix(joint.key_scales[sac],joint.key_scales[sac+1],fmod(sac,1.0f));

		// combine translation matrices
		rel_joint->trans = glm::translate(glm::mat4(1),tip)*glm::toMat4(rip)
				* glm::scale(glm::mat4(1),sip);
	}
	// TODO: process interpolation progression with individual key durations
	// FIXME: try mapping instead of finding for interpolation

	// kickstart transformation matrix recursion
	uint16_t tree_id = 0;
	rc_transform_interpolation(shader,jroot,glm::mat4(1),tree_id);
}

/*
	TODO
*/
std::ostream &operator<<(std::ostream &os,const MeshAnimation& obj)
{
	os << "---------------------< MeshAnimation >-----------------------\n";
	os << "vertex array size: " << obj.verts.size() << '\n';
	os << "vertex drawcall count: " << obj.size << "    drawcall offset: " << obj.ofs << '\n';
	os << "joint tree:\n";
	MeshAnimation::rc_print_joint_tree(os,obj.jroot,0);
	return os;
}

#ifdef LIGHT_SELFIMPLEMENTATION_COLLADA_LOAD

/*
	TODO
*/
std::vector<float> MeshAnimation::extract_array_data(std::vector<std::string> raw_data)
{
	// extract & filter first value
	bool nfound = true;
	int8_t vstart = -1;
	size_t findex;
	while (nfound) {
		vstart++;
		findex = raw_data[vstart].find('>');
		nfound = findex==std::string::npos;
	} raw_data[vstart] = raw_data[vstart].substr(findex+1);

	// filter last value
	raw_data.back() = raw_data.back().substr(0,raw_data.back().find('<'));

	// write values to list
	std::vector<float> out;
	for (uint16_t i=vstart;i<raw_data.size();i++) out.push_back(atof(raw_data[i].c_str()));
	return out;
}

/*
	TODO
*/
std::vector<std::string> MeshAnimation::parameters_from_line(std::string line)
{
	uint8_t lstart = line.find('<')+1;
	std::stringstream pline(line.substr(lstart,line.length()-lstart));
	std::vector<std::string> out;
	std::string rdata;
	while (std::getline(pline,rdata,' ')) out.push_back(rdata);
	return out;
}

/*
	TODO
*/
ColladaJoint MeshAnimation::rc_assemble_joint_hierarchy(std::ifstream &file)
{
	// extract node information from file
	ColladaJoint out;
	std::string line;
	while (std::getline(file,line)) {
		std::vector<std::string> raw_data = parameters_from_line(line);

		// check for information end bracket
		if (raw_data[0]=="/node>"||raw_data[0]=="/visual_scene>") break;

		// get translation matrix
		else if (raw_data[0]=="matrix") {
			std::vector<float> mvalues = extract_array_data(raw_data);
			for (uint8_t y=0;y<4;y++) {
				for (uint8_t x=0;x<4;x++)
					out.trans[x][y] = mvalues[y*4+x];
			}
		}

		// recursive children accumulation
		else if (raw_data[0]=="node") {

			// add child recursively
			uint16_t i = out.children.size();
			out.children.push_back(rc_assemble_joint_hierarchy(file));

			// add node information
			out.children[i].id = raw_data[1].substr(4,raw_data[1].length()-5);
		}
	} return out;
}

#else

/*
	TODO
*/
ColladaJoint MeshAnimation::rc_assemble_joint_hierarchy(aiNode* joint)
{
	// get joint name
	ColladaJoint out;
	out.id = joint->mName.C_Str();

	// recursively process children joints & output results
	out.trans = glmify(joint->mTransformation);
	for (uint16_t i=0;i<joint->mNumChildren;i++)
		out.children.push_back(rc_assemble_joint_hierarchy(joint->mChildren[i]));
	return out;
}

#endif

/*
	TODO
*/
uint16_t MeshAnimation::rc_get_joint_id(std::string jname,ColladaJoint cjoint,bool &found)
{
	// check id equivalence
	uint16_t out = 0;
	found = jname==cjoint.id||found;
	out += !found;

	// recursively check children nodes
	for (auto child : cjoint.children) {
		if (found) break;
		out += rc_get_joint_id(jname,child,found);
	} return out;
}

/*
	TODO
*/
ColladaJoint* MeshAnimation::rc_get_joint_object(ColladaJoint* cjoint,uint16_t anim_id,
		uint16_t &curr_id)
{
	ColladaJoint* out;
	uint8_t child_id = 0;
	while (curr_id<=anim_id) {
		if (anim_id==curr_id) {
			curr_id++;
			return cjoint;
		} else if (child_id>=cjoint->children.size()) return nullptr;
		out = rc_get_joint_object(&cjoint->children[child_id],anim_id,++curr_id);
		child_id++;
	} return out;
}

/*
	TODO
*/
void MeshAnimation::rc_transform_interpolation(Shader* shader,ColladaJoint cjoint,glm::mat4 gtrans,
		uint16_t &id)
{
	glm::mat4 lgtrans = gtrans*cjoint.trans;
	glm::mat4 btrans = lgtrans*bone_offsets[id];
	shader->upload_matrix(("joint_transform["+std::to_string(id++)+"]").c_str(),btrans);
	for (auto child : cjoint.children) rc_transform_interpolation(shader,child,lgtrans,id);
}
// TODO: this bone id determination is a big fat hack, a sensible system has yet to be implemented

/*
	TODO
*/
glm::vec3 MeshAnimation::glmify(aiVector3D ivec3)
{ return glm::vec3(ivec3.x,ivec3.y,ivec3.z); }
glm::quat MeshAnimation::glmify(aiQuaternion iquat)
{ return glm::quat(iquat.w,iquat.x,iquat.y,iquat.z); }
glm::mat4 MeshAnimation::glmify(aiMatrix3x3 imat3)
{
	glm::mat4 out;
	out[0][0] = imat3.a1,out[0][1] = imat3.b1,out[0][2] = imat3.c1,out[0][3] = 0,
	out[1][0] = imat3.a2,out[1][1] = imat3.b2,out[1][2] = imat3.c2,out[1][3] = 0,
	out[2][0] = imat3.a3,out[2][1] = imat3.b3,out[2][2] = imat3.c3,out[2][3] = 0,
	out[3][0] = 0,out[3][1] = 0,out[3][2] = 0,out[3][3] = 1;
	return out;
}
glm::mat4 MeshAnimation::glmify(aiMatrix4x4 imat4)
{ return glm::transpose(glm::make_mat4(&imat4.a1)); }

/*
	TODO
*/
void MeshAnimation::rc_print_joint_tree(std::ostream &os,ColladaJoint cjoint,uint8_t depth)
{
	for (auto joint : cjoint.children) {
		for (uint8_t i=0;i<depth;i++) os << "--";
		os << "> " << joint.id << '\n';
		rc_print_joint_tree(os,joint,depth+1);
	}
}
#include "builder.h"
/*------------------------------------------------------------------
	Material
	TODO: additional attributes
	Now we just work with basic attributes...
  ------------------------------------------------------------------*/
#pragma message(__FILE__"("S__LINE__"):>>>>>>>TODO - Add custome attributes for MATERIAL")
CMaterial::CMaterial(const char * name)
{
	m_name = name;

	pMat = NULL; // the cooked struct

	diffuse[0] = diffuse[1] = diffuse[2] = 0.0;
	specexp = 0.0;
	ambient[0] = ambient[1] = ambient[2] = 0.0;
	reflectivity = 0.0;
	transparency[0] = transparency[1] = transparency[2] = 0;
	translucency = 0.0;
	specular[0] = specular[1] = specular[2] = 0.0;

}

void CMaterial::setDiffuse(float r, float g, float b)
{
	diffuse[0] = r; diffuse[1] = g; diffuse[2] = b;
}
void CMaterial::setSpecexp(float s)
{
	specexp = s;
}
void CMaterial::setAmbient(float r, float g, float b)
{
	ambient[0] = r; ambient[1] = g; ambient[2] = b;
}
void CMaterial::setReflectivity(float s)
{
	reflectivity = s;
}
void CMaterial::setTransparency(float r, float g, float b)
{
	transparency[0] = r; transparency[1] = g; transparency[2] = b;
}
void CMaterial::setTranslucency(float s)
{
	translucency = s;
}
void CMaterial::setSpecular(float r, float g, float b)
{
	specular[0] = r; specular[1] = g; specular[2] = b;
}
void CMaterial::setShaderName(LPCSTR shdName, LPCSTR techName)
{
	if(shdName) shaderName = shdName;
	if(techName) techniqueName = techName;
}
void CMaterial::setDiffuseTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) diffuseTexture.filename = filename;
	if(name) diffuseTexture.name = name;
}
void CMaterial::setSpecExpTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) specExpTexture.filename = filename;
	if(name) specExpTexture.name = name;
}
void CMaterial::setAmbientTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) ambientTexture.filename = filename;
	if(name) ambientTexture.name = name;
}
void CMaterial::setReflectivityTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) reflectivityTexture.filename = filename;
	if(name) reflectivityTexture.name = name;
}
void CMaterial::setTransparencyTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) transparencyTexture.filename = filename;
	if(name) transparencyTexture.name = name;
}
void CMaterial::setTranslucencyTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) translucencyTexture.filename = filename;
	if(name) translucencyTexture.name = name;
}
void CMaterial::setSpecularTexture(LPCSTR	name, LPCSTR	filename)
{
	if(filename) specularTexture.filename = filename;
	if(name) specularTexture.name = name;
}

void CMaterial::getDiffuse(float &r, float &g, float &b)
{
	r = diffuse[0];
	g = diffuse[1];
	b = diffuse[2];
}
void CMaterial::getSpecexp(float &s)
{
	s = specexp;
}
void CMaterial::getAmbient(float &r, float &g, float &b)
{
	r = ambient[0];
	g = ambient[1];
	b = ambient[2];
}
void CMaterial::getReflectivity(float &s)
{
	s = reflectivity;
}
void CMaterial::getTransparency(float &r, float &g, float &b)
{
	r = transparency[0];
	g = transparency[1];
	b = transparency[2];
}
void CMaterial::getTranslucency(float &s)
{
	s = translucency;
}
void CMaterial::getSpecular(float &r, float &g, float &b)
{
	r = specular[0];
	g = specular[1];
	b = specular[2];
}
void CMaterial::getShaderName(char* shdName, int shdNameSz, char* techName, int techNameSz)
{
	strcpy_s(shdName, shdNameSz, shaderName.c_str());
}
bool CMaterial::getDiffuseTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(diffuseTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, diffuseTexture.name.c_str());
	strcpy_s(filename, filenameSz, diffuseTexture.filename.c_str());
	return true;
}
bool CMaterial::getSpecExpTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(specExpTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, specExpTexture.name.c_str());
	strcpy_s(filename, filenameSz, specExpTexture.filename.c_str());
	return true;
}
bool CMaterial::getAmbientTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(ambientTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, ambientTexture.name.c_str());
	strcpy_s(filename, filenameSz, ambientTexture.filename.c_str());
	return true;
}
bool CMaterial::getReflectivityTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(reflectivityTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, reflectivityTexture.name.c_str());
	strcpy_s(filename, filenameSz, reflectivityTexture.filename.c_str());
	return true;
}
bool CMaterial::getTransparencyTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(transparencyTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, transparencyTexture.name.c_str());
	strcpy_s(filename, filenameSz, transparencyTexture.filename.c_str());
	return true;
}
bool CMaterial::getTranslucencyTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(translucencyTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, translucencyTexture.name.c_str());
	strcpy_s(filename, filenameSz, translucencyTexture.filename.c_str());
	return true;
}
bool CMaterial::getSpecularTexture(char*	name, int nameSz, char*	filename, int filenameSz)
{
	if(specularTexture.name.empty())
		return false;
	strcpy_s(name, nameSz, specularTexture.name.c_str());
	strcpy_s(filename, filenameSz, specularTexture.filename.c_str());
	return true;
}

// strings have a minimum length of OTHERSTRINGMINSZ but can be longer... so we can later edit them for misc changes
#define STRLENMIN(s) ((s.length()+1) > OTHERSTRINGMINSZ ? s.length()+1 : OTHERSTRINGMINSZ)

size_t	CMaterial::getTotalSize(int &relocationSlots)
{
	relocationSlots += getNumRelocationSlots();
	size_t sz = sizeof(bk3d::Material) 
		+ STRLENMIN(shaderName) 
		+ STRLENMIN(techniqueName)
		+ STRLENMIN(diffuseTexture.name)
		+ STRLENMIN(diffuseTexture.filename)
		+ STRLENMIN(specExpTexture.name)
		+ STRLENMIN(specExpTexture.filename)
		+ STRLENMIN(ambientTexture.name)
		+ STRLENMIN(ambientTexture.filename)
		+ STRLENMIN(reflectivityTexture.name)
		+ STRLENMIN(reflectivityTexture.filename)
		+ STRLENMIN(transparencyTexture.name)
		+ STRLENMIN(transparencyTexture.filename)
		+ STRLENMIN(translucencyTexture.name)
		+ STRLENMIN(translucencyTexture.filename)
		+ STRLENMIN(specularTexture.name)
		+ STRLENMIN(specularTexture.filename);
	DPF(("Material : %d\n", sz));
	return sz;
}
TMaterial *CMaterial::build(bk3d::MaterialPool* pMaterialPool, int ID)
{
	DPF(("--------------\n"));
	//MStatus status;
	int	stringoffset = sizeof(bk3d::Material);

	size_t nodeByteSize = Bk3dPool::cur_bk3dPool->getUsedSize();
	//MFnDependencyNode dn(shd, &status);
	//
	// allocate the structure + additionnal space for strings
	//
	pMat = new(STRLENMIN(techniqueName) + STRLENMIN(shaderName)
	+ STRLENMIN(diffuseTexture.name)
	+ STRLENMIN(diffuseTexture.filename)
	+ STRLENMIN(specExpTexture.name)
	+ STRLENMIN(specExpTexture.filename)
	+ STRLENMIN(ambientTexture.name)
	+ STRLENMIN(ambientTexture.filename)
	+ STRLENMIN(reflectivityTexture.name)
	+ STRLENMIN(reflectivityTexture.filename)
	+ STRLENMIN(transparencyTexture.name)
	+ STRLENMIN(transparencyTexture.filename)
	+ STRLENMIN(translucencyTexture.name)
	+ STRLENMIN(translucencyTexture.filename)
	+ STRLENMIN(specularTexture.name)
	+ STRLENMIN(specularTexture.filename)) TMaterial;
	//
	// Fill names
	//
	SETNAME(pMat, m_name.c_str() ); // default name for the node
    pMat->parentPool = pMaterialPool;
    pMat->ID = ID;

#define COPYSTRING(dest, source)\
	if(source.length() > 0) {\
	pMat->dest = ((char*)pMat) + stringoffset;\
	stringoffset += (((int)source.length()+1) > OTHERSTRINGMINSZ ? (int)source.length()+1 : OTHERSTRINGMINSZ);\
	strcpy(pMat->dest, source.c_str()); };

	COPYSTRING(shaderName, shaderName);
	COPYSTRING(techniqueName, techniqueName);
	COPYSTRING(diffuseTexture.name, diffuseTexture.name);
	COPYSTRING(diffuseTexture.filename, diffuseTexture.filename);
	COPYSTRING(specExpTexture.name, specExpTexture.name);
	COPYSTRING(specExpTexture.filename, specExpTexture.filename);
	COPYSTRING(ambientTexture.name, ambientTexture.name);
	COPYSTRING(ambientTexture.filename, ambientTexture.filename);
	COPYSTRING(reflectivityTexture.name, reflectivityTexture.name);
	COPYSTRING(reflectivityTexture.filename, reflectivityTexture.filename);
	COPYSTRING(transparencyTexture.name, transparencyTexture.name);
	COPYSTRING(transparencyTexture.filename, transparencyTexture.filename);
	COPYSTRING(translucencyTexture.name, translucencyTexture.name);
	COPYSTRING(translucencyTexture.filename, translucencyTexture.filename);
	COPYSTRING(specularTexture.name, specularTexture.name);
	COPYSTRING(specularTexture.filename, specularTexture.filename);

	//
	// Fill properties
	//
    bk3d::MaterialData& matData = pMaterialPool->tableMaterialData.p[ID];
    pMat->pMaterialData = &matData;

	matData.ambient[0] = ambient[0];
	matData.ambient[1] = ambient[1];
	matData.ambient[2] = ambient[2];
	matData.diffuse[0] = diffuse[0];
	matData.diffuse[1] = diffuse[1];
	matData.diffuse[2] = diffuse[2];
	matData.translucency = translucency;
	matData.transparency[0] = transparency[0];
	matData.transparency[1] = transparency[1];
	matData.transparency[2] = transparency[2];
	matData.specular[0] = specular[0];
	matData.specular[1] = specular[1];
	matData.specular[2] = specular[2];
	matData.specexp = specexp;

	DPF(("CMaterial sz : %d\n", nodeByteSize));
	pMat->nextNode = (bk3d::Node*)Bk3dPool::cur_bk3dPool->getAvailablePtr(); // next available ptr... not allocated, yet !
	return pMat;
}
//void	CMaterial::ptrToOffset(bk3d::RelocationTable::Offsets *&relocSlot, bk3d::FileHeader *pHead, bk3d::Material *p)
//{
//	int localSlots = 0;
//	PTR2OFFSET(bk3d::Node, p->nextNode);
//    PTR2OFFSET(char, p->shaderName);
//    PTR2OFFSET(char, p->techniqueName);
//
//	PTR2OFFSET(char, p->diffuseTexture.name);
//	PTR2OFFSET(char, p->diffuseTexture.filename);
//	PTR2OFFSET(char, p->specExpTexture.name);
//	PTR2OFFSET(char, p->specExpTexture.filename);
//	PTR2OFFSET(char, p->ambientTexture.name);
//	PTR2OFFSET(char, p->ambientTexture.filename);
//	PTR2OFFSET(char, p->reflectivityTexture.name);
//	PTR2OFFSET(char, p->reflectivityTexture.filename);
//	PTR2OFFSET(char, p->transparencyTexture.name);
//	PTR2OFFSET(char, p->transparencyTexture.filename);
//	PTR2OFFSET(char, p->translucencyTexture.name);
//	PTR2OFFSET(char, p->translucencyTexture.filename);
//	PTR2OFFSET(char, p->specularTexture.name);
//	PTR2OFFSET(char, p->specularTexture.filename);
//}

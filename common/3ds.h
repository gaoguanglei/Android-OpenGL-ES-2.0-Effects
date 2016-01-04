/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/

#ifndef _3DS_H
#define _3DS_H

#define PRIMARY       0x4D4D

#define OBJECTINFO    0x3D3D				
#define VERSION       0x0002				
#define EDITKEYFRAME  0xB000				

#define MATERIAL	  0xAFFF				
#define OBJECT		  0x4000				


#define MATNAME       0xA000				
#define MATDIFFUSE    0xA020				
#define MATMAP        0xA200				
#define MATMAPFILE    0xA300			

#define OBJECT_MESH   0x4100		


#define OBJECT_VERTICES     0x4110	
#define OBJECT_FACES		0x4120	
#define OBJECT_MATERIAL		0x4130
#define OBJECT_UV			0x4140


#define SCREEN_WIDTH 400								
#define SCREEN_HEIGHT 300								
#define SCREEN_DEPTH 16									


#define MAX_TEXTURES 100							

typedef unsigned char BYTE;
using namespace std;


class CVector3 
{
public:
	float x, y, z;
	CVector3():x(0),y(0),z(0)
	{

	}
};


class CVector2 
{
public:
	float x, y;
};


struct tFace
{
	unsigned short vertIndex[3];		
//	int coordIndex[3];			
};


struct tMaterialInfo
{
	char  strName[255];			// texture name
	char  strFile[255];			// file name
	BYTE  color[3];				// color
	int   texureId;				// tex ID
	float uTile;				// u 
	float vTile;				// v 
	float uOffset;			    // u 
	float vOffset;				// v 
} ;


struct t3DObject 
{
	int  numOfVerts;			// 
	int  numOfFaces;			// 
	int  numTexVertex;			// 
	int  materialID;			// 
	bool bHasTexture;			// 
	char strName[255];			// 
	CVector3  *pVerts;			// 
	CVector3  *pNormals;		// 
	CVector2  *pTexVerts;		// 
	tFace *pFaces;				// 


	void clean()
	{
#define safely_free_mem(p) \
{\
	if(p!=0)\
	{\
		delete []p;\
		p = 0;\
	}\
}
		safely_free_mem(pVerts);
		safely_free_mem(pNormals);
		safely_free_mem(pTexVerts);
		safely_free_mem(pFaces);
		
#undef safely_free_mem 

	}
};

struct t3DModel 
{
	int numOfObjects;					// 
	int numOfMaterials;					// 
	std::vector<tMaterialInfo> pMaterials;	// 
	vector<t3DObject> pObject;			//

	t3DModel():numOfMaterials(0),numOfObjects(0){}
	~t3DModel()
	{
		pMaterials.clear();

		for(vector<t3DObject>::iterator ite = pObject.begin(); ite!=pObject.end(); ite++)
		{
			(*ite).clean();
		}
		pObject.clear();
	}
};

struct tIndices 
{							
	unsigned short a, b, c, bVisible;	
};

struct tChunk
{
	unsigned short int ID;					// 
	unsigned int length;					// 
	unsigned int bytesRead;					// 
	tChunk():ID(-1),length(0),bytesRead(0)
	{

	}
};


class CLoad3DS
{
public:
	CLoad3DS();								//

	bool Import3DS(t3DModel *pModel, const char *strFileName);

private:
	
	int GetString(char *);
	
	void ReadChunk(tChunk *);
	
	void ProcessNextChunk(t3DModel *pModel, tChunk *);
	
	void ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *);
	
	void ProcessNextMaterialChunk(t3DModel *pModel, tChunk *);
	
	void ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk);
	
	void ReadVertices(t3DObject *pObject, tChunk *);
	// 
	void ReadVertexIndices(t3DObject *pObject, tChunk *);
	// 
	void ReadUVCoordinates(t3DObject *pObject, tChunk *);
	// 
	void ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk);
	// 
	void ComputeNormals(t3DModel *pModel);
	//
	void CleanUp();
	// 
	ASSET_ITEM *m_FilePointer;

	tChunk *m_CurrentChunk;
	tChunk *m_TempChunk;
};


#endif



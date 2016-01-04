/************************************************************************************************************
 * Added by gaoguanglei 2012-3-28
 ************************************************************************************************************/
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <platform.h>
#include <math.h>
#include <3ds.h>


CLoad3DS::CLoad3DS()
{
	m_CurrentChunk = new tChunk;				
	m_TempChunk = new tChunk;
}

bool CLoad3DS::Import3DS(t3DModel *pModel, const char *strFileName)
{
	char strMessage[255] = {0};

	m_FilePointer = ASSET_OPEN(strFileName, "rb");

	if(!m_FilePointer) 
	{
		LOGI("%s does not exist", strFileName);
		return false;
	}
	ReadChunk(m_CurrentChunk);

	if (m_CurrentChunk->ID != PRIMARY)
	{
		return false;
	}
	ProcessNextChunk(pModel, m_CurrentChunk);
	ComputeNormals(pModel);

	CleanUp();

	return true;
}


void CLoad3DS::CleanUp()
{

	ASSET_CLOSE(m_FilePointer);
	delete m_CurrentChunk;
	delete m_TempChunk;	
}


void CLoad3DS::ProcessNextChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
	t3DObject newObject = {0};	
	tMaterialInfo newTexture = {0};	
	unsigned int version = 0;
//	int buffer[50000] = {0};

	m_CurrentChunk = new tChunk;		

	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
		ReadChunk(m_CurrentChunk);

		switch (m_CurrentChunk->ID)
		{
		case VERSION:							
			m_CurrentChunk->bytesRead += ASSET_READ(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);

			if (version > 0x03)
			{

			}
//				Assert(0);
//				MessageBox(NULL, "This 3DS file is over version 3 so it may load incorrectly", "Warning", MB_OK);
			break;

		case OBJECTINFO:	
			
			ReadChunk(m_TempChunk);
			m_TempChunk->bytesRead += ASSET_READ(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

			m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;

			ProcessNextChunk(pModel, m_CurrentChunk);
			break;

		case MATERIAL:				
			pModel->numOfMaterials++;

			pModel->pMaterials.push_back(newTexture);

			ProcessNextMaterialChunk(pModel, m_CurrentChunk);
			break;

		case OBJECT:					
		
			pModel->numOfObjects++;
	
			pModel->pObject.push_back(newObject);
			
			memset(&(pModel->pObject[pModel->numOfObjects - 1]), 0, sizeof(t3DObject));

			m_CurrentChunk->bytesRead += GetString(pModel->pObject[pModel->numOfObjects - 1].strName);
			
			ProcessNextObjectChunk(pModel, &(pModel->pObject[pModel->numOfObjects - 1]), m_CurrentChunk);
			break;

		case EDITKEYFRAME:
//			m_CurrentChunk->bytesRead += ASSET_READ(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			ASSET_SEEK(m_FilePointer, m_CurrentChunk->length - m_CurrentChunk->bytesRead, SEEK_CUR);
			m_CurrentChunk->bytesRead += (m_CurrentChunk->length - m_CurrentChunk->bytesRead);
			break;

		default: 
		
//			m_CurrentChunk->bytesRead += ASSET_READ(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			ASSET_SEEK(m_FilePointer, m_CurrentChunk->length - m_CurrentChunk->bytesRead, SEEK_CUR);
			m_CurrentChunk->bytesRead += (m_CurrentChunk->length - m_CurrentChunk->bytesRead);
			break;
		}

		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}


void CLoad3DS::ProcessNextObjectChunk(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
//	int buffer[50000] = {0};
//	int *buffer = new int[50000];

	m_CurrentChunk = new tChunk;

	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
	
		ReadChunk(m_CurrentChunk);

		switch (m_CurrentChunk->ID)
		{
		case OBJECT_MESH:				
			ProcessNextObjectChunk(pModel, pObject, m_CurrentChunk);
			break;

		case OBJECT_VERTICES:		
			ReadVertices(pObject, m_CurrentChunk);
			break;

		case OBJECT_FACES:				
			ReadVertexIndices(pObject, m_CurrentChunk);
			break;

		case OBJECT_MATERIAL:
			ReadObjectMaterial(pModel, pObject, m_CurrentChunk);			
			break;

		case OBJECT_UV:	
			ReadUVCoordinates(pObject, m_CurrentChunk);
			break;

		default:  
//			m_CurrentChunk->bytesRead += ASSET_READ(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			ASSET_SEEK(m_FilePointer, m_CurrentChunk->length - m_CurrentChunk->bytesRead, SEEK_CUR);
			m_CurrentChunk->bytesRead += (m_CurrentChunk->length - m_CurrentChunk->bytesRead);
			break;
		}

		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	delete m_CurrentChunk;
	
	m_CurrentChunk = pPreviousChunk;
//	delete []buffer;
}


void CLoad3DS::ProcessNextMaterialChunk(t3DModel *pModel, tChunk *pPreviousChunk)
{
	int buffer[50000] = {0};			


	m_CurrentChunk = new tChunk;

	while (pPreviousChunk->bytesRead < pPreviousChunk->length)
	{
	
		ReadChunk(m_CurrentChunk);

		switch (m_CurrentChunk->ID)
		{
		case MATNAME:					
			m_CurrentChunk->bytesRead += ASSET_READ(pModel->pMaterials[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;

		case MATDIFFUSE:
			ReadColorChunk(&(pModel->pMaterials[pModel->numOfMaterials - 1]), m_CurrentChunk);
			break;
		
		case MATMAP:
			ProcessNextMaterialChunk(pModel, m_CurrentChunk);
			break;

		case MATMAPFILE:	
			m_CurrentChunk->bytesRead += ASSET_READ(pModel->pMaterials[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		
		default:  
			m_CurrentChunk->bytesRead += ASSET_READ(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
			break;
		}

		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}


void CLoad3DS::ReadChunk(tChunk *pChunk)
{
	pChunk->bytesRead = ASSET_READ(&pChunk->ID, 1, 2, m_FilePointer);
	pChunk->bytesRead += ASSET_READ(&pChunk->length, 1, 4, m_FilePointer);
}


int CLoad3DS::GetString(char *pBuffer)
{
	int index = 0;
	ASSET_READ(pBuffer, 1, 1, m_FilePointer);
	while (*(pBuffer + index++) != 0) {

		ASSET_READ(pBuffer + index, 1, 1, m_FilePointer);
	}
	return strlen(pBuffer) + 1;
}


void CLoad3DS::ReadColorChunk(tMaterialInfo *pMaterial, tChunk *pChunk)
{

	ReadChunk(m_TempChunk);
	m_TempChunk->bytesRead += ASSET_READ(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);
	pChunk->bytesRead += m_TempChunk->bytesRead;
}


void CLoad3DS::ReadVertexIndices(t3DObject *pObject, tChunk *pPreviousChunk)
{
	unsigned short index = 0;		

	pPreviousChunk->bytesRead += ASSET_READ(&pObject->numOfFaces, 1, 2, m_FilePointer);
	pObject->pFaces = new tFace [pObject->numOfFaces];
	memset(pObject->pFaces, 0, sizeof(tFace) * pObject->numOfFaces);

	for(int i = 0; i < pObject->numOfFaces; i++)
	{
		for(int j = 0; j < 4; j++)
		{			
			pPreviousChunk->bytesRead += ASSET_READ(&index, 1, sizeof(index), m_FilePointer);

			if(j < 3)
			{			
				pObject->pFaces[i].vertIndex[j] = index;
			}
		}
	}
}



void CLoad3DS::ReadUVCoordinates(t3DObject *pObject, tChunk *pPreviousChunk)
{
	pPreviousChunk->bytesRead += ASSET_READ(&pObject->numTexVertex, 1, 2, m_FilePointer);
	pObject->pTexVerts = new CVector2 [pObject->numTexVertex];
	pPreviousChunk->bytesRead += ASSET_READ(pObject->pTexVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}

void CLoad3DS::ReadVertices(t3DObject *pObject, tChunk *pPreviousChunk)
{
	pPreviousChunk->bytesRead += ASSET_READ(&(pObject->numOfVerts), 1, 2, m_FilePointer);

	pObject->pVerts = new CVector3 [pObject->numOfVerts];
	memset(pObject->pVerts, 0, sizeof(CVector3) * pObject->numOfVerts);

	pPreviousChunk->bytesRead += ASSET_READ(pObject->pVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);

	for(int i = 0; i < pObject->numOfVerts; i++)
	{
		float fTempY = pObject->pVerts[i].y;
		pObject->pVerts[i].y = pObject->pVerts[i].z;
		pObject->pVerts[i].z = -fTempY;
	}
}


void CLoad3DS::ReadObjectMaterial(t3DModel *pModel, t3DObject *pObject, tChunk *pPreviousChunk)
{
	char strMaterial[255] = {0};
	int buffer[50000] = {0};	


	pPreviousChunk->bytesRead += GetString(strMaterial);

	for(int i = 0; i < pModel->numOfMaterials; i++)
	{
		if(strcmp(strMaterial, pModel->pMaterials[i].strName) == 0)
		{
			pObject->materialID = i;
			if(strlen(pModel->pMaterials[i].strFile) > 0) {
				pObject->bHasTexture = true;
			}	
			break;
		}
		else
		{
			pObject->materialID = -1;
		}
	}

	pPreviousChunk->bytesRead += ASSET_READ(buffer, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}			

#define Mag(Normal) (sqrt(Normal.x*Normal.x + Normal.y*Normal.y + Normal.z*Normal.z))


CVector3 SubVector(CVector3 vPoint1, CVector3 vPoint2)
{
	CVector3 vVector;							

	vVector.x = vPoint1.x - vPoint2.x;			
	vVector.y = vPoint1.y - vPoint2.y;			
	vVector.z = vPoint1.z - vPoint2.z;			

	return vVector;								
}


void AddToVector(CVector3& target, const CVector3& source)
{
	target.x += source.x;
	target.y += source.y;
	target.z += source.z;
}



CVector3 Cross(const CVector3& vVector1, const CVector3& vVector2)
{
	CVector3 vCross;								
												
	vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
												
	vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
												
	vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));

	return vCross;								
}


void Normalize( CVector3& vNormal)
{
	double Magnitude;							

	Magnitude = Mag(vNormal);		
	vNormal.x /= (float)Magnitude;				
	vNormal.y /= (float)Magnitude;				
	vNormal.z /= (float)Magnitude;				
							
}



void CLoad3DS::ComputeNormals(t3DModel *pModel)
{
	int i;
	CVector3 vVector1, vVector2, vNormal, vPoly[3];

	if(pModel->numOfObjects <= 0)
		return;

	for(int index = 0; index < pModel->numOfObjects; index++)
	{

		t3DObject *pObject = &(pModel->pObject[index]);

		pObject->pNormals = new CVector3 [pObject->numOfVerts];

		for(i=0; i < pObject->numOfFaces; i++)
		{								
			unsigned short* pIdx = pObject->pFaces[i].vertIndex;
			vPoly[0] = pObject->pVerts[pIdx[0]];
			vPoly[1] = pObject->pVerts[pIdx[1]];
			vPoly[2] = pObject->pVerts[pIdx[2]];

			vVector1 = SubVector(vPoly[0], vPoly[1]);		
			vVector2 = SubVector(vPoly[0], vPoly[2]);

			vNormal  = Cross(vVector1, vVector2);	
			Normalize(vNormal);	

			AddToVector(pObject->pNormals[pIdx[0]], vNormal);
			AddToVector(pObject->pNormals[pIdx[1]], vNormal);
			AddToVector(pObject->pNormals[pIdx[2]], vNormal);
		}

		for (i = 0; i < pObject->numOfVerts; i++)			
		{
			Normalize(pObject->pNormals[i]);											
		}

	}
}


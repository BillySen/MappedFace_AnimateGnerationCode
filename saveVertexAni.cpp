//UINT: unsigned int


void KG3D_SaveVertexAnimation(KG3D_MESH_FILE_DATA** pFileData, UINT uNum, const char* pcszAniFilePath)
{
	//XMFLOAT3为一个类型声明 等同于typedef
	using XMFLOAT3 = DirectX::XMFLOAT3;

	//HRESULT 常被用作COM调用的返回值。充分利用HRESULT返回信息可以帮助提高我们的代码质量，提供程序的健壮性。
	//E_FAIL 未指定的失败 0x80004005 一般来讲是函数调用错误。需要用（hr == E_FAIL） 来判断　
	HRESULT hrResult = E_FAIL;

	FILE* pFile = nullptr;
	//unsigned long 64位中为8字节
	size_t uSize = 0;

	//DWORD代表unsigned long  64位机上为64位
	DWORD dwVertexNum = pFileData[0]->dwVertexCount;
	DWORD dwFrameNum = uNum;

	DWORD dwRealAnimatedVertexNum = 0;
	DWORD dwStaticVertexNum = 0;
	DWORD dwBlockLength = 0;

	/*
	struct _ANI_FILE_HEADER
	{
        DWORD dwMask;
        DWORD dwBlockLength;
        DWORD dwNumAnimations;
        DWORD dwType;
        char  strDesc[ANI_STRING_SIZE];
	};

	struct _VERTEX_ANI_VERSION2
	{
    DWORD dwNumVertices;
    DWORD dwNumAnimatedVertices;
    DWORD dwNumFrames;
    float fFrameLength;
    DWORD dwRealAnimatedVertex;
	};
	*/
	_ANI_FILE_HEADER aniHeader;
	_VERTEX_ANI_VERSION2 vetrexAniHeader;

	XMFLOAT3* pAnimatedVertexPos = KG3D_NewArray(nullptr, XMFLOAT3, dwFrameNum * dwVertexNum);
	XMFLOAT3* pInitVertexPos = nullptr;
	XMFLOAT3* pRealAnimatedVertexPos = nullptr;
	DWORD* pVertexIndex = nullptr;
	DWORD* pRealVertexIndex = nullptr;

	std::vector<XMFLOAT3> vecVetex;

	// ani file header
	aniHeader.dwMask = std::vector<XMFLOAT3> vecVetex;;	
	aniHeader.dwNumAnimations = 1;
	aniHeader.dwType = ANIMATION_VERTICES;
	strcpy_s(aniHeader.strDesc, _countof(aniHeader.strDesc), "test");
	dwBlockLength += sizeof(_ANI_FILE_HEADER);


	// vertex ani file header
	vetrexAniHeader.dwNumVertices = dwVertexNum;
	vetrexAniHeader.dwNumAnimatedVertices = dwVertexNum;
	vetrexAniHeader.dwNumFrames = dwFrameNum;
	vetrexAniHeader.fFrameLength = 1000 / 30;	
	dwBlockLength += sizeof(_VERTEX_ANI_VERSION2);

	pVertexIndex = KG3D_NewArray(nullptr, DWORD, dwVertexNum);
	for (DWORD i = 0; i < dwVertexNum; ++i)
	{
		pVertexIndex[i] = i;
	}
	dwBlockLength += sizeof(DWORD) * dwVertexNum;


	vecVetex.reserve(dwVertexNum * dwFrameNum);
	for (DWORD i = 0; i < dwVertexNum; ++i)
	{
		for (UINT j = 0; j < dwFrameNum; ++j)
		{
			vecVetex.emplace_back(pFileData[j]->pPos[i]);
		}
	}
	memcpy(pAnimatedVertexPos, &vecVetex.begin()[0], sizeof(XMFLOAT3) * dwFrameNum * dwVertexNum);
	

	pRealVertexIndex = KG3D_NewArray(nullptr, DWORD, dwVertexNum);
	for (DWORD i = 0; i < dwVertexNum; ++i)
	{
		BOOL bEqual = TRUE;
		pRealVertexIndex[i] = -1;

		for (DWORD j = 1; j < dwFrameNum; ++j)
		{
			if (!XMFloat3Equal(pAnimatedVertexPos[i * dwFrameNum + j - 1], pAnimatedVertexPos[i * dwFrameNum + j]))
			{
				bEqual = FALSE;
				break;
			}
			
		}

		if (!bEqual)
		{
			pRealVertexIndex[i] = i;
			++dwRealAnimatedVertexNum;
		}

	}
	dwBlockLength += sizeof(DWORD) * dwVertexNum;
	

	vetrexAniHeader.dwRealAnimatedVertex = dwRealAnimatedVertexNum;
	if (dwRealAnimatedVertexNum < dwVertexNum)
	{
		dwStaticVertexNum = dwVertexNum - dwRealAnimatedVertexNum;
		pInitVertexPos = KG3D_NewArray(nullptr, XMFLOAT3, dwStaticVertexNum);

		for (DWORD i = 0, j = 0; i < dwVertexNum && j < dwStaticVertexNum; ++i)
		{
			if (pRealVertexIndex[i] == -1)
			{
				memcpy(pInitVertexPos + j, pAnimatedVertexPos + (i * dwFrameNum), sizeof(XMFLOAT3));
				++j;
			}
		}

		dwBlockLength += sizeof(XMFLOAT3) * dwStaticVertexNum;
	}

	if (dwRealAnimatedVertexNum > 0)
	{
		pRealAnimatedVertexPos = KG3D_NewArray(nullptr, XMFLOAT3, dwRealAnimatedVertexNum * dwFrameNum);

		for (DWORD i = 0, k = 0; i < dwVertexNum && k < dwRealAnimatedVertexNum; ++i)
		{
			if (pRealVertexIndex[i] == -1)
				continue;

			for (DWORD j = 0; j < dwFrameNum; ++j)
			{
				memcpy(&pRealAnimatedVertexPos[k * dwFrameNum + j], &(pFileData[j]->pPos[i]), sizeof(XMFLOAT3));
			}

			++k;
		}

		dwBlockLength += sizeof(XMFLOAT3) * dwRealAnimatedVertexNum * dwFrameNum;
	}


	dwBlockLength += sizeof(DWORD); // end flag

	aniHeader.dwBlockLength = dwBlockLength;

	// write
	{
		fopen_s(&pFile, pcszAniFilePath, "wb");
		KGLOG_PROCESS_ERROR(pFile);
 
		uSize = fwrite(&aniHeader, sizeof(_ANI_FILE_HEADER), 1, pFile);
		KGLOG_PROCESS_ERROR(uSize == 1);

		uSize = fwrite(&vetrexAniHeader, sizeof(_VERTEX_ANI_VERSION2), 1, pFile);
		KGLOG_PROCESS_ERROR(uSize == 1);

		uSize = fwrite(pVertexIndex, sizeof(DWORD) * dwVertexNum, 1, pFile);
		KGLOG_PROCESS_ERROR(uSize == 1);

		uSize = fwrite(pRealVertexIndex, sizeof(DWORD) * dwVertexNum, 1, pFile);
		KGLOG_PROCESS_ERROR(uSize == 1);

		if (dwStaticVertexNum > 0)
		{
			uSize = fwrite(pInitVertexPos, sizeof(XMFLOAT3) * dwStaticVertexNum, 1, pFile);
			KGLOG_PROCESS_ERROR(uSize == 1);
		}

		if (dwRealAnimatedVertexNum > 0)
		{
			uSize = fwrite(pRealAnimatedVertexPos, sizeof(XMFLOAT3) * dwRealAnimatedVertexNum * dwFrameNum, 1, pFile);
			KGLOG_PROCESS_ERROR(uSize == 1);
		}

		uSize = fwrite(&ANI_FILE_END_FLAG, sizeof(DWORD), 1, pFile);
		KGLOG_PROCESS_ERROR(uSize == 1);
	}


	hrResult = S_OK;
Exit0:
	fclose(pFile);
	KG3D_DELETE_ARRAY(nullptr, pRealVertexIndex);
	KG3D_DELETE_ARRAY(nullptr, pAnimatedVertexPos);	
}

/*
1.
errno_t fopen_s( FILE** pFile, const char *filename, const char *mode )
pFile 文件指针将接收到打开的文件指针指向的指针
infilename 文件名
inmode 允许的访问类型

2.
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
ptr-- 这是指向要被写入的元素数组的指针。
size-- 这是要被写入的每个元素的大小，以字节为单位。
nmemb-- 这是元素的个数，每个元素的大小为 size 字节。
stream-- 这是指向 FILE 对象的指针，该 FILE 对象指定了一个输出流。

3.void *memcpy(void *str1, const void *str2, size_t n)
str1 -- 指向用于存储复制内容的目标数组，类型强制转换为 void* 指针。
str2 -- 指向要复制的数据源，类型强制转换为 void* 指针。
n -- 要被复制的字节数。


const unsigned        ANI_FILE_MASK = 0x414E494D;
const unsigned        ANI_FILE_MASK_VERVION2 = 0x324E494D;
const unsigned        ANI_FILE_MASK_COMPRESS = 0x434E494D;//Version1.0的压缩版
const unsigned        ANI_FILE_MASK_VERVION2_COMPRESS = 0x343E393D;//Version2.0的压缩版

const float           CRPRECISION = 1.f / SHRT_MAX;
const DWORD           ANI_FILE_END_FLAG = 0xFFFFFFFF;
*/
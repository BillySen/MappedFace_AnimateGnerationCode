void KG3D_SaveVertexAnimation(KG3D_MESH_FILE_DATA** pFileData, UINT uNum, const char* pcszAniFilePath)
{
	using XMFLOAT3 = DirectX::XMFLOAT3;

	HRESULT hrResult = E_FAIL;

	FILE* pFile = nullptr;
	size_t uSize = 0;


	DWORD dwVertexNum = pFileData[0]->dwVertexCount;
	DWORD dwFrameNum = uNum;

	DWORD dwRealAnimatedVertexNum = 0;
	DWORD dwStaticVertexNum = 0;
	DWORD dwBlockLength = 0;

	_ANI_FILE_HEADER aniHeader;
	_VERTEX_ANI_VERSION2 vetrexAniHeader;

	XMFLOAT3* pAnimatedVertexPos = KG3D_NewArray(nullptr, XMFLOAT3, dwFrameNum * dwVertexNum);
	XMFLOAT3* pInitVertexPos = nullptr;
	XMFLOAT3* pRealAnimatedVertexPos = nullptr;
	DWORD* pVertexIndex = nullptr;
	DWORD* pRealVertexIndex = nullptr;

	std::vector<XMFLOAT3> vecVetex;

	// ani file header
	aniHeader.dwMask = ANI_FILE_MASK_VERVION2;	
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
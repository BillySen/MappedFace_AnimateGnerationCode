import os
import struct

import numpy as np
import sys
import pickle
from ctypes import *
from dataclasses import dataclass
from enum import Enum

'''
--声明：
@函数：
    void KG3D_SaveVertexAnimation(KG3D_MESH_FILE_DATA** pFileData, UINT uNum, const char* pcszAniFilePath)
@变量处理：
    KG3D_MESH_FILE_DATA -> KG3Dmeshdata
    用 abs for converting unsigned to signed in python
@结构体：
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
'''

#枚举dwType的类型
TYPES = {"ANIMATION_VERTICES": 1}
def get(key):
    return TYPES.get(key)

@dataclass()
class XMFLOAT3(): #对应using XMFLOAT3 = DirectX::XMFLOAT3; 实现float点构造
    x: float
    y: float
    z: float

class _ANI_FILE_HEADER():
    dwMask: int
    dwBlockLength: int
    dwNumAnimations: int
    dwType: int
    strDesc: list  #char strDesc[ANI_STRING_SIZE]

class _VERTEX_ANI_VERSION2():
    dwNumVertices: int
    dwNumAnimatedVertices: int
    dwNumFrames: int
    dwRealAnimatedVertex: int
    fFrameLength: float

def XMFloat3Equal(p1,p2):
    if p1.all() == p2.all():
        return True
    else:
        return False

uNum = c_uint #需要unsigned int 类型
#pcszAniFilePath = POINTER(c_char) # 对应const char* pcszAniFilePath

def read_mesh():
    path = './res'
    path_list = os.listdir(path)

    pp = []
    for filename in path_list:
        # f = open(os.path.join(path, filename), 'rb')
        with open(os.path.join(path, filename)) as file:
            points = []
            while 1:
                line = file.readline()
                if not line:
                    break
                strs = line.split(" ")
                if strs[0] == "v":
                    points.append((float(strs[1]), float(strs[2]), float(strs[3])))

        # targets_points = np.array(points)

        pp.append(points)
    pp = np.array(pp)
    print(pp.shape)
    return pp

def KG3D_SaveVertexAnimation(uNum, pcszAniFilePath):
    #FILE* pFile = nullptr 先不定义
    uSize = 0

    #DWORD类型的相关变量
    # dwVertexNum = pFileData[0]->dwVertexCount;
    dwVertexNum = 3646
    dwFrameNum = uNum
    # dwRealAnimatedVertexNum = c_ulong(0)
    # dwStaticVertexNum = c_ulong(0)
    # dwBlockLength = c_ulong(0)

    dwRealAnimatedVertexNum = 0
    dwStaticVertexNum = 0
    dwBlockLength = 0

    aniHeader = _ANI_FILE_HEADER()
    vetrexAniHeader = _VERTEX_ANI_VERSION2()

    '''
    XMFLOAT3* pAnimatedVertexPos = KG3D_NewArray(nullptr, XMFLOAT3, dwFrameNum * dwVertexNum); 没被用过
	XMFLOAT3* pInitVertexPos = nullptr; 没被用过
	XMFLOAT3* pRealAnimatedVertexPos = nullptr; 没被用过
	DWORD* pVertexIndex = nullptr; 没被用过
	DWORD* pRealVertexIndex = nullptr; 没被用过
    '''
    pAnimatedVertexPos = [] #对应 XMFLOAT3* pAnimatedVertexPos = KG3D_NewArray(nullptr, XMFLOAT3, dwFrameNum * dwVertexNum);
    vecVetex = [] #对应 std::vector<XMFLOAT3> vecVetex 用append添加实例对象

    #ani file header
    aniHeader.dwMask = vecVetex
    aniHeader.dwNumAnimations = 1
    aniHeader.dwType = get('ANIMATION_VERTICES') #一个type对应自定义的类型码
    aniHeader.strDesc = "test" #strcpy_s(aniHeader.strDesc, _countof(aniHeader.strDesc), "test");
    #dwBlockLength += sys.getsizeof(_ANI_FILE_HEADER);

    #vertex ani file header
    vetrexAniHeader.dwNumVertices = dwVertexNum
    vetrexAniHeader.dwNumAnimatedVertices = dwVertexNum
    vetrexAniHeader.dwNumFrames = dwFrameNum
    vetrexAniHeader.fFrameLength = 1000 / 30
    #dwBlockLength += sys.getsizeof(_VERTEX_ANI_VERSION2)

    pVertexIndex = [i for i in range(dwVertexNum)]



    # for i in range(dwVertexNum):
    #     pVertexIndex[i] = i
    #dwBlockLength += sys.getsizeof(int) * dwVertexNum

    # vecVetex.reserve(dwVertexNum * dwFrameNum); vector的方法，分配足够的内存，python中自动扩容 目前先不考虑这一行

    # for i in range(dwVertexNum):
    #     for j in range(dwFrameNum):
    #         vecVetex.append(pFileData[j]->pPos[i])
    vecVetex = read_mesh()
    #memcpy(pAnimatedVertexPos, &vecVetex.begin()[0], sizeof(XMFLOAT3) * dwFrameNum * dwVertexNum); 应该不用考虑内存分配
    pAnimatedVertexPos = vecVetex

    # for i in range(dwVertexNum):
    #     bEqual = True
    #     # pRealVertexIndex[i] = -1
    #     for j in range(1,dwFrameNum):
    #         # if (not(XMFloat3Equal(pAnimatedVertexPos[i * dwFrameNum + j - 1], pAnimatedVertexPos[i * dwFrameNum + j]))):
    #         if (pAnimatedVertexPos[i * dwFrameNum + j - 1].all() != pAnimatedVertexPos[i * dwFrameNum + j].all()):
    #             bEqual = False
    #             break
    #         if(not(bEqual)):
    #             pRealVertexIndex[i] = i
    #             dwRealAnimatedVertexNum += 1
    pRealVertexIndex = [-1] * dwVertexNum
    for i in range(vecVetex.shape[1]):
        bEqual = True
        for j in range(vecVetex.shape[0]):
            if j == 0:
                continue
            if not((vecVetex[j - 1][i] == vecVetex[j][i]).all()):
                bEqual = False
                break
        if not(bEqual):
            pRealVertexIndex[i] = i
            dwRealAnimatedVertexNum += 1

    j = 0
    for i in pRealVertexIndex:
        if i != -1:
            j = j + 1
    print("j =", j)


    #dwBlockLength += sys.getsizeof(c_ulong) * dwVertexNum
    pInitVertexPos = []
    vetrexAniHeader.dwRealAnimatedVertex = dwRealAnimatedVertexNum
    if dwRealAnimatedVertexNum < dwVertexNum:
        dwStaticVertexNum = dwVertexNum - dwRealAnimatedVertexNum

        j = 0
        for i in range(dwVertexNum):
            if(j < dwStaticVertexNum and pRealVertexIndex[i] == -1):
                #memcpy(pInitVertexPos + j, pAnimatedVertexPos + (i * dwFrameNum), sizeof(XMFLOAT3))
                pInitVertexPos = pAnimatedVertexPos
                j += 1
            #dwBlockLength += sys.getsizeof(XMFLOAT3) * dwStaticVertexNum


    # if dwRealAnimatedVertexNum > 0:
    #     pRealAnimatedVertexPos = []
    #     k = 0
    #     if k < dwRealAnimatedVertexNum:
    #         for i in range(dwVertexNum):
    #             if pRealVertexIndex[i] == -1:
    #                 continue
    #             for j in range(dwFrameNum):
    #                 #memcpy( & pRealAnimatedVertexPos[k * dwFrameNum + j], & (pFileData[j]->pPos[i]), sizeof(XMFLOAT3)); 赋值
    #                 pRealAnimatedVertexPos[k * dwFrameNum + j] = read_mesh()[j][i]
    #             k += 1
    pRealAnimatedVertexPos = []
    k = 0
    if k < dwRealAnimatedVertexNum:
        for j in range(vecVetex.shape[1]):
                if pRealVertexIndex[j] == -1:
                    continue
                for i in range(vecVetex.shape[0]):
                    pRealAnimatedVertexPos.append(vecVetex[i][j])

            #dwBlockLength += sys.getsizeof(XMFLOAT3) * dwRealAnimatedVertexNum * dwFrameNum

    #dwBlockLength += sys.getsizeof(c_ulong)

    aniHeader.dwBlockLength = dwBlockLength

    #write
    with open(pcszAniFilePath,"wb") as f:
        #uSize = fwrite( & aniHeader, sizeof(_ANI_FILE_HEADER), 1, pFile) 类对象
        pickle.dump(aniHeader,f,pickle.HIGHEST_PROTOCOL)
        #uSize = fwrite( & vetrexAniHeader, sizeof(_VERTEX_ANI_VERSION2), 1, pFile) 类对象
        pickle.dump(vetrexAniHeader, f, pickle.HIGHEST_PROTOCOL)

        #uSize = fwrite(pVertexIndex, sizeof(DWORD) * dwVertexNum, 1, pFile) 写入list内容
        for element in pVertexIndex:
            f.write(bytes(element))
        #uSize = fwrite(pRealVertexIndex, sizeof(DWORD) * dwVertexNum, 1, pFile)
        for element in pRealVertexIndex:
            s = struct.pack('h', element)
            f.write(s)

            # f.write(bytes(element))

        if dwStaticVertexNum > 0:
            #uSize = fwrite(pInitVertexPos, sizeof(XMFLOAT3) * dwStaticVertexNum, 1, pFile)
            for element in pInitVertexPos:
                for elem in element:
                    for ele in elem:
                        s = struct.pack('d', ele)
                        f.write(s)

        if dwRealAnimatedVertexNum > 0:
            #uSize = fwrite(pRealAnimatedVertexPos, sizeof(XMFLOAT3) * dwRealAnimatedVertexNum * dwFrameNum, 1, pFile)
            for element in pRealAnimatedVertexPos:
                for ele in element:
                    s = struct.pack('d', ele)
                    f.write(s)

        f.write(b'0xFFFFFFFF')

    # hrResult = S_OK;

    f.close()
    del pRealVertexIndex
    del pRealAnimatedVertexPos

if __name__ == '__main__':
    # point1 = XMFLOAT3(5.2222,3.58,694.4)
    # print(point1)
    # print("size of int: ", sys.getsizeof(int))
    # print("size of XMFLOAT3: ", sys.getsizeof(XMFLOAT3))
    # print("size of _ANI_FILE_HEADER: ",sys.getsizeof(_ANI_FILE_HEADER))
    # print("size of : _VERTEX_ANI_VERSION2: ",sys.getsizeof( _VERTEX_ANI_VERSION2))

    # KG3D_SaveVertexAnimation(93, "./anifile.exe")
    print(sys.getsizeof("sizeof ANIFILEHEADER:",_ANI_FILE_HEADER))
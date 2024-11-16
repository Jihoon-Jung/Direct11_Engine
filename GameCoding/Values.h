#pragma once
#include "Types.h"

//const int32 GWinSizeX = 1920;// 1600;
//const int32 GWinSizeY = 1040;// 900;

// �۾� ǥ������ ������ ȭ�� ũ�⸦ ��� �Լ�
inline void GetWorkAreaSize(int32& width, int32& height)
{
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    width = workArea.right - workArea.left;
    height = workArea.bottom - workArea.top;
}

// ���� ���� �ʱ�ȭ
inline int32 GWinSizeX = 1600;  // �⺻��
inline int32 GWinSizeY = 900;   // �⺻��

// ������ ũ�� �ʱ�ȭ �Լ�
inline void InitializeWindowSize()
{
    GetWorkAreaSize(GWinSizeX, GWinSizeY);
}
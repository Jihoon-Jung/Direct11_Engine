#pragma once
#include "Types.h"

//const int32 GWinSizeX = 1920;// 1600;
//const int32 GWinSizeY = 1040;// 900;

// 작업 표시줄을 제외한 화면 크기를 얻는 함수
inline void GetWorkAreaSize(int32& width, int32& height)
{
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    width = workArea.right - workArea.left;
    height = workArea.bottom - workArea.top;
}

// 전역 변수 초기화
inline int32 GWinSizeX = 1600;  // 기본값
inline int32 GWinSizeY = 900;   // 기본값

// 윈도우 크기 초기화 함수
inline void InitializeWindowSize()
{
    GetWorkAreaSize(GWinSizeX, GWinSizeY);
}
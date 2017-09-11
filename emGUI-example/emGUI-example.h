#pragma once

#include "resource.h"
#include "GUI.h"


#include <spdlog/spdlog.h>

using namespace std;
#define DATA_ARRAY_SIZE 100
std::shared_ptr<spdlog::logger> logger;

typedef struct {
	LPCWSTR portName;
	bool exitFlag;
	int16_t data[DATA_ARRAY_SIZE];
	bool        bDataFilled;      ///< ������, ������������, ��� ������ ��������� (����� ���� �� � 1 ������)
	uint32_t    ulElemCount;      ///< ���������� �������� � ������� ������ ������
	uint32_t    ulCapacity;       ///< ������������ ���������� �������� � ������
	uint32_t    ulWritePos = 0;       ///< ������� ��������� ������ � ������

	std::shared_ptr<spdlog::logger> logger;
}serialThreadParams_t;
void handleData(int data, serialThreadParams_t * params);
void HandleAChar(char sym, serialThreadParams_t * params);
void HandleASuccessfulRead(char * lpBuf, WORD dwRead, serialThreadParams_t * params);

unsigned __stdcall SecondThreadFunc(void* pArguments);
// storage.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "monitor.h"
#include <windows.h>
Monitor<unsigned long> m;


void fill24Data() {
  unsigned hourOffset = 5;
  unsigned long begin = hourOffset * 60 * 60;
  for (unsigned long i = 0; i < 60 * 60 * 24; ++i)
  {
    unsigned long cur = begin + i;
    unsigned long hh = cur % (24 * 3600) / 3600;
    unsigned long mm = cur % 3600 / 60;
    if (hh == 1 && (mm >= 0 && mm <= 20))
      m.PutData(cur, 1023);
    else
      m.PutData(cur, 8);
  }
}
static int x = 0;
unsigned int filled = 0;
void printStats(unsigned size) {
  COORD coordScreen = { 0, 0 };
  SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coordScreen);
  unsigned long totaloffset = 0;
  for (unsigned long i = 0; i < 12; ++i) {
    unsigned long period = 0;
    unsigned long lu = 0;
    unsigned long k = m.NextAlignedData(totaloffset, size, period, lu);
    printf("%d : %d mA {%d}\n", totaloffset, k, ++x);
    totaloffset += period;
  }
  printf("------------------------------%d\n", filled);
}

int _tmain(int argc, _TCHAR* argv[])
{
  fill24Data();
  printStats(3600);
/*
  unsigned long begin = 455659;
  while (1) {
    m.PutData(begin + filled, 109);
    filled++;
    printStats(30);
    Sleep(100);
  }
*/

	return 0;
}


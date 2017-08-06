//  08/06/17 DDM
//  This utility was used to run ftdi.latency.exe as Admin,
//  before I figured out a more-direct solution.
//  The following page from Microsoft, from 2006, describes the technique:
//  http://blogs.msdn.com/b/vistacompatteam/archive/2006/09/25/771232.aspx
//  build: g++ -Wall -O2 -DUNICODE -D_UNICODE -s fix.ftdi.cpp -o fix.ftdi.exe
#include <windows.h>
#include <stdio.h>
#include <shellapi.h>
#include <tchar.h>

//**********************************************************************
#define  HTAB   9
//lint -esym(714, next_field)
//lint -esym(759, next_field)
//lint -esym(765, next_field)
char *next_field(char *q)
{
   while (*q != ' '  &&  *q != HTAB  &&  *q != 0)
      q++ ; //  walk past non-spaces
   while (*q == ' '  ||  *q == HTAB)
      q++ ; //  walk past all spaces
   return q;
}

//****************************************************************************
#define  MAX_UNICODE_LEN   1024
char *unicode2ascii(WCHAR *UnicodeStr)
{
   static char AsciiStr[MAX_UNICODE_LEN+1] ;
   WideCharToMultiByte(CP_ACP, 0, UnicodeStr, -1, AsciiStr, MAX_UNICODE_LEN, NULL, NULL);
   return AsciiStr ;
}

//*********************************************************
static void usage(void)
{
  puts("Usage: fix.ftdi [options]");
  puts("Options:");
  puts("-l = list all FTDI devices and current latencies");
  puts("-fN = set latency for all FTDI devices to N msec (1-30)");
}

//****************************************************************************
int main(void)
{
  SHELLEXECUTEINFO shExecInfo;                              
  LPTSTR argstr = GetCommandLineW();
  if (argstr == NULL) {
    usage();
    getchar();
    return 1 ;
  }
  argstr = _tcschr(argstr, '-');
  if (argstr == NULL) {
    usage();
    getchar();
    return 1 ;
  }

  shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);

  shExecInfo.fMask = NULL;
  shExecInfo.hwnd = NULL;
  shExecInfo.lpVerb = L"runas";  //  cause lpFile to be run elevated
  shExecInfo.lpFile = L"ftdi.latency.exe";
  shExecInfo.lpParameters = argstr;
  shExecInfo.lpDirectory = NULL;
  shExecInfo.nShow = SW_MAXIMIZE;
  shExecInfo.hInstApp = NULL;

  ShellExecuteEx(&shExecInfo);
  return 0;
}

/*
 * Windows implementation of fixing latency on an FTDI-based serial port, using registry.
 * https://raw.githubusercontent.com/projectgus/hairless-midiserial/master/src/
 * https://raw.githubusercontent.com/projectgus/hairless-midiserial/master/src/PortLatency_win32.cpp
 *SYSTEM\CurrentControlSet\Enum\FTDIBUS\-device id-\0000\Device Parameters
 * 
 * DDM Note - there is one -device id- entry for each FTDI device
 * that has been previously connected to the system.  Program needs to do further
 * work to find the device that one cares about, or it could just change all devices.
 */

#include <windows.h>
#include <stdio.h>

#define  LOOP_FOREVER   1

static bool isSwitched = false ;
#define  MAX_UNICODE_LEN   1024

//****************************************************************************
char *unicode2ascii(WCHAR *UnicodeStr)
{
   static char AsciiStr[MAX_UNICODE_LEN+1] ;
   WideCharToMultiByte(CP_ACP, 0, UnicodeStr, -1, AsciiStr, MAX_UNICODE_LEN, NULL, NULL);
   return AsciiStr ;
}

//****************************************************************************
// Return the windows registry entry for the FTDI device (open), or 0
//****************************************************************************
bool ListFTDIDeviceParameters(void)
{
    TCHAR paramPath[1024];
    HKEY key;
    const wchar_t *keypath = L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS";
    long r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keypath, 0, KEY_READ, &key);
    if(r) {
        printf("FTDI drivers don't seem to be installed. Not attempting any FTDI latency tricks.\n");
        return false;
    }

    // Look through available FTDI devices
    int index = 0;
    HKEY readOnlyKey = 0, paramKey = 0;
    while(LOOP_FOREVER) {
        if(paramKey) {
          RegCloseKey(paramKey);
          paramKey = 0;
        }
        DWORD subkey_sz = 140;
        wchar_t subkey[subkey_sz];
        r = RegEnumKeyEx(key, index++, subkey, &subkey_sz, 0, NULL, NULL, NULL);        
        if(r != ERROR_SUCCESS) {
            // if(r==ERROR_NO_MORE_ITEMS)
            //     printf("Port doesn't seem to be an FTDI device. No latency tricks will be attempted.\n");
            // else
            //     printf("Unexpected error while enumerating FTDI devices.\n");
            break;
        }
        // QString paramPath = QString("%1\\0000\\Device Parameters").arg(QString::fromWCharArray(subkey));
        wsprintf(paramPath, L"%s\\0000\\Device Parameters", subkey) ; 
        r = RegOpenKeyEx(key, paramPath, 0, KEY_READ, &readOnlyKey);
        if(r != ERROR_SUCCESS) {
          printf("%s: cannot open Device Params\n", unicode2ascii(subkey));
          continue;
        }
        //**********************************************************
        //  read PortName parameter
        DWORD regPortName_sz = 10;
        wchar_t regPortName[regPortName_sz];
        DWORD type;
        r = RegQueryValueEx(readOnlyKey, L"PortName", NULL, &type, (LPBYTE)&regPortName, &regPortName_sz);
        // if(!r && portName == QString::fromWCharArray(regPortName)) {
        if(r == ERROR_SUCCESS) {
           // r = RegOpenKeyEx(readOnlyKey, NULL, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
           // if(r) {
           //    printf("Port is an FTDI device, but we don't have administrator access to change the latency setting.\n");
           // } else {
              printf("%s: ", unicode2ascii(subkey));
              printf("%s, ", unicode2ascii(regPortName));
           // }
        } else {
          printf("%s: Er %u, ", unicode2ascii(subkey), (unsigned) GetLastError());
        }
        //**********************************************************
        //  read PortName parameter
        // DWORD regPortName_sz = 10;
        // wchar_t regPortName[regPortName_sz];
        DWORD latency ;
        DWORD latency_sz = sizeof(latency);
        r = RegQueryValueEx(readOnlyKey, L"LatencyTimer", NULL, &type, (LPBYTE)&latency, &latency_sz);
        // if(!r && portName == QString::fromWCharArray(regPortName)) {
        if(r == ERROR_SUCCESS) {
           r = RegOpenKeyEx(readOnlyKey, NULL, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
           if(r) {
              printf("Port is an FTDI device, but we don't have administrator access to change the latency setting.\n");
           } else {
              printf("latency=%u\n", (unsigned)latency);
           }
        } else {
          printf("no LatencyTimer: %s\n", unicode2ascii(paramPath));
        }
        
        RegCloseKey(readOnlyKey);
        // if(paramKey) break;
    }
    RegCloseKey(key);
    return true;
}

//****************************************************************************
// Return the windows registry entry for the FTDI device (open), or 0
//****************************************************************************
bool FixFTDILatencies(unsigned new_msec)
{
    TCHAR paramPath[1024];
    HKEY key;
    const wchar_t *keypath = L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS";
    long r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keypath, 0, KEY_READ, &key);
    if(r) {
        printf("FTDI drivers don't seem to be installed. Not attempting any FTDI latency tricks.\n");
        return false;
    }

    // Look through available FTDI devices
    int index = 0;
    HKEY readOnlyKey = 0, paramKey = 0;
    while(LOOP_FOREVER) {
        if(paramKey) {
          RegCloseKey(paramKey);
          paramKey = 0;
        }
        DWORD subkey_sz = 140;
        wchar_t subkey[subkey_sz];
        r = RegEnumKeyEx(key, index++, subkey, &subkey_sz, 0, NULL, NULL, NULL);        
        if(r != ERROR_SUCCESS) {
            // if(r==ERROR_NO_MORE_ITEMS)
            //     printf("Port doesn't seem to be an FTDI device. No latency tricks will be attempted.\n");
            // else
            //     printf("Unexpected error while enumerating FTDI devices.\n");
            break;
        }
        // QString paramPath = QString("%1\\0000\\Device Parameters").arg(QString::fromWCharArray(subkey));
        wsprintf(paramPath, L"%s\\0000\\Device Parameters", subkey) ; 
        r = RegOpenKeyEx(key, paramPath, 0, KEY_READ, &readOnlyKey);
        if(r != ERROR_SUCCESS) {
          printf("%s: cannot open Device Params\n", unicode2ascii(subkey));
          continue;
        }
        //**********************************************************
        //  read PortName parameter
        bool valid = false ;
        DWORD regPortName_sz = 10;
        wchar_t regPortName[regPortName_sz];
        DWORD type;
        r = RegQueryValueEx(readOnlyKey, L"PortName", NULL, &type, (LPBYTE)&regPortName, &regPortName_sz);
        // if(!r && portName == QString::fromWCharArray(regPortName)) {
        if(r == ERROR_SUCCESS) {
           // r = RegOpenKeyEx(readOnlyKey, NULL, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
           // if(r) {
           //    printf("Port is an FTDI device, but we don't have administrator access to change the latency setting.\n");
           // } else {
              printf("%s: ", unicode2ascii(subkey));
              printf("%s, ", unicode2ascii(regPortName));
              valid = true ;
           // }
        } else {
          printf("%s: Error %u\n", unicode2ascii(subkey), (unsigned) GetLastError());
        }
        //**********************************************************
        //  read PortName parameter
        // DWORD regPortName_sz = 10;
        // wchar_t regPortName[regPortName_sz];
        if (valid) {
          DWORD latency ;
          DWORD latency_sz = sizeof(latency);
          r = RegQueryValueEx(readOnlyKey, L"LatencyTimer", NULL, &type, (LPBYTE)&latency, &latency_sz);
          // if(!r && portName == QString::fromWCharArray(regPortName)) {
          if(r == ERROR_SUCCESS) {
             r = RegOpenKeyEx(readOnlyKey, NULL, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
             if(r) {
                printf("Port is an FTDI device, but we don't have administrator access to change the latency setting.\n");
             } else {
                DWORD new_latency = new_msec;
                r = RegSetValueEx(paramKey, L"LatencyTimer", 0, REG_DWORD, (LPBYTE)&new_latency, sizeof(new_latency));
                if(r == ERROR_SUCCESS) {
                  printf("new latency=%u\n", (unsigned)new_latency);
                } else {
                  printf("old latency=%u\n", (unsigned)latency);
                }
             }
          } else {
            printf("no LatencyTimer: %s\n", unicode2ascii(paramPath));
          }
        }
        
        RegCloseKey(readOnlyKey);
        // if(paramKey) break;
    }
    RegCloseKey(key);
    return true;
}

// Return the windows registry entry for the FTDI device (open), or 0
HKEY openFTDIDeviceParameters(void)
{
    TCHAR paramPath[1024];
    HKEY key;
    const wchar_t *keypath = L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS";
    long r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keypath, 0, KEY_READ, &key);
    if(r) {
        printf("FTDI drivers don't seem to be installed. Not attempting any FTDI latency tricks.\n");
        return 0;
    }

    // Look through available FTDI devices
    int index = 0;
    HKEY readOnlyKey = 0, paramKey = 0;
    while(1) {
        if(paramKey) {
          RegCloseKey(paramKey);
          paramKey = 0;
        }
        DWORD subkey_sz = 140;
        wchar_t subkey[subkey_sz];
        r = RegEnumKeyEx(key, index++, subkey, &subkey_sz, 0, NULL, NULL, NULL);        
        if(r) {
            if(r==ERROR_NO_MORE_ITEMS)
                printf("Port doesn't seem to be an FTDI device. No latency tricks will be attempted.\n");
            else
                printf("Unexpected error while enumerating FTDI devices.\n");
            break;
        }
        // QString paramPath = QString("%1\\0000\\Device Parameters").arg(QString::fromWCharArray(subkey));
        wsprintf(paramPath, L"%s\\0000\\Device Parameters", subkey) ; 
        r = RegOpenKeyEx(key, paramPath, 0, KEY_READ, &readOnlyKey);
        if(r) {
            continue;
        }
        DWORD regPortName_sz = 10;
        wchar_t regPortName[regPortName_sz];
        DWORD type;
        r = RegQueryValueEx(readOnlyKey, L"PortName", NULL, &type, (LPBYTE)&regPortName, &regPortName_sz);
        // if(!r && portName == QString::fromWCharArray(regPortName)) {
        if(r == ERROR_SUCCESS) {
           r = RegOpenKeyEx(readOnlyKey, NULL, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
           if(r) {
              printf("Port is an FTDI device, but we don't have administrator access to change the latency setting.\n");
           } else {
              printf("found %s\n", unicode2ascii(regPortName));
           }
        } else {
          printf("no Portname: %s\n", unicode2ascii(paramPath));
        }
        RegCloseKey(readOnlyKey);
        if(paramKey) break;
    }
    RegCloseKey(key);
    return paramKey;
}

/*
 * Read a DWORD value from one registry value under paramKey and copy it to another value under paramKey
 */
bool switchLatency(HKEY paramKey, const wchar_t *readFromKey, const wchar_t *writeToKey, bool checkValue)
{
    DWORD latency, type, lsize = sizeof(DWORD);
    int r = RegQueryValueEx(paramKey, readFromKey, NULL, &type, (LPBYTE)&latency, &lsize);
    if(r) {
        // printf("Failed to read the %1 value for FTDI device %2").arg(QString::fromWCharArray(readFromKey)).arg(portName));
        // printf("Failed to read the value for FTDI device %s\n", portName);
        printf("Failed to read the value for FTDI device\n");
        return false;
    }
    if(type != REG_DWORD) {
        // emit errorMessage(QString("%1 of %2 has unexpected type %3").arg(QString::fromWCharArray(readFromKey)).arg(portName).arg(type));
        printf("unexpected type\n");
        return false;
    }
    if(checkValue && latency == 1) {
        // printf("LatencyTimer for %s has already been turned down to %u\n", portName, latency);
        printf("LatencyTimer for device has already been turned down to %u\n", (unsigned)latency);
        return false;
    }
    r = RegSetValueEx(paramKey, writeToKey, 0, REG_DWORD, (LPBYTE)&latency, sizeof(latency));
    if(r) {
        // emit errorMessage(QString("Failed to write key %1. Missing privileges?").arg(QString::fromWCharArray(writeToKey)));
        printf("Failed to write key. Missing privileges?\n");
        return false;
    }
    return true;
}

void fixLatency(void)
{
    if(isSwitched) return;
    HKEY paramKey = openFTDIDeviceParameters();
    if(!paramKey) return;

    // if we get here, paramKey is the subkey with Device Parameters for our port

    if(switchLatency(paramKey, L"LatencyTimer", L"OldLatencyTimer", true))
    {
        DWORD latency = 1;
        int r = RegSetValueEx(paramKey, L"LatencyTimer", 0, REG_DWORD, (LPBYTE)&latency, sizeof(latency));
        isSwitched = (r == 0);                                    
    }
    RegCloseKey(paramKey);
}

void resetLatency(void)
{
    if(!isSwitched) return;
    HKEY paramKey = openFTDIDeviceParameters();
    if(!paramKey) return;

    // if we get here, paramKey is the subkey with Device Parameters for our port

    isSwitched = !switchLatency(paramKey, L"OldLatencyTimer", L"LatencyTimer", false);
    if(!isSwitched)
    {
        int r = RegDeleteValue(paramKey, L"OldLatencyTimer");
        if(r) {
            printf("Failed to delete OldLatencyTimer key after switching back.\n");
            printf("You can probably ignore this message.\n");
        }
    }
    RegCloseKey(paramKey);
}

//*********************************************************
static void usage(void)
{
  puts("Usage: ftdi.latency [options]");
  puts("Options:");
  puts("-l = list all FTDI devices and current latencies");
  puts("-fN = set latency for all FTDI devices to N msec (1-30)");
}

//*********************************************************
int main(int argc, char **argv)
{
  unsigned char result = 0 ;
  int idx ;
  bool list_mode = false ;
  bool fix_mode = false ;
  unsigned fix_msec = 0 ;
  for (idx=1; idx<argc; idx++) {
    char *p = argv[idx];
    if (*p == '-') {
      p++ ;
      switch (*p) {
      case 'l':  
        list_mode = true ;  
        break ;
        
      case 'f':
        p++ ;
        fix_mode = true ;
        fix_msec = (unsigned) atoi(p);
        if (fix_msec == 0  ||  fix_msec > 30) {
          usage();
          result = 1 ;
          goto wait_exit ;
        }
        break;
        
      default:
        usage();
        result = 1 ;
        goto wait_exit ;
      }
    } else {
      usage();
      result = 1 ;
      goto wait_exit ;
    }
  }
  
  //  verify valid arguments
  if (!list_mode  &&  !fix_mode) {
    usage();
    result = 1 ;
    goto wait_exit ;
  }
  
  if (fix_mode) {
    FixFTDILatencies(fix_msec) ;
    list_mode = true ;
  }
  
  if (list_mode) {
    puts("");
    puts("Listing FTDI devices:");
    ListFTDIDeviceParameters() ;
  }

wait_exit:
  //  These are used to keep window open, 
  //  when running via RunAs ... 
  puts("Press Enter key to exit...");
  getchar();
  return result ;
}

#include <stdio.h>
#include <stdarg.h>
#include "zconf.h"
int main()
{
#ifndef STDC
  choke me
#endif
  return 0;
}

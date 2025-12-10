#include <stdio.h>

void help()
{
writes("tinydns - small proxy DNS server.\n\
  -c config       load config instead of tinydns.conf / /etc/tinydns.conf\n\
  -d              Run as daemon.\n\
  -D              show debug info\n\
  -v              Show program version.\n\
  -h              Print this help.\n");
exit(0);
}

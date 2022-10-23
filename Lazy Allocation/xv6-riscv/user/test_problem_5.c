#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/processinfo.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  struct processinfo p;
  get_process_info(&p); 
  printf("Process ID -> %d\n", p.pid);
  printf("Process Name -> %s\n", p.name);
  printf("Memory Size -> %d Bytes\n", p.sz/8);
  exit(0);	
}
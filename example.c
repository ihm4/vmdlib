#include <stdio.h>
#include <stdlib.h>
#include "vmd.h"

int main(int argc, char* argv[]) {
  VMDFile* vf;
  VMD_ERROR = VMDLIB_E_INIT;

  // check args
  if ( argc == 1 ){
    printf("No arguments given.\n");
    exit(EXIT_FAILURE);
  }

  printf("Open file : %s\n", argv[1]);

  vf = VMDLoadFromFile(argv[1]);
  if ( vf == NULL ) {
    fprintf(stderr, "failed to open file\n");
    exit(EXIT_FAILURE);
  }
  VMDSortAllFrames(vf);
  VMDDisplayData(vf);
  VMDWriteToFile(vf, "output.vmd");
  VMDReleaseVMDFile(vf);

  return 0;
}

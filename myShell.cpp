#include <cstdio>
#include <cstdlib>

#include "command.h"
#include "shell.h"

using std::string;
using std::vector;

int main(void) {
  // Construct a Shell object.
  Shell shell;

  // Start the shell program.
  shell.execute();

  return EXIT_SUCCESS;
}

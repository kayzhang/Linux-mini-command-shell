#include "command.h"

#include "shell.h"

/**
 * Command::getArgv
 *
 * This method will convert the @argv field from vector<string>
 * to vector<char *>, and store the result in @c_argv.
 *
 * Note: @c_argv will be terminated with an extra NULL pointer.
 */
void Command::getArgv(vector<char *> & c_argv) {
  // Convert every string in @argv to a char * in @c_argv.
  for (int i = 0; i < argc; i++) {
    c_argv.push_back(&argv[i][0]);
  }
  // Add an extra NULL pointer to be consistent with execve().
  c_argv.push_back(NULL);
}

/**
 * Command::execute
 *
 * This method executes the command by creating a child process.
 */
void Command::execute() {
  // fork a new process
  pid_t child_pid;
  int child_status;

  child_pid = fork();
  if (child_pid == -1) {  // fork failed
    std::cerr << "fork failed." << std::endl;
    exit(EXIT_FAILURE);
  }

  if (child_pid == 0) {  // Code executed by child
    // convert @argv to vector<char *>
    vector<char *> c_argv;
    getArgv(c_argv);
    // get the environment variables
    extern char ** environ;
    // execute the command
    execve(c_argv[0], &c_argv[0], environ);

    // execve() returns only on error
    std::cerr << "execve failed." << std::endl;
    exit(EXIT_FAILURE);
  }
  else {  // Code executed by parent.
    do {
      // Wait for the child to terminate
      pid_t w = waitpid(child_pid, &child_status, 0);
      if (w == -1) {  // waitpid failed
        std::cerr << "waitpid failed." << std::endl;
        exit(EXIT_FAILURE);
      }

      if (WIFEXITED(child_status)) {  // program exited
        std::cout << "Program exited with status " << WEXITSTATUS(child_status) << std::endl;
      }
      else if (WIFSIGNALED(child_status)) {  // program was killed
        std::cout << "Program was killed by signal " << WTERMSIG(child_status) << std::endl;
      }
    } while (!WIFEXITED(child_status) && !WIFSIGNALED(child_status));
  }
}

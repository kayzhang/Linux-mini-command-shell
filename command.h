#ifndef __SIMPLECMD_H__
#define __SIMPLECMD_H__
#include <dirent.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::string;
using std::vector;

class Shell;

/**
 * The Command class describes a shell command with arguments list.
 */

class Command
{
 private:
  int argc;             // Number of arguments
  vector<string> argv;  // Vector of arguments

  // getArgv()  converts @argv from vector<string> to vector<char *>
  void getArgv(vector<char *> & c_argv);

 public:
  // Default constructor
  Command() : argc(0), argv() {}

  // Overloaded constructor which takes a vector<string>
  Command(const vector<string> & currArgv) : argc(currArgv.size()), argv(currArgv) {}

  // execute() executes this command
  void execute();
};

#endif

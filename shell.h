#ifndef __SHELL_H__
#define __SHELL_H__
#include <map>

#include "command.h"
using std::string;
using std::vector;

/**
 * The Shell class describes a shell program, which stores the shell
 * environment variables and the current command.
 */
class Shell
{
 private:
  string currCmd;                   // Current command string
  int currArgc;                     // Number of current arguments
  vector<string> currArgv;          // Vector of current arguments
  std::map<string, string> envMap;  // Environment variables map

  // Print the shell prompt
  void printPrompt();

  // Read a command line from std::cin
  bool getCmd();

  // Expand every variable in &cmd from index strat to its value
  void expandVariables(string & cmd, size_t start);

  // A helper method to escape the back slash in @str
  void escapeBackSlash(string & str);

  // Parse @currCmd and initialize @currArgv
  void parseArgv();

  // Cheak if @cmdName is a valid environment variable name
  bool isBuiltIn(const string & cmdName);

  // Check if the command currArgv[0] exists.
  // If it exists, update currArgv[0] to the command's full path.
  bool updateCmdName();

  // Check if the file @fileName exists in @ECE551PATH.
  // If it exists, store its full path in @fullPath
  bool findFullPath(string & fullPath, const string fileName);

  // Check if file @fileName exists in the directory @dirName
  bool isFileExists(const string & dirName, const string & fileName);

  // Erase the contents of the fields except for @envMap
  void clear();

  // Execute the currend command if it's a built-in command
  void exeBuiltIn();

  // Built-in command to change the current working directory
  void myCd();

  // Check if @c is a valid char for an environment variable name
  bool isVarNameChar(const char c);

  // Check if @name is a valid environment variable name
  bool isVarName(const string & name);

  // Built-in command to set an environment variable
  void mySet();

  // A helper method to export environment variable &key
  void exportVar(const string & key);

  // Built-in command to export an environment variable
  void myExport();

  // Convert a string @str to a double number
  double str2d(const string & str);

  // Built-in command to increment an environment variable
  void myInc();

 public:
  // Default constructor
  Shell();

  // Start executing the shell program.
  void execute();
};

#endif

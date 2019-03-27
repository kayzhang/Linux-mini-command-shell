#include "shell.h"

#include <sstream>

/**
 * Shell::Shell - Default constructor
 *
 * This constructor pushes all the shell environment variables
 * in @environ into @envMap, and add an extra variable @ECE551PATH,
 * whose value is initialized to be the same with @PATH. 
 */
Shell::Shell() : currCmd(), currArgc(0), currArgv(), envMap() {
  extern char ** environ;
  char ** envPtr = environ;  // pointer to "key=value" pair

  while (*envPtr) {  // Walk through all pairs
    // Use stringstream to split the pair by delimiter '='
    std::stringstream ss;
    ss << *envPtr;
    string key, value;
    std::getline(ss, key, '=');
    std::getline(ss, value);

    // Push (key, value) into the environment map @envMap
    envMap[key] = value;
    envPtr++;
  }

  // Add an extra variable @ECE551PATH
  envMap[string("ECE551PATH")] = envMap["PATH"];
}

/**
 * Shell::printPrompt
 *
 * This method prints a shell prompt in the following format:
 * myShell:~/ece551/mp_miniproject $
 */
void Shell::printPrompt() {
  char * cwd = getenv("PWD");     // current working directory
  char * home = getenv("HOME");   // home directory
  size_t homeLen = strlen(home);  // length of home path
  // If @cwd starts with @home, replace it with "~"
  if (strncmp(cwd, home, homeLen) == 0) {
    std::cout << "myShell:~" << &cwd[homeLen] << " $";
  }
  else {
    std::cout << "myShell:" << getenv("PWD") << " $";
  }
}

/** Shell::getCmd
 *
 * This method read a new line from the std::cin itto string
 * @currCmd. If EOF is read, return false. Otherwise, return true.
 */
bool Shell::getCmd() {
  // read a new line from std::cin into string @currCmd
  if (std::getline(std::cin, currCmd)) {  // not EOF
    return true;
  }
  else if (std::cin.eof()) {  // EOF
    return false;
  }
  else {  // Otherwise, report the error and exit
    std::cerr << "Error reading." << std::endl;
    exit(EXIT_FAILURE);
  }
}

/** Shell::expandVariables
 *
 * This method takes a string @cmd, and an index @start, expands every
 * environment variable in the substring of @cmd which starts from
 * index @start to its value.
 * 
 * Note: if '$' is followed by a non-variable char, it's not expanded.
 * Note: if the variable doesn't exist, expand it to empty string.
 * Example: if a=3, then "echo $ $b$a" expands to "echo $ 3".
 */
void Shell::expandVariables(string & cmd, size_t start) {
  // Base case: the substring from @start does not contain a '$'.
  size_t pos = cmd.find_first_of('$', start);
  if (pos != string::npos) {  // find a '$'
    start = pos + 1;
    // If '$' is followed by a non-variable char, it's not expanded.
    if (!isVarNameChar(cmd[start])) {
      expandVariables(cmd, start);
    }
    else {
      string key, value;  // value is initialized to an empty string
      // Get the variable name
      while (start < cmd.size() && isVarNameChar(cmd[start])) {
        key += cmd[start];
        start++;
      }

      std::map<string, string>::iterator it = envMap.find(key);
      // If key exists, assign it to value. Otherwise, value is empty.
      if (it != envMap.end()) {
        value = it->second;
      }

      // Expand the variable, and recurse on the remaining string.
      cmd.replace(pos, start - pos, value);
      expandVariables(cmd, pos + value.size());
    }
  }
}

/** Shell::excapeBackSlash
 * 
 * This method is a helper methor to excape the back slash in &str.
 * 1. Replace continuous whitespaces which are not following a '\' with a '\' 
 * 2. Delete a single '\' which is not followed by a space ' '.  
 * 3. Replace a '\ ' with a ' '
 * 
 * Note: after this work, the string can be splited with delimiter '\'
 * Note: "a\b" will be converted to "ab".
 */
void Shell::escapeBackSlash(string & str) {
  size_t i = 0;
  // We might check two letters at one loop, so i < str.size() - 1
  while (i < str.size() - 1) {
    if (std::isspace(str[i])) {  // replace continuous whitespaces with a '\'
      str[i] = '\\';
      i++;
      while (std::isspace(str[i])) {
        str.erase(i, 1);
      }
    }
    else if (str[i] == '\\') {
      str.erase(i, 1);      // delete a '\'
      if (str[i] == ' ') {  // if it's a '\ ', then skip the ' '
        i++;
      }
    }
    else {  // skip a char which is not ' ' or '\'
      i++;
    }
  }
  // If the last is a not processed ' ', delete it (it's delimiter).
  if (i == str.size() - 1 && str[i] == ' ') {
    str.erase(i);
  }
}

/** Shell::parseArgv
 *
 * This method parses the command string @currCmd, and stores
 * the arguments list in the vector<string> @currArgv.
 * The arguments number @currArgv is also updated.
 *
 * Note: the string @currCmd remains unchanged.
 */
void Shell::parseArgv() {
  // Make a copy of @currCmd to keep it unchanged
  string currCmdCopy(currCmd);

  // Pre-process the string to escape the back slash
  escapeBackSlash(currCmdCopy);

  // Use stringstream and delimeter '\' to split the arguments
  std::stringstream ss(currCmdCopy);
  string arg;
  while (std::getline(ss, arg, '\\')) {
    currArgv.push_back(arg);
    currArgc++;
  }
}

/** Shell::isBuiltIn
 *
 * This method checks if the @cmdName is a built-in command name.
 * Currently, built-in commands are: cd, set, export, inc.
 */
bool Shell::isBuiltIn(const string & cmdName) {
  return !(cmdName.compare("cd") && cmdName.compare("set") && cmdName.compare("export") &&
           cmdName.compare("inc"));
}

/** Shell::updateCmdName
 *
 * This method checks if the command currArgv[0] exists.
 * If it exists, update currArgv[0] to the command's full path.
 */
bool Shell::updateCmdName() {
  // If no '/' in the path name, replace it with it's full path
  if (currArgv[0].find('/') == string::npos) {
    return findFullPath(currArgv[0], currArgv[0]);
  }
  // Otherwise, check if the file exists.
  else if (FILE * f = fopen(currArgv[0].c_str(), "r")) {
    fclose(f);
    return true;
  }

  return false;
}

/** Shell::findFullPath
 *
 * This method checks if the file @fileName exists in @ECE551PATH.
 * If it exists, store its full path in @fullPATH.
 */
bool Shell::findFullPath(string & fullPath, const string fileName) {
  // Use stringstream and delimiter ':' to split the path
  std::stringstream ss(envMap["ECE551PATH"]);
  string dirName;
  while (std::getline(ss, dirName, ':')) {
    if (isFileExists(dirName, fileName)) {
      fullPath = dirName;
      // If @dirName has not an ending '/', add one.
      if (dirName[dirName.size() - 1] != '/') {
        fullPath += "/";
      }
      fullPath += fileName;
      return true;
    }
  }
  return false;
}

/** Shell::isFileExists
 *
 * This method checks if the file @fileName exists in the @dirName.
 */
bool Shell::isFileExists(const string & dirName, const string & fileName) {
  DIR * dr = opendir(dirName.c_str());  // open directory
  // opendir() returns NULL if couldn't open directory
  if (dr != NULL) {
    struct dirent * de;  // pointer for directory entry
    while ((de = readdir(dr)) != NULL) {
      // Check if @de->name is the same with @fileName
      if (strcmp(de->d_name, fileName.c_str()) == 0) {
        closedir(dr);
        return true;
      }
    }
    closedir(dr);
  }
  return false;
}

/** Shell::clear
 *
 * This method clear the contents of the fields except for @envMap,
 * which should keep its value in the whole shell program.
 */
void Shell::clear() {
  currCmd.clear();
  currArgc = 0;
  currArgv.clear();
}

/** Shell::exeBuiltin
 *
 * This method is called when the command is a built-in command.
 */
void Shell::exeBuiltIn() {
  if (currArgv[0].compare("cd") == 0) {
    myCd();
  }
  else if (currArgv[0].compare("set") == 0) {
    mySet();
  }
  else if (currArgv[0].compare("export") == 0) {
    myExport();
  }
  else if (currArgv[0].compare("inc") == 0) {
    myInc();
  }
  return;
}

/** Shell::myCd
 *
 * This method implements the built-in "cd" command, which changes
 * the current working directory.
 */
void Shell::myCd() {
  // If the command contains only one argument "cd", go to home.
  if (currArgc == 1) {
    if (chdir(getenv("HOME")) == -1) {
      std::cerr << "cd: change directory failed" << std::endl;
    }
  }
  else if (currArgc == 2) {  // Go to the required directory
    if (chdir(currArgv[1].c_str()) == -1) {
      std::cerr << "cd: change directory failed" << std::endl;
    }
  }
  else {  // too many arguments
    std::cerr << "cd: too many arguments" << std::endl;
  }

  // Change the "PWD" variable in the @envMap and export it
  char cwd[256];
  if (!getcwd(cwd, 256)) {
    std::cerr << "cd: after chdir, failed to getcwd" << std::endl;
  }
  envMap["PWD"] = string(cwd);
  exportVar("PWD");
}

/** Shell::isVarnamechar
 *
 * This method checks if @c is a valid char for an environment
 * variable name, which should be either letters, numbers, or '_'.
 */
bool Shell::isVarNameChar(const char c) {
  return std::isdigit(c) || std::isalpha(c) || c == '_';
}

/** Shell::isVarName
 *
 * This method checks if @name is a valid environment variable name,
 * which should only contains letters and '_'.
 */
bool Shell::isVarName(const string & name) {
  if (name.empty()) {  // empty string is not valid
    return false;
  }
  for (string::const_iterator it = name.begin(); it != name.end(); ++it) {
    if (!isVarNameChar(*it)) {
      return false;
    }
  }
  return true;
}

/** Shell::mySet
 *
 * This method implements the built-in "set" command, which push the
 * variable in the environment variable map @envMap.
 * Note: set does not export the variable.
 */
void Shell::mySet() {
  // Use stringstream to split the @currCmd
  std::stringstream ss(currCmd);
  string cmdName, key, value;
  ss >> cmdName >> key;
  if (isVarName(key)) {  // the key exists
    // The rest part (except the starting whitespaces) is the value.
    std::getline(ss, value);
    // delete the starting whitespaces in value.
    value.erase(0, value.find_first_not_of(" \f\n\r\t\v"));
    envMap[key] = value;
  }
  else {  // the key does not exists
    std::cerr << "set: invalid variable name" << std::endl;
  }
}

/** Shell::exportVar
 *
 * This method takes a variable name @key, and export it.
 */
void Shell::exportVar(const string & key) {
  if (setenv(key.c_str(), envMap[key].c_str(), 1) == -1) {
    std::cerr << "export: variable " << key << " exists but export failed" << std::endl;
  }
}

/** Shell::myExport
 *
 * This method implements the built-in "export" command, which
 * exports the variable required by the command.
 */
void Shell::myExport() {
  // Use stringstream to split the command.
  std::stringstream ss(currCmd);
  // Use @extraArg to check if there is too many arguments.
  string cmdName, key, extraArg;
  ss >> cmdName >> key >> extraArg;
  if (isVarName(key) && extraArg.empty()) {  // the command is valid
    std::map<string, string>::iterator it = envMap.find(key);
    if (it != envMap.end()) {  // the key exists
      exportVar(key);
    }
    else {  // the key does not exist
      std::cout << "export: variable " << key << " doesn't exist" << std::endl;
    }
  }
  else {
    std::cerr << "export: invalid variable name or invalid argument number" << std::endl;
  }
}

/** Shell::str2d
 *
 * This method converts a string @str to a double number.
 * Note: If the str is not a valid double number, evaluate it to 0.
 */
double Shell::str2d(const string & str) {
  double ans = 0;
  // If the @str is empty or has starting whitespaces, return 0.
  if (!str.empty() && !std::isspace(str[0])) {
    char * pEnd;
    ans = strtod(str.c_str(), &pEnd);
    // If the string has other chars after the number, return 0.
    if (*pEnd != '\0') {
      ans = 0;
    }
  }
  return ans;
}

/** Shell::myInc
 *
 * This method implements the built-in "inc" command, which
 * increment the required variable. If its not a valid double
 * number, evaluate it to 0, and increment it to 1.
 */
void Shell::myInc() {
  // Use stringstream to split the command.
  std::stringstream ss(currCmd);
  // Use @extraArg to check if there is too many arguments.
  string cmdName, key, extraArg;
  ss >> cmdName >> key >> extraArg;
  if (isVarName(key) && extraArg.empty()) {  // the command is valid
    std::map<string, string>::iterator it = envMap.find(key);
    if (it != envMap.end()) {  // the key exists
      double val = str2d(it->second);
      std::stringstream d2s;
      d2s << (val + 1);
      it->second = d2s.str();
    }
    else {  // the key does not exist
      envMap[key] = string("1");
    }
  }
  else {
    std::cerr << "inc: invalid variable name or invalid argument number" << std::endl;
  }
}

/** Shell::execute
 *
 * This method starts the shell program.
 */
void Shell::execute() {
  while (true) {  // Repeatedly execute the command
    // print a prompt "myShell$"
    printPrompt();

    // If the input is EOF or "exit", terminate the shell program.
    if (!getCmd() || !currCmd.compare("exit")) {
      return;
    }

    if (!currCmd.empty()) {  // If just hit ENTER, do nothing.
      // Evaluate the environment variables
      expandVariables(currCmd, 0);

      // Parse the command to arguments list
      parseArgv();

      if (isBuiltIn(currArgv[0])) {  // built-in command name
        exeBuiltIn();
      }
      else if (updateCmdName()) {  // valid built-out command name
        Command cmd(currArgv);
        cmd.execute();
      }
      else {  // invalid command name
        std::cerr << "Command " << currArgv[0] << " not found" << std::endl;
      }
    }

    // Clear the contents of related fields to store the next command.
    clear();
  }
}

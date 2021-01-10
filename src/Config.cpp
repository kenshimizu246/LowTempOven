
#include <iostream>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filewritestream.h>

#include "Config.hpp"

using namespace std;
using namespace rapidjson;

namespace tamageta {


Config::Config() {
  initialized = false;
}

Config::~Config() {
}

void Config::load(const char* filename){
  ifstream ifs(filename);
  string ss, str;
  Document doc;

  const char *cpidfile = "pidfile";
  const char *clogdir = "logdir";
  const char *cappdir = "appdir";
  const char *cport = "port";

  const char *cDaemon = "daemon";

  if (ifs.fail()) {
    throw runtime_error("file read failed!");
  }

  while (getline(ifs, ss)) {
    cout << "[" << ss << "]" << endl;
    str += ss;
  }
  cout << "str[" << str << "]" << endl;

  if (doc.Parse<0>(str.c_str()).HasParseError()) {
    throw runtime_error("configuration parse error!");
  }

  // pidfile
  if (!doc.HasMember(cpidfile)) {
    throw runtime_error("pidfile is not configured!");
  }
  if (!doc[cpidfile].IsString()) {
    throw runtime_error("pidfile must be string!");
  }
  pidfile = doc[cpidfile].GetString();

  // appdir
  if (!doc.HasMember(cappdir)) {
    throw runtime_error("appdir is not configured!");
  }
  if (!doc[cappdir].IsString()) {
    throw runtime_error("appdir must be string!");
  }
  appdir = doc[cappdir].GetString();

  // logdir
  if (!doc.HasMember(clogdir)) {
    throw runtime_error("logdir is not configured!");
  }
  if (!doc[clogdir].IsString()) {
    throw runtime_error("logdir must be string!");
  }
  logdir = doc[clogdir].GetString();

  // port
  if (!doc.HasMember(cport)) {
    throw runtime_error("port is not configured!");
  }
  if (!doc[cport].IsInt()) {
    throw runtime_error("port must be integer!");
  }
  port = doc[cport].GetInt();

  // daemon
  daemon = false;
  if (!doc.HasMember(cDaemon)) {
    throw runtime_error("daemon is not configured!");
  }
  if (!doc[cDaemon].IsBool()) {
    throw runtime_error("daemon must be bool!");
  }
  daemon = doc[cDaemon].GetBool();
  if (!doc.HasMember(cDaemon)) {
    throw runtime_error("daemon is not configured!");
  }

  initialized = true;
  cout << "loaded!" << endl;
}

string Config::getPidFile() {
  if (!initialized) {
    throw runtime_error("not initialized!");
  }
  return pidfile;
}

string Config::getAppDir() {
  if (!initialized) {
    throw runtime_error("not initialized!");
  }
  return appdir;
}

string Config::getLogDir() {
  if (!initialized) {
    throw runtime_error("not initialized!");
  }
  return logdir;
}

unsigned int Config::getPort() {
  if (!initialized) {
    throw runtime_error("not initialized!");
  }
  return port;
}

bool Config::isDaemon() {
  if (!initialized) {
    throw runtime_error("not initialized!");
  }
  return daemon;
}

bool Config::isInitialized() {
  return initialized;
}

}


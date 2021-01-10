
#include <string>
#include <exception>
#include <stdexcept>

using namespace std;

#ifndef _Config_hpp
#define _Config_hpp

namespace tamageta {

class Config {
  private:
    Config();
    ~Config();
    bool initialized;
    string appdir;
    string logdir;
    string pidfile;
    unsigned int port = 9089;
    bool daemon;

  public:
    void load(const char* filename);
    string getAppDir();
    string getLogDir();
    string getPidFile();
    unsigned int getPort();
    bool isInitialized();

    bool isDaemon();

    static Config& getInstance() {
      static Config config;
      return config;
    }
};

}

#endif /*_Config_hpp*/

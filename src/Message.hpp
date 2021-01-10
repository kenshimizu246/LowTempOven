
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filewritestream.h>

#include "Command.hpp"
#include "worker/max31855_worker.hpp"

using namespace rapidjson;

#ifndef _message_handler_hpp
#define _message_handler_hpp

class message_handler_error : public std::runtime_error {
public:
  message_handler_error(const char *_message)
      : runtime_error(_message) {}
};

class message_handler{
public:
  static void toJSON(max31855_event& event, std::string& s);
  static void toJSON(command_event& event, std::string& s);

  static std::shared_ptr<command> toCommand(command_factory& factory, std::string& s);

  static std::shared_ptr<command> toCommand(std::string& s);

  static const char *kHeader;
  static const char *kBody;

  static const std::string kAUTO;

private:
  void toString(std::time_t t, char* b);
};

#endif


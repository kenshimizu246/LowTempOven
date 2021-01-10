
#include <iostream>
#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filewritestream.h>

//#include <time.h>

#include <ctime>
#include <iostream>
#include <locale>
#include <exception>
#include <typeinfo>


#include "worker/max31855_worker.hpp"
#include "Message.hpp"
#include "Command.hpp"

using namespace rapidjson;

const char* message_handler::kHeader = "header";
const char* message_handler::kBody = "body";

void message_handler::toString(std::time_t t, char* b){
  if(!std::strftime(b, sizeof(b), "%FT%T%z", std::localtime(&t))){
    std::cerr << "time error!" << std::endl;
  }
}

/*************************************
{ "header":{
    "message-type":"event",
    "message-version":1,
    "session-id":"abc123",
    "source": "max31855"
    "source-id": "0"
  },
  "body":{
    "temperature": 123,
    "status": "SUCCESS",
    "timestamp": "",
  }
}
*************************************/
void message_handler::toJSON(max31855_event& event, std::string& s){
  Document d;
  d.SetObject();

  // create an allocator
  Document::AllocatorType& allocator = d.GetAllocator();

  Value header(kObjectType);
  header.AddMember("message-type", "event", allocator);
  header.AddMember("source", "max31855", allocator);
  header.AddMember("source-id", event.get_id(), allocator);
  d.AddMember("header", header, allocator);

  char buff[100];
  int len = strftime(buff, sizeof(buff), "%D %T", event.get_gmtime());

  Value body(kObjectType);
  body.AddMember("temperature", Value().SetFloat(event.get_temperature()), allocator);
  body.AddMember("status", event.get_status(), allocator);
  body.AddMember("timestamp", Value().SetString(buff, len, allocator), allocator);
  d.AddMember("body", body, allocator);

  StringBuffer sb;
  Writer<StringBuffer> writer(sb);
  d.Accept(writer);

  s.append(sb.GetString(), sb.GetLength());
}

/*************************************
{ "header":{
    "message-type":"event",
    "message-version":1,
    "session-id":"abc123",
    "source": "max31855"
    "source-id": "0"
  },
  "body":{
    "temperature": 123,
    "status": "SUCCESS",
    "timestamp": "",
  }
}
*************************************/
void message_handler::toJSON(command_event& event, std::string& s){
  Document d;
  d.SetObject();

  // create an allocator
  Document::AllocatorType& allocator = d.GetAllocator();

  Value header(kObjectType);
  header.AddMember("message-type", "event", allocator);
  header.AddMember("source", "controller", allocator);
  d.AddMember("header", header, allocator);

  char buff[100];
  int len = strftime(buff, sizeof(buff), "%D %T", event.get_gmtime());

  Value body(kObjectType);
  //body.AddMember("temperature", Value().SetFloat(event.get_temperature()), allocator);
  //body.AddMember("status", event.get_status(), allocator);
  //body.AddMember("timestamp", Value().SetString(buff, len, allocator), allocator);
  d.AddMember("body", body, allocator);

  StringBuffer sb;
  Writer<StringBuffer> writer(sb);
  d.Accept(writer);

  s.append(sb.GetString(), sb.GetLength());
}

/*************************************
{ "header":{
    "message-type":"command",
    "message-version":1,
    "session-id":"abc123",
    "target-id": "0",
    "seq-id":0
  },
  "body":{ "command":"set_power", "value": 100}
}
*************************************/
std::shared_ptr<command> message_handler::toCommand(command_factory& factory, std::string& s){
  Document d;
  std::cout << "toCommand: " << s << std::endl;

  if (d.Parse<0>(s.c_str()).HasParseError()){
    std::stringstream ss;
    ss << "Parse Error : " << s;
    throw message_handler_error(ss.str().c_str());
  }

  if (!d.HasMember(kHeader)){
    std::stringstream ss;
    ss << "No Header Error : " << s;
    throw message_handler_error(ss.str().c_str());
  }

  if (!d.HasMember(kBody)){
    std::stringstream ss;
    ss << "No body tag Error : " << s;
    throw message_handler_error(ss.str().c_str());
  }

  if (!d[kBody].HasMember("command")){
    std::stringstream ss;
    ss << "No body tag Error : " << s;
    throw message_handler_error(ss.str().c_str());
  }
  std::string cmd = d[kBody]["command"].GetString();

  std::shared_ptr<command> p;
  if(cmd == "set_power") {
    unsigned int value = d[kBody]["value"].GetInt();

    p = factory.create_set_power_command(value);
  }
  return p;
}

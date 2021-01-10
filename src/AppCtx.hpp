
#include <string>
#include <exception>
#include <stdexcept>
#include <vector>
#include <memory>

using namespace std;

#ifndef _AppCtx_hpp
#define _AppCtx_hpp

namespace raspserver {

class AppCtx {
	private:
		AppCtx(){}
		~AppCtx(){}

	public:
		static AppCtx& getInstance() {
			static AppCtx instance;
			return instance;
		}
};

}

#endif /*_AppCtx_hpp*/

#include <iostream>
#include "core/Application.h"
#include "utils/Log.h"
#include <cstdarg>
#include "utils/Global.h"

int main(){

	try{
		core::Application app;
		app.Init();
		app.Load();
		while (!app.mIsExit)
		{
			app.Run();
		}
		app.Clear();
	}
	catch (const std::exception& e){
		LOG_ERROR(e.what());
	}
	return 0;
}


#include "VMControllers/QEMUController.h"
#include "CollabVM.h"
#ifdef _WIN32
	#include <Windows.h>
	#include <shellapi.h>
#else
	#include <unistd.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <wordexp.h>
	#include <system_error>
	#include <signal.h>
	#include <dirent.h>
	#include <sys/resource.h>
#if defined(__FreeBSD__) || defined(__APPLE__)
	#include <sys/procctl.h>
#else
	#include <sys/prctl.h>
#endif
#endif
#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>
#include <cstdio>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <sstream>

// Here we gooooo

VMWareController::VMWareController(CollabVMServer& server, boost::asio::io_service& service, const std::shared_ptr<VMSettings>& settings)
	: VMController(server, service, settings),
	  guac_client_(server, *this, users_, settings->VNCAddress, settings->VNCPort),
	  internal_state_(InternalState::kInactive),
	  timer_(service),
	  retry_count_(0)
{
	SetVmwareName(settings->VmwareName);
    SetVmwareSnap(settings->VmwareSnap);


}

bool VMWareController::psexec(std::string command) {
    auto cmdLine = settings->pwshBin + "-command" + command;
    char* cmdLineL[];
    boost::split(cmdLineL, cmdLine, boost::is_any_of(" "));
    #ifdef _WIN32
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        PROCESS_INFORMATION pwsh_process_;
        ZeroMemory(&pwsh_process_, sizeof(pwsh_process_));
        BOOL cmdStatus = CreateProcess(nullptr, cmdLineW, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pwsh_process_);
    #else 
        pid_t parent_before_fork = getpid();
        pid_t pId = fork();
        if (pId == 0) {
            exit(exec(cmdLineL[0], cmdLineL));
        } else if (pId == -1) {
            std::cout << "Could not create process to run Powershell binary.";
            return FALSE;
        } else {
            wait();

        }
    #endif 
}

void VMWareController::SetVmwareName(const std::string& vmwarename) {
    std::cout << "VM Name: " << vmwarename << "\n";

}
#include <sstream>
#include <unistd.h>
#include <string>
#include <filesystem>
#include <fstream>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <vector>
#include <cstring>

#include"Process.h"

using namespace std;

bool LGetBaseAdress(size_t*, size_t*, pid_t);

bool Process::Attach(pid_t pid) {
	if (isAttached) return false;
	if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) return false;

	int status = 0;
	pid_t waited = waitpid(pid, &status, 0);

	if (waited == -1) return false;
	if (!LGetBaseAdress(&base, &size, pid)) return false;

	processPid = pid;
	isAttached = true;

	return true;
}

void Process::Detach() {
	if (!isAttached) return;
	ptrace(PTRACE_DETACH, processPid, nullptr, nullptr);
	isAttached = false;
}

size_t Process::GetBaseAdress() {
	return base;
}

size_t Process::GetBaseSize() {
	return size;
}

bool LGetBaseAdress(size_t* result, size_t* size, pid_t pid) {
	string maps_path = "/proc/" + to_string(pid) + "/maps";
	ifstream maps_file(maps_path);

	if (!maps_file.is_open()) return false;

	string line;
	while (getline(maps_file, line)) {
		if (line.find('/') != string::npos) { //Естли в строке есть символ / значит это путь к бинарнику
			stringstream ss(line); //Создаем поток чтоб легче было работать
			string base_addr_str;
			string base_addr_end_str;

			getline(ss, base_addr_str, '-');
			getline(ss, base_addr_end_str, ' ');

			*size = stoull(base_addr_end_str, nullptr, 16) - stoull(base_addr_str, nullptr, 16);

			*result = stoull(base_addr_str, nullptr, 16);
			return true;
		}
	}

	return false;
}

#ifndef PROCESS_H
#define PROCESS_H

#include <cstdint>
#include <sys/types.h>
#include <sys/uio.h>

class Process {
private:
	pid_t processPid;
	size_t base;
	size_t size; 
	bool isAttached = false;

public:
	bool Attach(pid_t pid);
	void Detach();

	size_t GetBaseAdress();
	size_t GetBaseSize();

	//ssize_t FindMemory(size_t, size_t, uint8_t[], uint8_t[], size_t);
	//size_t InjectMethod(size_t, void (*)());

	template<typename T>
	ssize_t RPC_WriteAbsolute(void *, T);

	template<typename T>
	ssize_t RPC_Write(void *, T);

	template<typename T>
	ssize_t RPC_ReadAbsolute(void *, T*);

	template<typename T>
	ssize_t RPC_Read(void *, T*);

	~Process();
};

template<typename T>
ssize_t Process::RPC_WriteAbsolute(void *address, T value) {
	if (!isAttached) return -1;

	struct iovec local_iov[1];
	local_iov[0].iov_base = &value;
	local_iov[0].iov_len = sizeof(T);

	struct iovec remote_iov[1];
	remote_iov[0].iov_base = (void*)address;
	remote_iov[0].iov_len = sizeof(T);

	return process_vm_writev(processPid, local_iov, 1, remote_iov, 1, 0);
}

template<typename T>
ssize_t Process::RPC_WriteBase(void *offset, T value) {
	return RPC_WriteAbsolute(base + offset, value);
}

template<typename T>
ssize_t Process::RPC_ReadAbsolute(void *address, T* result) {
	if (!isAttached) return -1;

	struct iovec local_iov[1];
	local_iov[0].iov_base = result;
	local_iov[0].iov_len = sizeof(T);

	struct iovec remote_iov[1];
	remote_iov[0].iov_base = (void*)address;
	remote_iov[0].iov_len = sizeof(T);

	return process_vm_readv(processPid, local_iov, 1, remote_iov, 1, 0);
}

template<typename T>
ssize_t Process::RPC_ReadBase(void *offset, T* result) {
	return RPC_ReadAbsolute(base + offset, result);
}

#endif

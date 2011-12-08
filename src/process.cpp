#include "xorg/gtest/process.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <vector>

struct xorg::testing::Process::Private {
  pid_t pid;
};

xorg::testing::Process::Process() : d_(new Private) {
  d_->pid = -1;
}

void xorg::testing::Process::Start(const std::string& program, va_list args) {
  if (d_->pid != -1)
    throw std::runtime_error("Attempting to start an already started process");

  d_->pid = vfork();

  if (d_->pid == -1) {
    throw std::runtime_error("Failed to fork child process");
  } else if (d_->pid == 0) { /* Child */
    close(0);
    close(1);
    close(2);

    std::vector<char*> argv;

    do
      argv.push_back(va_arg(args, char*));
    while (argv.back());

    execvp(program.c_str(), &argv[0]);

    throw std::runtime_error("Failed to start process");
  }
}

void xorg::testing::Process::Start(const std::string& program, ...) {
  va_list list;
  va_start(list, program);
  Start(program, list);
  va_end(list); /* Shouldn't get here */
}

bool xorg::testing::Process::Terminate() {
  if (d_->pid == -1) {
    return false;
  } else if (d_->pid == 0) {
    /* Child */
    throw std::runtime_error("Child process tried to terminate itself");
  } else { /* Parent */
    if (kill(d_->pid, SIGTERM) < 0) {
      return false;
    }
  }
  return true;
}

bool xorg::testing::Process::Kill() {
  if (d_->pid == -1) {
    return false;
  } else if (d_->pid == 0) {
    /* Child */
    throw std::runtime_error("Child process tried to kill itself");
  } else { /* Parent */
    if (kill(d_->pid, SIGKILL) < 0) {
      return false;
    }
  }
  return true;
}

void xorg::testing::Process::SetEnv(const char* name, const char* value,
                                    bool overwrite) {
  if (setenv(name, value, overwrite) != 0)
    throw std::runtime_error("Failed to set environment variable in process");

  return;
}

const char* xorg::testing::Process::GetEnv(const char* name) {
  return getenv(name);
}

pid_t xorg::testing::Process::Pid() const {
  return d_->pid;
}

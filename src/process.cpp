#include "xorg/gtest/process.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdarg.h>

#include <cstdio>
#include <stdexcept>

struct xorg::testing::Process::Private {
  pid_t pid;
};

xorg::testing::Process::Process() : d_(new Private) {
  d_->pid = -1;
}

xorg::testing::Process::~Process() {
  delete d_;
}

bool xorg::testing::Process::Start(const std::string& program, ...) {
  if (d_->pid == -1) {
    d_->pid = vfork();

    if (d_->pid == -1) {
      return false;
    } else if (d_->pid == 0) { /* Child */
      close(0);
      close(1);
      close(2);
      va_list list;
      va_start(list, program);
      execlp(program.c_str(), program.c_str(), list, NULL);
      perror("Failed to run process.");
      exit(-1);
    }
    return true;
  }
  return false;
}

int xorg::testing::Process::Wait(int* status, int options) {
  if (d_->pid == -1 || d_->pid == 0)
    return false;

  return waitpid(d_->pid, status, options);
}

bool xorg::testing::Process::Terminate() {
  if (d_->pid == -1) {
    return false;
  } else if (d_->pid == 0) {
    /* Child */
    return false;
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
    return false;
  } else { /* Parent */
    if (kill(d_->pid, SIGKILL) < 0) {
      return false;
    }
  }
  return true;
}

bool xorg::testing::Process::SetEnv(const char* name, const char* value,
                     Process::SetEnvBehaviour b) {
  if (name == NULL || value == NULL)
    return false;

  return setenv(name, value, static_cast<int>(b)) == 0;
}

const char* xorg::testing::Process::GetEnv(const char* name) {
  if (name == NULL)
    return NULL;
  return getenv(name);
}

pid_t xorg::testing::Process::pid() const {
  return d_->pid;
}

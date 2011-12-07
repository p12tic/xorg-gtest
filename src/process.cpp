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

namespace xorg {
namespace testing {

struct Process::Private {
  Private()
      : pid_(-1) {
  }

  pid_t pid_;
};

Process::Process()
    : d_(new Private()) {
}

Process::~Process() {
  delete d_;
}

bool Process::Start(const std::string& program, ...) {
  if (d_->pid_ == -1) {
    d_->pid_ = vfork();

    if (d_->pid_ == -1) {
      return false;
    } else if (d_->pid_ == 0) { /* Child */
      close(0);
      close(1);
      close(2);
      va_list list;
      va_start(list, program);
      execlp(program.c_str(), program.c_str(), list, NULL);
      perror("Failed to run process.");
      exit(-1);
    } else if (d_->pid_ > 0) { /* Parent */
    }
    return true;
  }
  return false;
}

int Process::Wait(int* status, int options) {
  if (d_->pid_ == -1 || d_->pid_ == 0)
    return false;

  return waitpid(d_->pid_, status, options);
}

bool Process::Terminate() {
  if (d_->pid_ == -1) {
    return false;
  } else if (d_->pid_ == 0) {
    /* Child */
    return false;
  } else { /* Parent */
    if (kill(d_->pid_, SIGTERM) < 0) {
      return (false);
    }
  }
  return true;
}

bool Process::Kill() {
  if (d_->pid_ == -1) {
    return false;
  } else if (d_->pid_ == 0) {
    /* Child */
    return false;
  } else { /* Parent */
    if (kill(d_->pid_, SIGKILL) < 0) {
      return (false);
    }
  }
  return true;
}

bool Process::SetEnv(const char* name, const char* value,
                     Process::SetEnvBehaviour b) {
  if (name == NULL || value == NULL)
    return false;

  return setenv(name, value, static_cast<int>(b)) == 0;
}

const char* Process::GetEnv(const char* name) {
  if (name == NULL)
    return NULL;
  return getenv(name);
}

pid_t Process::pid() const {
  return d_->pid_;
}

} // namespace testing
} // namespace xorg

#include "xorg/gtest/process.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

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

    char** argv = reinterpret_cast<char**>(malloc(sizeof(char*)));
    if (!argv)
      throw std::bad_alloc();

    int argv_size = 1;

    while (true) {
      char* arg = va_arg(args, char*);
      if (!arg)
        break;

      char** tmp = reinterpret_cast<char**>(
          realloc(argv, ++argv_size * sizeof(char*)));
      if (!tmp) {
        free(argv);
        throw std::bad_alloc();
      }

      argv = tmp;
      argv[argv_size - 2] = strdup(arg);
      if (!argv[argv_size - 2]) {
        free(argv);
        throw std::bad_alloc();
      }
    }

    argv[argv_size - 1] = NULL;

    execvp(program.c_str(), argv);

    for (int i = 0; i < argv_size; ++i)
      free(argv[i]);
    free(argv);
    throw std::runtime_error("Failed to start process");
  }
}

void xorg::testing::Process::Start(const std::string& program, ...) {
  va_list list;
  va_start(list, program);
  Start(program, list);
  va_end(list); /* Shouldn't get here */
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

pid_t xorg::testing::Process::Pid() const {
  return d_->pid;
}

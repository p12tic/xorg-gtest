/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2011, 2012 Canonical Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ******************************************************************************/

#include "xorg/gtest/xorg-gtest-process.h"

#include <sys/prctl.h>
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
  enum State state;
};

xorg::testing::Process::Process() : d_(new Private) {
  d_->pid = -1;
  d_->state = NONE;
}

enum xorg::testing::Process::State xorg::testing::Process::GetState() {
  if (d_->state == RUNNING) {
    int status;
    int pid = waitpid(Pid(), &status, WNOHANG);
    if (pid == Pid()) {
      if (WIFEXITED(status)) {
        d_->pid = -1;
        d_->state = WEXITSTATUS(status) ? FINISHED_FAILURE : FINISHED_SUCCESS;
      }
    }
  }

  return d_->state;
}

void xorg::testing::Process::Start(const std::string &program, const std::vector<std::string> &argv) {
  if (d_->pid != -1)
    throw std::runtime_error("Attempting to start an already started process");

  d_->pid = fork();

  if (d_->pid == -1) {
    d_->state = ERROR;
    throw std::runtime_error("Failed to fork child process");
  } else if (d_->pid == 0) { /* Child */
    close(0);
    if (getenv("XORG_GTEST_CHILD_STDOUT") == NULL) {
      close(1);
      close(2);
    }

#ifdef __linux
    prctl(PR_SET_PDEATHSIG, SIGTERM);
#endif

    std::vector<char*> args;
    std::vector<std::string>::const_iterator it;

    args.push_back(strdup(program.c_str()));

    for (it = argv.begin(); it != argv.end(); it++)
      args.push_back(strdup(it->c_str()));
    args.push_back(NULL);

    execvp(program.c_str(), &args[0]);

    d_->state = ERROR;
    throw std::runtime_error("Failed to start process");
  }

  d_->state = RUNNING;
}

void xorg::testing::Process::Start(const std::string& program, va_list args) {
  std::vector<std::string> argv;

  if (args) {
    char *arg;
    do {
      arg = va_arg(args, char*);
      if (arg)
        argv.push_back(std::string(arg));
    } while (arg);
  }

  Start(program, argv);
}

void xorg::testing::Process::Start(const std::string& program, ...) {
  va_list list;
  va_start(list, program);
  Start(program, list);
  va_end(list); /* Shouldn't get here */
}

bool xorg::testing::Process::WaitForExit(unsigned int timeout) {
  for (int i = 0; i < timeout * 100; i++) {
    int status;
    int pid = waitpid(Pid(), &status, WNOHANG);

    if (pid == Pid())
      return true;

      usleep(10);
  }

  return false;
}

bool xorg::testing::Process::KillSelf(int signal, unsigned int timeout) {
  bool wait_success = true;

  enum State state = GetState();
  switch (state) {
    case FINISHED_SUCCESS:
    case FINISHED_FAILURE:
    case TERMINATED:
      return true;
    case ERROR:
    case NONE:
      return false;
    default:
      break;
  }

  if (d_->pid == -1) {
    return false;
  } else if (d_->pid == 0) {
    /* Child */
    throw std::runtime_error("Child process tried to kill itself");
  } else { /* Parent */
    if (kill(d_->pid, signal) < 0) {
      d_->pid = -1;
      d_->state = ERROR;
      return false;
    }
    if (timeout > 0)
      wait_success = WaitForExit(timeout);
    d_->pid = -1;
  }
  d_->state = TERMINATED;
  return wait_success;
}

bool xorg::testing::Process::Terminate(unsigned int timeout) {
  return KillSelf(SIGTERM, timeout);
}

bool xorg::testing::Process::Kill(unsigned int timeout) {
  return KillSelf(SIGKILL, timeout);
}

void xorg::testing::Process::SetEnv(const std::string& name,
                                    const std::string& value, bool overwrite) {
  if (setenv(name.c_str(), value.c_str(), overwrite) != 0)
    throw std::runtime_error("Failed to set environment variable in process");

  return;
}

std::string xorg::testing::Process::GetEnv(const std::string& name,
                                           bool* exists) {
  const char* var = getenv(name.c_str());
  if (exists != NULL)
    *exists = (var != NULL);

  return std::string(var);
}

pid_t xorg::testing::Process::Pid() const {
  return d_->pid;
}

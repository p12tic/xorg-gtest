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

void xorg::testing::Process::Start(const std::string &program, const std::vector<std::string> &argv) {
  if (d_->pid != -1)
    throw std::runtime_error("Attempting to start an already started process");

  d_->pid = vfork();

  if (d_->pid == -1) {
    throw std::runtime_error("Failed to fork child process");
  } else if (d_->pid == 0) { /* Child */
    close(0);
    close(1);
    close(2);

    std::vector<char*> args;
    std::vector<std::string>::const_iterator it;

    for (it = argv.begin(); it != argv.end(); it++)
      if (!it->empty())
        args.push_back(strdup(it->c_str()));
    args.push_back(NULL);

    execvp(program.c_str(), &args[0]);

    throw std::runtime_error("Failed to start process");
  }
}

void xorg::testing::Process::Start(const std::string& program, va_list args) {
  std::vector<std::string> argv;

  do {
    std::string arg(va_arg(args, char*));
    argv.push_back(arg);
  } while (!argv.back().empty());

  Start(program, argv);
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
      d_->pid = -1;
      return false;
    }
    d_->pid = -1;
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
      d_->pid = -1;
      return false;
    }
    d_->pid = -1;
  }
  return true;
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

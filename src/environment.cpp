/*******************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
 *
 * Copyright (C) 2011, 2012 Canonical Ltd.
 * Copyright © 2012 Red Hat, Inc.
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

#include "xorg/gtest/xorg-gtest-environment.h"
#include "xorg/gtest/xorg-gtest-process.h"
#include "xorg/gtest/xorg-gtest-xserver.h"

#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include <X11/Xlib.h>

struct xorg::testing::Environment::Private {
  Private() : display(-1) {
  }
  std::string path_to_conf;
  std::string path_to_log_file;
  std::string path_to_server;
  int display;
  XServer server;
};

xorg::testing::Environment::Environment()
    : d_(new Private) {
}

xorg::testing::Environment::~Environment() {}

void xorg::testing::Environment::set_log_file(const std::string& path_to_log_file)
{
  SetLogFile(path_to_log_file);
}

void xorg::testing::Environment::SetLogFile(const std::string& path_to_log_file)
{
  d_->path_to_log_file = path_to_log_file;
}

const std::string& xorg::testing::Environment::GetLogFile() const
{
  return d_->path_to_log_file;
}

void xorg::testing::Environment::SetConfigFile(const std::string& path_to_conf_file)
{
  d_->path_to_conf = path_to_conf_file;
}

const std::string& xorg::testing::Environment::GetConfigFile() const
{
  return d_->path_to_conf;
}

void xorg::testing::Environment::SetServerPath(const std::string& path_to_server)
{
  d_->path_to_server = path_to_server;
}

const std::string& xorg::testing::Environment::GetServerPath() const
{
  return d_->path_to_server;
}

void xorg::testing::Environment::SetDisplayNumber(int display_num)
{
  d_->display = display_num;
}

void xorg::testing::Environment::SetUp() {
  if (d_->display >= 0)
    d_->server.SetDisplayNumber(d_->display);
  if (d_->path_to_log_file.length())
    d_->server.SetOption("-logfile", d_->path_to_log_file);
  if (d_->path_to_conf.length())
    d_->server.SetOption("-config", d_->path_to_log_file);
  d_->server.Start(d_->path_to_server);

  Process::SetEnv("DISPLAY", d_->server.GetDisplayString(), true);
}

void xorg::testing::Environment::TearDown() {
  if (!d_->server.Terminate(1000))
    Kill();
}

void xorg::testing::Environment::Kill() {
  d_->server.Kill(1000);
}


/* DEPRECATED */
const std::string& xorg::testing::Environment::log_file() const
{
  return GetLogFile();
}

void xorg::testing::Environment::set_conf_file(const std::string& path_conf_file)
{
  return SetConfigFile(path_conf_file);
}

const std::string& xorg::testing::Environment::conf_file() const
{
  return GetConfigFile();
}

void xorg::testing::Environment::set_server(const std::string& path_to_server)
{
  SetServerPath(path_to_server);
}

const std::string& xorg::testing::Environment::server() const
{
  return GetServerPath();
}

void xorg::testing::Environment::set_display(int display_num)
{
  SetDisplayNumber(display_num);
}

int xorg::testing::Environment::display() const
{
  return d_->display;
}

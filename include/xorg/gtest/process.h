/*****************************************************************************
 *
 * utouch-frame - Touch Frame Library
 *
 * Copyright (C) 2011 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************************/
#ifndef XORG_GTEST_PROCESS_H
#define XORG_GTEST_PROCESS_H

#include <stdarg.h>

#include <memory>
#include <string>

namespace xorg {
namespace testing {

class Process {
 public:

  enum SetEnvBehaviour {
    DONT_OVERWRITE_EXISTING_VALUE = 0,
    OVERWRITE_EXISTING_VALUE = 1
  };

  Process();

  void Start(const std::string& program, va_list args);
  void Start(const std::string& program, ...);

  bool Terminate();
  bool Kill();

  void SetEnv(const char* name, const char* value, bool overwrite);
  const char * GetEnv(const char* name);

  pid_t Pid() const;

 private:
  struct Private;
  std::auto_ptr<Private> d_;

  /* Disable copy c'tor, assignment operator */
  Process(const Process&);
  Process& operator=(const Process&);
};

} // xorg
} // testing

#endif // XORG_GTEST_PROCESS_H

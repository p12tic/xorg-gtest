/*****************************************************************************
 *
 * X testing environment - Google Test environment featuring dummy x server
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

/**
 * @class Process test.h xorg/gtest/process.h
 *
 * Class that abstracts child process creation and termination.
 *
 * This class allows for forking, running and terminating child processes.
 * In addition, manipulation of the child process' environment is supported.
 * For example, starting an X server instance on display port 133 as a child
 * process can be realized with the following code snippet:
 * @code
 * Process xorgServer;
 * try {
 *   xorgServer.Start("Xorg", "Xorg", ":133");
 * } catch (const std::runtime_error&e) {
 *   std::cerr << "Problem starting the X server: " << e.what() << std::endl;
 * }
 * ...
 * if (!xorgServer.Terminate()) {
 *   std::cerr << "Problem terminating server ... killing now ..." << std::endl;
 *   if (!xorgServer.Kill())
 *     std::cerr << "Problem killing server" << std::endl;
 * }
 * @endcode
 */
class Process {
 public:
  /**
   * Creates a child-process that is in a terminated state.
   */
  Process();

  /**
   * Starts a program as a child process.
   *
   * See 'man execvp' for further information on the variadic argument list.
   *
   * @param program The program to start.
   * @param args Variadic list of arguments passed to the program.
   *
   * @throws std::runtime_error on failure.
   */
  void Start(const std::string& program, va_list args);

  /**
   * Starts a program as a child process.
   *
   * Takes a variadic list of arguments passed to the program.
   * See 'man execvp' for further information on the variadic argument list.
   *
   * @param program The program to start.
   *
   * @throws std::runtime_error on failure.
   */
  void Start(const std::string& program, ...);

  /**
   * Terminates (SIGTERM) this child process.
   *
   * @throws std::runtime_error if child tries to terminate itself.
   *
   * @returns true if termination succeeded, false otherwise.
   *
   */
  bool Terminate();

  /**
   * Kills (SIGKILL) this child process.
   *
   * @throws std::runtime_error if child tries to kill itself.
   *
   * @returns true if kill succeeded, false otherwise.
   */
  bool Kill();

  /**
   * Adjusts the environment of the child process.
   *
   * @param name Name of the environment variable, must not be NULL.
   * @param value Value of the environment variable, must not be NULL.
   * @param overwrite Whether to overwrite the value of existing env variables.
   *
   * @throws std::runtime_error if adjusting the environment does not succeed.
   */
  void SetEnv(const char* name, const char* value, bool overwrite);

  /**
   * Queries the environment of the child process.
   *
   * @param name The name of the environment variable, must not be NULL.
   *
   * @returns The value of the environment variable, or NULL.
   */
  const char * GetEnv(const char* name);

  /**
   * Accesses the pid of the child process.
   *
   * @returns The pid of the child process.
   */
  pid_t Pid() const;

 private:
  struct Private;
  std::auto_ptr<Private> d_;

  /* Disable copy constructor, assignment operator */
  Process(const Process&);
  Process& operator=(const Process&);
};

} // testing
} // xorg

#endif // XORG_GTEST_PROCESS_H

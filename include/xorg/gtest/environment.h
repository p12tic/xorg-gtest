/*****************************************************************************
 *
 * X testing environment - Google Test environment feat. dummy x server
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
#ifndef XORG_GTEST_ENVIRONMENT_H
#define XORG_GTEST_ENVIRONMENT_H

#include <memory>
#include <string>

#include <gtest/gtest.h>

namespace xorg {
namespace testing {

/**
 * \mainpage X.org Google %Test Framework
 *
 * Xorg-gtest makes it easy to write test cases
 * for a dummy headless X.org server. It can also run tests
 * using a running X11 server.
 *
 */

/**
 * @class Environment environment.h xorg/gtest/environment.h
 *
 * Global Google %Test environment providing a dummy X server.
 *
 * Starts up a dummy X server for testing purposes.
 * Either associate the environment manually
 * with the overall testing framework like
 * @code
 * std::string xorg_conf_path("conf/dummy.conf");
 * int xorg_display = 133;
 * std::string server("Xorg");
 *
 * xorg::testing::Environment* environment = new xorg::testing::Environment(
 *       xorg_conf_path,
 *       server,
 *       xorg_display);
 * testing::AddGlobalTestEnvironment(environment);
 * @endcode
 * or link to libxorg-gtest_main.
 */
class Environment : public ::testing::Environment {
 public:
  /**
   * Constructs an object to provide a global X server dummy environment.
   * @param path_to_conf Path to xserver configuration.
   * @param path_to_server Path to xserver executable.
   * @param display Display port of dummy xserver instance.
   */
  Environment(const std::string& path_to_conf,
              const std::string& path_to_server = "Xorg", int display = 133);

  /**
   * Starts the dummy X server.
   *
   * Reimplemented from ::testing::Environment. Should only be called by subclasses.
   * See Google %Test documentation for details.
   *
   * @throws std::runtime_error if a dummy X server cannot be started.
   *
   * @post If successful: subsequent connections to the dummy X server succeed.
   * @post If successful: %Environment variable DISPLAY contains the
   * display port for connecting to the dummy X server.
   */
  virtual void SetUp();

  /**
   * Stops the dummy X server.
   *
   * Reimplemented from ::testing::Environment. Should only be called by subclasses.
   * See Google %Test documentation for details.
   *
   * @post Dummy X server stopped.
   */
  virtual void TearDown();

 private:
  struct Private;
  std::auto_ptr<Private> d_;

  /* Disable copy constructor & assignment operator */
  Environment(const Environment&);
  Environment& operator=(const Environment&);
};

} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_ENVIRONMENT_H

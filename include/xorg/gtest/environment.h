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
 * xorg::testing::Environment* environment = new xorg::testing::Environment;
 * environment->set_server("Xorg");
 * environment->set_display(133);
 * environment->set_conf_file("conf/dummy.conf");
 * environment->set_log_file("/tmp/MyDummyXorg.log");
 * testing::AddGlobalTestEnvironment(environment);
 * @endcode
 * or link to libxorg-gtest_main.
 */
class Environment : public ::testing::Environment {
 public:
  /**
   * Constructs an object to provide a global X server dummy environment.
   */
  Environment();

  virtual ~Environment();

  /**
   * Sets the path where the server log file will be created.
   *
   * The path will be passed on to the server via the command line argument
   * "-logfile". The default value is "/tmp/Xorg.GTest.log".
   *
   * @param path_to_log_file Path to server logfile.
   */
  void set_log_file(const std::string& path_to_log_file);

  /**
   * Returns the path where the server log file will be created.
   *
   * @return Path to server logfile.
   */
  const std::string& log_file() const;

  /**
   * Sets the path to the desired server configuration file.
   *
   * The path will be passed on to the server via the command line argument
   * "-config". The default value is "[datadir]/xorg/gtest/dummy.conf".
   *
   * @param path_conf_file Path to a Xorg X server .conf file.
   */
  void set_conf_file(const std::string& path_conf_file);

  /**
   * Returns the path of the server configuration file to be used.
   *
   * @return File path of the server configuration currently set
   */
  const std::string& conf_file() const;

  /**
   * Sets the path to the server executable
   *
   * The default value is "Xorg".
   *
   * @param path_to_server Path to an X.org server executable
   */
  void set_server(const std::string& path_to_server);

  /**
   * Returns the path of the server executable to be used.
   *
   * @return Path to server executable.
   */
  const std::string& server() const;

  /**
   * Sets the display number that the server will use.
   *
   * The display number will be passed on to the server via the command line.
   * The default value is 133.
   *
   * @param diplay_num A display number.
   */
  void set_display(int display_num);

  /**
   * Returns the display number of the server instance.
   *
   * @return Display number of the server.
   */
  int display() const;

 protected:
  /**
   * Starts the dummy X server.
   *
   * Reimplemented from ::testing::Environment. See Google %Test documentation
   * for details.
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
   * Reimplemented from ::testing::Environment. See Google %Test documentation
   * for details.
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

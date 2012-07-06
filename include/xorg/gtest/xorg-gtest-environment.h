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
 * @class Environment xorg-gtest-environment.h environment.h xorg/gtest/xorg-gtest-environment.h
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
  void SetLogFile(const std::string& path_to_log_file);

  /**
   * Returns the path where the server log file will be created.
   *
   * @return Path to server logfile.
   */
  const std::string& GetLogFile() const;

  /**
   * Sets the path to the desired server configuration file.
   *
   * The path will be passed on to the server via the command line argument
   * "-config". The default value is "[datadir]/xorg/gtest/dummy.conf".
   *
   * @param path_to_conf_file Path to a Xorg X server .conf file.
   */
  void SetConfigFile(const std::string& path_to_conf_file);

  /**
   * Returns the path of the server configuration file to be used.
   *
   * @return File path of the server configuration currently set
   */
  const std::string& GetConfigFile() const;

  /**
   * Sets the path to the server executable
   *
   * The default value is "Xorg".
   *
   * @param path_to_server Path to an X.org server executable
   */
  void SetServerPath(const std::string& path_to_server);

  /**
   * Returns the path of the server executable to be used.
   *
   * @return Path to server executable.
   */
  const std::string& GetServerPath() const;

  /**
   * Sets the display number that the server will use.
   *
   * The display number will be passed on to the server via the command line.
   * The default value is 133.
   *
   * @param display_num A display number.
   */
  void SetDisplayNumber(int display_num);

  /**
   * Returns the display number of the server instance.
   *
   * @return Display number of the server.
   */
  int GetDisplayNumber() const;

  /**
   * Kill the dummy Xorg server with SIGKILL.
   */
  void Kill();

  /* DEPRECATED */
  /**
   * @deprecated
   * @see SetLogFile
   */
  void set_log_file(const std::string& path_to_log_file);

  /**
   * @deprecated
   * @see SetLogFile
   */
  const std::string& log_file() const;

  /**
   * @deprecated
   * @see SetConfigFile
   */
  void set_conf_file(const std::string& path_conf_file);

  /**
   * @deprecated
   * @see GetConfigFile
   */
  const std::string& conf_file() const;

  /**
   * @deprecated
   * @see SetServerPath
   */
  void set_server(const std::string& path_to_server);

  /**
   * @deprecated
   * @see GetServerPath
   */
  const std::string& server() const;

  /**
   * @deprecated
   * @see SetDisplay
   */
  void set_display(int display_num);

  /**
   * @deprecated
   * @see GetDisplayNumber()
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

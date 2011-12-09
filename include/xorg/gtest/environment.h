/*****************************************************************************
 *
 * xorg-gtest - Google test addon for dummy xserver setup
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
 * @brief Dummy Xorg Google Test environment.
 *
 * Starts up a dummy Xorg server for testing purposes on
 * display :133. Either associate the environment manually
 * with the overall testing framework or link to libxorg-gtest_main.
 */
class Environment : public ::testing::Environment {
 public:
  /**
   * @brief C'tor.
   * @param path_to_conf Path to xserver configuration.
   * @param path_to_server Path to xserver executable.
   * @param display Display port of dummy xserver instance.
   */
  Environment(const std::string& path_to_conf,
              const std::string& path_to_server = "Xorg", int display = 133);

  /**
   * @brief Starts the dummy xserver.
   *
   * @throws std::runtime_error if dummy xserver cannot be started.
   */
  virtual void SetUp();

  /**
   * @brief Stops the dummy xserver.
   */
  virtual void TearDown();

 private:
  struct Private;
  std::auto_ptr<Private> d_;

  /* Disable copy c'tor & assignment op. */
  Environment(const Environment&);
  Environment& operator=(const Environment&);
};

} // namespace testing
} // namespace xorg

#endif // XORG_GTEST_ENVIRONMENT_H

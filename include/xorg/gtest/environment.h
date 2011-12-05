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

#include <string>

#include <gtest/gtest.h>

namespace testing {
namespace xorg {

/**
 * @brief Dummy Xorg Google Test environment.
 *
 * Starts up a dummy Xorg server for testing purposes on
 * display :133. Either associate the environment manually
 * with the overall testing framework or link to libxtestingenvironment_main.a.
 */
class Environment : public testing::Environment {
 public:
  Environment(const std::string& pathToConf, int display = 133);

  virtual void SetUp();
  virtual void TearDown();
 private:
  std::string path_to_conf_;
  int display_;
  pid_t child_pid_;
};

} // namespace xorg
} // namespace testing

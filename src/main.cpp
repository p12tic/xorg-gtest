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

#include <getopt.h>

#include <gtest/gtest.h>

#include "xorg/gtest/environment.h"

namespace {

int help = false;
int no_dummy_server = false;
int xorg_conf = false;
int xorg_display_opt = false;

const struct option longopts[] = {
  { "help", no_argument, &help, true, },
  { "no-dummy-server", no_argument, &no_dummy_server, true, },
  { "xorg-conf", optional_argument, &xorg_conf, true, },
  { "xorg-display", optional_argument, &xorg_display_opt, true, },
  { NULL, 0, NULL, 0 }
};

} // namespace

int main(int argc, char *argv[]) {
  /* Default Xorg dummy conf path. */
  std::string xorg_conf_path("x11/dummy.conf");

  /* Default X display */
  int xorg_display = 133;

  /* Reset getopt state */
  optind = 0;

  int ret;
  do {
    ret = getopt_long(argc, argv, "", longopts, NULL);

    if (xorg_conf) {
      xorg_conf_path = optarg;
    }

    if (xorg_display_opt) {
      xorg_display = atoi(optarg);
    }
  } while (ret == 0);

  if (ret != -1)
    exit(-1);

  if (help) {
    std::cout << "\nAdditional options:\n";
    std::cout << "    --no-dummy-server: Use the currently running X server "
        "for testing\n";
    std::cout << "    --xorg-conf: Path to xorg dummy configuration file\n";
    std::cout << "    --xorg-display: xorg dummy display port\n";
    exit(-1);
  }

  testing::InitGoogleTest(&argc, argv);

  if (!no_dummy_server) {
    testing::xorg::Environment* environment = new testing::xorg::Environment(
        xorg_conf_path,
        xorg_display);
    testing::AddGlobalTestEnvironment(environment);
  }

  return RUN_ALL_TESTS();
}

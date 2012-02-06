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
#include "defines.h"

namespace {

int help = false;
int no_dummy_server = false;
int xorg_conf_specified = false;
int xorg_display_specified = false;
int xorg_logfile_specified = false;
int server_specified = false;

const struct option longopts[] = {
  { "help", no_argument, &help, true, },
  { "no-dummy-server", no_argument, &no_dummy_server, true, },
  { "xorg-conf", required_argument, &xorg_conf_specified, true, },
  { "xorg-display", required_argument, &xorg_display_specified, true, },
  { "xorg-logfile", required_argument, &xorg_logfile_specified, true, },
  { "server", required_argument, &server_specified, true, },
  { NULL, 0, NULL, 0 }
};

} // namespace

int main(int argc, char *argv[]) {
  std::string xorg_conf_path;
  std::string xorg_log_file_path;
  int xorg_display = -1;
  std::string server;

  testing::InitGoogleTest(&argc, argv);

  /* Reset getopt state */
  optind = 0;

  while (true) {
    int ret;
    int index;
    ret = getopt_long(argc, argv, "", longopts, &index);

    if (ret == -1)
      break;

    if (ret == '?')
      exit(-1);

    switch (index) {
      case 2:
        xorg_conf_path = optarg;
        break;

      case 3:
        xorg_display = atoi(optarg);
        break;

      case 4:
        xorg_log_file_path = optarg;
        break;

      case 5:
        server = optarg;
        break;

      default:
        break;
    }
  }

  if (help) {
    std::cout << "\nAdditional options:\n";
    std::cout << "    --no-dummy-server: Use the currently running X server "
        "for testing\n";
    std::cout << "    --xorg-conf: Path to xorg dummy configuration file\n";
    std::cout << "    --server: Path to X server executable\n";
    std::cout << "    --xorg-display: xorg dummy display port\n";
    std::cout << "    --xorg-logfile: xorg logfile filename. See -logfile in \"man Xorg\".\n"
                 "                    Its default value is "DEFAULT_XORG_LOGFILE".\n";
    exit(-1);
  }

  if (!no_dummy_server) {
    xorg::testing::Environment* environment = new xorg::testing::Environment;

    if (xorg_conf_specified)
      environment->set_conf_file(xorg_conf_path);

    if (server_specified)
      environment->set_server(server);

    if (xorg_display_specified)
      environment->set_display(xorg_display);

    if (xorg_logfile_specified)
        environment->set_log_file(xorg_log_file_path);

    testing::AddGlobalTestEnvironment(environment);
  }

  return RUN_ALL_TESTS();
}

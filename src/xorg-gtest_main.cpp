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

#include <getopt.h>

#include <csignal>

#include <gtest/gtest.h>

#include "xorg/gtest/xorg-gtest-environment.h"
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

xorg::testing::Environment* environment = NULL;

static void signal_handler(int signum) {
  if (environment)
    environment->Kill();
  
  /* This will call the default handler because we used SA_RESETHAND */
  raise(signum);
}

static void setup_signal_handlers() {
  static const int signals[] = {
    SIGHUP,
    SIGTERM,
    SIGQUIT,
    SIGILL,
    SIGABRT,
    SIGFPE,
    SIGSEGV,
    SIGPIPE,
    SIGALRM,
    SIGTERM,
    SIGUSR1,
    SIGUSR2,
    SIGBUS,
    SIGPOLL,
    SIGPROF,
    SIGSYS,
    SIGTRAP,
    SIGVTALRM,
    SIGXCPU,
    SIGXFSZ,
    SIGIOT,
    SIGSTKFLT,
    SIGIO,
    SIGPWR,
    SIGUNUSED,
  };

  struct sigaction action;
  action.sa_handler = signal_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESETHAND;

  for (unsigned i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i)
    if (sigaction(signals[i], &action, NULL))
      std::cerr << "Warning: Failed to set signal handler for signal "
                << signals[i] << "\n";
}

static int usage(int exitcode) {
  std::cout << "\nAdditional options:\n";
  std::cout << "    --no-dummy-server: Use the currently running X server "
               "for testing\n";
  std::cout << "    --xorg-conf: Path to xorg dummy configuration file\n";
  std::cout << "    --server: Path to X server executable\n";
  std::cout << "    --xorg-display: xorg dummy display port\n";
  std::cout << "    --xorg-logfile: xorg logfile filename. See -logfile in \"man Xorg\".\n"
               "                    Its default value is "DEFAULT_XORG_LOGFILE".\n";
  return exitcode;
}

int main(int argc, char *argv[]) {
  std::string xorg_conf_path;
  std::string xorg_log_file_path;
  int xorg_display = -1;
  std::string server;

  setup_signal_handlers();

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

  if (help)
    return usage(-1);

  if (!no_dummy_server) {
    environment = new xorg::testing::Environment;

    if (xorg_conf_specified)
      environment->SetConfigFile(xorg_conf_path);

    if (server_specified)
      environment->SetServerPath(server);

    if (xorg_display_specified)
      environment->SetDisplayNumber(xorg_display);

    if (xorg_logfile_specified)
      environment->SetLogFile(xorg_log_file_path);

    testing::AddGlobalTestEnvironment(environment);
  }

  return RUN_ALL_TESTS();
}

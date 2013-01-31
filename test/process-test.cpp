#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <gtest/gtest.h>
#include <xorg/gtest/xorg-gtest.h>

#include <stdexcept>

using namespace xorg::testing;

TEST(Process, StartWithNULLArg)
{
  XORG_TESTCASE("invocation of 'ls' with no arguments");
  Process p;
  p.Start("ls", NULL);
  ASSERT_GT(p.Pid(), 0);
}

TEST(Process, StartWithNULLTerminatedArg)
{
  XORG_TESTCASE("invocation of 'ls' with NULL-terminated argument list");

  Process p;
  p.Start("ls", "-l", NULL);
  ASSERT_GT(p.Pid(), 0);
}

TEST(Process, ExitCodeSuccess)
{
  XORG_TESTCASE("invocation of 'echo -n', check for success exit status");

  Process p;
  ASSERT_EQ(p.GetState(), Process::NONE);

  /* Process:Start closes stdout, so we need something that doesn't print */
  p.Start("echo", "-n", NULL);
  ASSERT_GT(p.Pid(), 0);
  ASSERT_EQ(p.GetState(), Process::RUNNING);

  /* ls shouldn't take longer terminate */
  for (int i = 0; i < 100; i++) {
    if (p.GetState() == Process::RUNNING)
      usleep(5000);
  }
  ASSERT_EQ(p.GetState(), Process::FINISHED_SUCCESS);
}

TEST(Process, ExitCodeFailure)
{
  XORG_TESTCASE("an invalid invocation of 'ls', check for error exit status");
  Process p;
  ASSERT_EQ(p.GetState(), Process::NONE);

  /* Process:Start closes stdout, so ls should fail with status 2, if not,
   * that file is unlikely to exists so we get status 1 */
  p.Start("ls", "asqwerq.aqerqw_rqwe", NULL);
  ASSERT_GT(p.Pid(), 0);
  ASSERT_EQ(p.GetState(), Process::RUNNING);

  /* ls shouldn't take longer than 5s to terminate */
  for (int i = 0; i < 10; i++) {
    if (p.GetState() == Process::RUNNING)
      usleep(500);
  }
  ASSERT_EQ(p.GetState(), Process::FINISHED_FAILURE);
}

TEST(Process, ChildTearDown)
{
  XORG_TESTCASE("ensure child process dies when parent does");

  int pipefd[2];
  ASSERT_NE(pipe(pipefd), -1);

  /* Fork, the child will spawn a Process, write that Process's PID to a
     pipe and then kill itself. The parent checks that the child was
     terminated when the parent was killed.
   */
  pid_t pid = fork();
  if (pid == 0) { /* child */
    close(pipefd[0]);

    Process p;
    p.Start("sleep", "1000", NULL); /* forks another child */
    ASSERT_GT(p.Pid(), 0);

    char *buffer;
    ASSERT_GT(asprintf(&buffer, "%d", p.Pid()), 0);
    ASSERT_EQ(write(pipefd[1], buffer, strlen(buffer)), (int)strlen(buffer));
    close(pipefd[1]);

    raise(SIGKILL);
  } else { /* parent */
    close(pipefd[1]);

    char buffer[20] = {0};
    ASSERT_GT(read(pipefd[0], buffer, sizeof(buffer)), 0);
    close(pipefd[0]);

    pid_t child_pid = atoi(buffer);
    for (int i = 0; i < 10; i++) {
      if (kill(child_pid, 0) != -1)
        usleep(100);

    }
    ASSERT_EQ(kill(child_pid, 0), -1);
    ASSERT_EQ(errno, ESRCH);
  }
}

TEST(Process, TerminationFailure)
{
  XORG_TESTCASE("if Process::Terminate() fails to terminate the \n"
                "child process, kill must terminate it it instead");

  sigset_t sig_mask;
  struct timespec sig_timeout = {0, 5000000L};

  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGUSR1);

  Process p;
  p.Start(TEST_ROOT_DIR "process-test-helper", NULL);
  /* don't check error here, the helper may have sent the signal before we
     get here */
  sigtimedwait(&sig_mask, NULL, &sig_timeout);

  ASSERT_GT(p.Pid(), 0);
  kill(p.Pid(), SIGSTOP);

  ASSERT_FALSE(p.Terminate(100));
  ASSERT_EQ(p.GetState(), Process::RUNNING);
  ASSERT_TRUE(p.Kill(100));
}

TEST(Process, KillExitStatus)
{
  XORG_TESTCASE("a child process killed must have a state of\n"
                "FINISHED_FAILURE");
  Process p;
  p.Start(TEST_ROOT_DIR "process-test-helper", NULL);
  p.Kill(1000);
  ASSERT_EQ(p.GetState(), Process::FINISHED_FAILURE);
}

TEST(Process, DoubleStart)
{
  struct timespec sig_timeout = {0, 5000000L};

  XORG_TESTCASE("starting a process after it has been started\n"
                "fails. Re-starting a process succeeds\n");

  /* Process double-started must fail */
  Process p;
  p.Start("echo", "-n", NULL);
  try {
    p.Start("echo", "-n", NULL);;
    FAIL() << "Expected exception";
  } catch (std::runtime_error &e) {
  }
  p.Terminate(1000);
  /* we sent it a SIGTERM, counts as failure */
  ASSERT_EQ(p.GetState(), Process::FINISHED_FAILURE);

  sigset_t sig_mask;
  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGCHLD);
  sigaddset(&sig_mask, SIGUSR1);
  sigprocmask(SIG_BLOCK, &sig_mask, 0);

  /* restart job after a failed one, must succeed */
  try {
    p.Start("echo", "-n", NULL);
  } catch (std::runtime_error &e) {
    FAIL();
  }

  ASSERT_EQ(sigtimedwait(&sig_mask, NULL, &sig_timeout), SIGCHLD);
  ASSERT_EQ(p.GetState(), Process::FINISHED_SUCCESS);

  /* restart job after successful one, must succeed */
  try {
    p.Start("echo", "-n", NULL);
  } catch (std::runtime_error &e) {
    FAIL();
  }
  ASSERT_EQ(sigtimedwait(&sig_mask, NULL, &sig_timeout), SIGCHLD);
  ASSERT_EQ(p.GetState(), Process::FINISHED_SUCCESS);

  /* job that must be killed, followed by job */
  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGUSR1);
  p.Start(TEST_ROOT_DIR "process-test-helper", NULL);
  sigtimedwait(&sig_mask, NULL, &sig_timeout);
  ASSERT_EQ(p.GetState(), Process::RUNNING);
  p.Kill(100);
  ASSERT_EQ(p.GetState(), Process::FINISHED_FAILURE);

  /* restart job after successful one, must succeed */
  try {
    p.Start("echo", "-n", NULL);
  } catch (std::runtime_error &e) {
    FAIL();
  }
  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGCHLD);
  ASSERT_EQ(sigtimedwait(&sig_mask, NULL, &sig_timeout), SIGCHLD);
  ASSERT_EQ(p.GetState(), Process::FINISHED_SUCCESS);

  /* job that fails to terminate, starting another one must fail */
  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGUSR1);
  p.Start(TEST_ROOT_DIR "process-test-helper", NULL);
  sigtimedwait(&sig_mask, NULL, &sig_timeout);
  ASSERT_EQ(p.GetState(), Process::RUNNING);
  ASSERT_FALSE(p.Terminate(100));

  try {
    p.Start("echo", "-n", NULL);
    FAIL() << "exception expected";
  } catch (std::runtime_error &e) {
  }

  sigemptyset(&sig_mask);
  sigaddset(&sig_mask, SIGCHLD);
  sigaddset(&sig_mask, SIGUSR1);
  sigprocmask(SIG_UNBLOCK, &sig_mask, 0);
}

TEST(Process, ForkedParentStart)
{
  XORG_TESTCASE("Fork() and calling Start() on the parent causes an exception");
  Process p;
  if (p.Fork() > 0) {
    ASSERT_GT(p.Pid(), 0);
    ASSERT_EQ(p.GetState(), Process::RUNNING);
    ASSERT_THROW({ p.Start("ls", NULL); }, std::runtime_error);
  }
}

TEST(Process, ForkedChildStart)
{
  XORG_TESTCASE("Fork() and calling Start() executes the process");
  Process p;
  if (p.Fork() == 0) {
    ASSERT_EQ(p.GetState(), Process::RUNNING);
    p.Start("ls", NULL);
    ASSERT_GT(p.Pid(), 0);
  }
}

TEST(Process, ForkedChildDoubleStart)
{
  XORG_TESTCASE("Fork() and calling Start() twice causes an exception");
  Process p;
  if (p.Fork() == 0) {
    ASSERT_EQ(p.GetState(), Process::RUNNING);
    p.Start("ls", NULL);
    ASSERT_THROW({
        p.Start("ls", NULL);
    }, std::runtime_error);
  }
}

class ProcessValgrindWrapper : public ::testing::Test,
                               public ::testing::WithParamInterface<std::string> {
public:
  virtual void SetUp() {
    CheckForValgrind();
  }

  virtual void CheckForValgrind() {
    Process valgrind;

    /* check if valgrind actually exists */
    valgrind.Start("valgrind", "--version", NULL);
    int status;
    ASSERT_EQ(waitpid(valgrind.Pid(), &status, 0), valgrind.Pid());
    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), 0) << "valgrind failed to start\n";
  }
};

TEST_P(ProcessValgrindWrapper, ValgrindWrapper)
{
  XORG_TESTCASE("Use the valgrind wrapper to start valgrind");

  std::string executable = GetParam();

  /* now set the env and fire up valgrind */
  setenv("XORG_GTEST_USE_VALGRIND", executable.c_str(), 1);
  Process p;
  p.Start("ls", NULL);
  unsetenv("XORG_GTEST_USE_VALGRIND");

  /* Check /proc/<pid>/comm to make sure valgrind
     was started. But comm takes a while to update, it's first our binary
     then valgrind, then memcheck-amd64 (or whatever applies)
  */
  char buff[1024] = {0};
  char fname[128];
  sprintf(fname, "/proc/%d/comm", p.Pid());

  do {
    FILE *fp = fopen(fname, "r");
    ASSERT_TRUE(fp);
    fgets(buff, sizeof(buff), fp);
    fclose(fp);
  } while(strstr(buff, program_invocation_short_name));

  if (executable.compare("valgrind") == 0)
    ASSERT_TRUE(strstr(buff, "memcheck") || strstr(buff, "valgrind"));
  else
    ASSERT_TRUE(strstr(buff, executable.c_str()));
}

class ProcessValgrindArgsWrapper : public ProcessValgrindWrapper {};

TEST_P(ProcessValgrindArgsWrapper, ValgrindWrapperWithArgs)
{
  XORG_TESTCASE("Use the valgrind wrapper with additional args to start valgrind");

  std::string vargs = GetParam();
  std::vector<std::string> valgrind_args;
  char *all_args = strdup(vargs.c_str());
  char *tok = strtok(all_args, " ");
  while(tok) {
    valgrind_args.push_back(std::string(tok));
    tok = strtok(NULL, " ");
  }
  free(all_args);

  /* now set the env and fire up valgrind */
  setenv("XORG_GTEST_USE_VALGRIND", vargs.c_str(), 1);
  Process p;
  p.Start(TEST_ROOT_DIR "process-test-helper", NULL);
  unsetenv("XORG_GTEST_USE_VALGRIND");

  ASSERT_EQ(p.GetState(), Process::RUNNING);

  char buff[1024] = {0};
  char fname[128];
  sprintf(fname, "/proc/%d/cmdline", p.Pid());

  do {
    FILE *fp = fopen(fname, "r");
    ASSERT_TRUE(fp);
    fgets(buff, sizeof(buff), fp);
    fclose(fp);
  } while(strstr(buff, program_invocation_short_name));

  const char * arg = buff + strlen(buff) + 1;

  /* accommodate the case that valgrind has a shell script wrapper */
  if (0 == strcmp(buff, "/bin/sh")) {
    if (0 == strcmp(arg, "-e")) {
      arg += strlen(arg) + 1;
    }
    arg += strlen(arg) + 1;
  }

  std::vector<std::string>::const_iterator it = valgrind_args.begin();

  it++; /* first one is "valgrind" */

  while(strlen(arg) && it != valgrind_args.end()) {
    ASSERT_EQ(it->compare(arg), 0);
    arg += strlen(arg) + 1;
    it++;
  }

  ASSERT_EQ(it, valgrind_args.end());
  p.Kill(100);
}

INSTANTIATE_TEST_CASE_P(, ProcessValgrindWrapper, ::testing::Values("valgrind", "ls"));
INSTANTIATE_TEST_CASE_P(, ProcessValgrindArgsWrapper,
                        ::testing::Values("valgrind --leak-check=full", "valgrind -q --trace-children=yes", "valgrind "));

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

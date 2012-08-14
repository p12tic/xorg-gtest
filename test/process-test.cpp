#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <gtest/gtest.h>
#include <xorg/gtest/xorg-gtest.h>

using namespace xorg::testing;

TEST(Process, StartWithNULLArg)
{
  SCOPED_TRACE("TESTCASE: invocation of 'ls' with no arguments");
  Process p;
  p.Start("ls", NULL);
  ASSERT_GT(p.Pid(), 0);
}

TEST(Process, StartWithNULLTerminatedArg)
{
  SCOPED_TRACE("TESTCASE: invocation of 'ls' with NULL-terminated argument list");

  Process p;
  p.Start("ls", "-l", NULL);
  ASSERT_GT(p.Pid(), 0);
}

TEST(Process, TerminationSuccess)
{
  SCOPED_TRACE("TESTCASE: invocation of 'echo -n', check for success exit status");

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

TEST(Process, TerminationFailure)
{
  SCOPED_TRACE("TESTCASE: an invalid invocation of 'ls', check for error exit status");
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
  SCOPED_TRACE("TESTCASE: ensure child process dies when parent does");

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


int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

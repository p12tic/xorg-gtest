#include <gtest/gtest.h>
#include <xorg/gtest/xorg-gtest.h>

#ifdef HAVE_EVEMU

using namespace xorg::testing;

TEST(Device, DeviceNode)
{
  XORG_TESTCASE("Device node is /dev/input/eventX");

  xorg::testing::evemu::Device d(TEST_ROOT_DIR "PIXART-USB-OPTICAL-MOUSE.desc");

  ASSERT_EQ(d.GetDeviceNode().compare(0, 16, "/dev/input/event"), 0);
}

TEST(Device, InotifyWait)
{
  XORG_TESTCASE("device node can never be empty.\n"
                "This test may show false positives");

  xorg::testing::evemu::Device d(TEST_ROOT_DIR "PIXART-USB-OPTICAL-MOUSE.desc");

  ASSERT_FALSE(d.GetDeviceNode().empty());
}

#endif

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


#include <gtest/gtest.h>
#include <xorg/gtest/xorg-gtest.h>

#ifdef HAVE_EVEMU
#ifndef BTN_TOOL_QUINTTAP
#define BTN_TOOL_QUINTTAP 0x148
#endif

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

TEST(Device, HasEvent)
{
    XORG_TESTCASE("HasEvent must return the right bits.\n");

    xorg::testing::evemu::Device d(TEST_ROOT_DIR "PIXART-USB-OPTICAL-MOUSE.desc");

    for (int i = ABS_X; i < ABS_MAX; i++)
        ASSERT_FALSE(d.HasEvent(EV_ABS, i)) << "Axis code " << i;

    for (int i = REL_X; i < REL_MAX; i++) {
        if (i == REL_X || i == REL_Y || i == REL_WHEEL)
            ASSERT_TRUE(d.HasEvent(EV_REL, i)) << "Axis code " << i;
        else
            ASSERT_FALSE(d.HasEvent(EV_REL, i)) << "Axis code " << i;
    }

    for (int i = BTN_LEFT; i < KEY_MAX; i++) {
        if (i == BTN_LEFT || i == BTN_RIGHT || i == BTN_MIDDLE)
            ASSERT_TRUE(d.HasEvent(EV_KEY, i)) << "Axis code " << i;
        else
            ASSERT_FALSE(d.HasEvent(EV_KEY, i)) << "Axis code " << i;
    }

    xorg::testing::evemu::Device d2(TEST_ROOT_DIR "SynPS2-Synaptics-TouchPad.desc");

    for (int i = REL_X; i < REL_MAX; i++)
        ASSERT_FALSE(d2.HasEvent(EV_REL, i)) << "Axis code " << i;

    for (int i = ABS_X; i < ABS_MAX; i++) {
        switch (i) {
            case ABS_X:
            case ABS_Y:
            case ABS_PRESSURE:
            case ABS_TOOL_WIDTH:
            case ABS_MT_SLOT:
            case ABS_MT_POSITION_X:
            case ABS_MT_POSITION_Y:
            case ABS_MT_TRACKING_ID:
            case ABS_MT_PRESSURE:
                ASSERT_TRUE(d2.HasEvent(EV_ABS, i)) << "Axis code " << i;
                break;
            default:
                ASSERT_FALSE(d2.HasEvent(EV_REL, i)) << "Axis code " << i;
                break;

        }
    }

    for (int i = BTN_LEFT; i < KEY_MAX; i++) {
        switch (i) {
            case BTN_LEFT:
            case BTN_TOOL_FINGER:
            case BTN_TOOL_QUINTTAP:
            case BTN_TOUCH:
            case BTN_TOOL_DOUBLETAP:
            case BTN_TOOL_TRIPLETAP:
            case BTN_TOOL_QUADTAP:
                ASSERT_TRUE(d2.HasEvent(EV_KEY, i)) << "Axis code " << i;
                break;
            default:
                ASSERT_FALSE(d2.HasEvent(EV_KEY, i)) << "Axis code " << i;
        }
    }

}

TEST(Device, AbsAxisData) {
    XORG_TESTCASE("GetAbsData must return the right values.\n");

    xorg::testing::evemu::Device d(TEST_ROOT_DIR "SynPS2-Synaptics-TouchPad.desc");

    const int DEFAULT = -3;

    for (int i = ABS_X; i < ABS_MAX; i++) {
        int min = 0, max = 0, fuzz = 0, flat = 0, resolution = 0;
        bool axis_exists = 1;

        switch (i) {
            case ABS_X:
            case ABS_MT_POSITION_X:
                min = 1472;
                max = 5472;
                fuzz = 8;
                break;
            case ABS_Y:
            case ABS_MT_POSITION_Y:
                min = 1408;
                max = 4448;
                fuzz = 8;
                break;
            case ABS_PRESSURE:
                min = 0;
                max = 255;
                break;
            case ABS_TOOL_WIDTH:
                min = 0;
                max = 15;
                break;
            case ABS_MT_SLOT:
                min = 0;
                max = 1;
                break;
            case ABS_MT_TRACKING_ID:
                min = 0;
                max = 65535;
                break;
            case ABS_MT_PRESSURE:
                min = 0;
                max = 255;
                break;
            default:
                axis_exists = false;
                break;
        }

        if (axis_exists) {
            int min_ret = min;
            int max_ret = max;
            int fuzz_ret = fuzz;
            int flat_ret = flat;
            int res_ret = resolution;

            ASSERT_TRUE(d.GetAbsData(i, &min_ret, &max_ret)) << "Axis code " << i;
            ASSERT_EQ(min, min_ret);
            ASSERT_EQ(max, max_ret);

            ASSERT_TRUE(d.GetAbsData(i, &min_ret, &max_ret, &fuzz)) << "Axis code " << i;
            ASSERT_EQ(min, min_ret);
            ASSERT_EQ(max, max_ret);
            ASSERT_EQ(fuzz, fuzz_ret);

            ASSERT_TRUE(d.GetAbsData(i, &min_ret, &max_ret, &fuzz, &flat)) << "Axis code " << i;
            ASSERT_EQ(min, min_ret);
            ASSERT_EQ(max, max_ret);
            ASSERT_EQ(fuzz, fuzz_ret);
            ASSERT_EQ(flat, flat_ret);

            ASSERT_TRUE(d.GetAbsData(i, &min_ret, &max_ret, &fuzz, &flat, &res_ret)) << "Axis code " << i;
            ASSERT_EQ(min, min_ret);
            ASSERT_EQ(max, max_ret);
            ASSERT_EQ(fuzz, fuzz_ret);
            ASSERT_EQ(flat, flat_ret);
            ASSERT_EQ(res_ret, resolution);
        } else {
            min = DEFAULT;
            max = DEFAULT;
            fuzz = DEFAULT;
            flat = DEFAULT;
            resolution = DEFAULT;

            ASSERT_FALSE(d.GetAbsData(i, &min, &max)) << "Axis code " << i;

            /* make sure we didn't overwrite the values */
            ASSERT_EQ(min, DEFAULT);
            ASSERT_EQ(max, DEFAULT);
            ASSERT_EQ(fuzz, DEFAULT);
            ASSERT_EQ(flat, DEFAULT);
            ASSERT_EQ(resolution, DEFAULT);
        }

    }
}


#endif

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}


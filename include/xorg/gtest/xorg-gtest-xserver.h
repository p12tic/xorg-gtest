/*******************************************************************************
 *
 * X testing environment - Google Test helper class to communicate with the
 * server
 *
 * Copyright (C) 2012 Red Hat, Inc.
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


#ifndef XORG_GTEST_XSERVER_H
#define XORG_GTEST_XSERVER_H

#include <gtest/gtest.h>
#include <xorg/gtest/xorg-gtest.h>
#include <X11/Xlib.h>

namespace xorg {
namespace testing {

/**
 * @class XServer xorg-gtest-xserver.h xorg/gtest/xorg-gtest-xserver.h
 *
 * Class representing the X server process.
 *
 * @code
 * XServer server;
 * server.SetOption("-logfile", "/tmp/Xserver.log");
 * server.Start();
 *
 * ...
 *
 * if (!server.Terminate()) {
 *   std::cerr << "Problem terminating server ... killing now ..." << std::endl;
 *   if (!server.Kill())
 *     std::cerr << "Problem killing server" << std::endl;
 * }
 * @endcode
 */
class XServer : public xorg::testing::Process {
  public:
    XServer();
    ~XServer();

    /**
     * Start a new server. If no binary is given, the server started is the
     * default compiled-in server binary.
     *
     * @param [in] program Path to the XServer binary
     */
    void Start(const std::string &program = "");

    /**
     * Terminates this server process. Will signal the server to terminate
     * multiple times before giving up.
     *
     * @param [in] timeout The timeout in millis to wait for the process to
     *                     terminate. A timeout of 0 implies not to wait but
     *                     return immediately.
     *
     * @returns true if termination succeeded and, if a timout is given, the
     *          process shut down within that timeout. false otherwise.
     */
    virtual bool Terminate(unsigned int timeout = 1000);

    /**
     * Kills the server. With a vengeance.
     *
     * @param [in] timeout The timeout in millis to wait for the process to
     *                     terminate. A timeout of 0 implies not to wait but
     *                     return immediately.
     *
     * @returns true if kill succeeded and, if a timout is given, the
     *          process shut down within that timeout. false otherwise.
     */
    virtual bool Kill(unsigned int timeout = 1000);

    /**
     * Remove the log file used by this server. By default, this function
     * only removes the log file if the server was terminated or finished
     * with an exit code of 0.
     *
     * If force is true, the log file is removed regardless of the state of
     * the server.
     *
     * @param force Force removal of the log file
     */
    void RemoveLogFile(bool force = false);

    /**
     * Waits until this server is ready to take connections.
     */
    _X_DEPRECATED void WaitForConnections(void);

    /**
     * Set the display number for this server. This number must be set
     * before the server is started to have any effect.
     * If unset, the default display number is used.
     *
     * @param [in] display_number The display number the server runs on
     */
    void SetDisplayNumber(unsigned int display_number);

    /**
     * Set the path to the server binary to be started. Optional call, if
     * not invoked the built-in default path is chosen.
     *
     * @param [in] path_to_server The path to the binary
     */
    void SetServerPath(const std::string &path_to_server);

    /**
     * Get the display number from this server. If the server was not
     * started yet, this function returns the display number the server will
     * be started on.
     *
     * @return The numeric display number this server runs on
     */
    unsigned int GetDisplayNumber(void);

    /**
     * Get the display string that may be used for XOpenDisplay to this
     * server. This string is effectively :display_number.
     *
     * @return The display string used for XOpenDisplay() to this server.
     */
    const std::string& GetDisplayString(void);

    /**
     * Get the X server version as printed into the log file, usually in the
     * form a.b.c[.d], with d being the optional part for release
     * candidates.
     *
     * @return A string representing this server's version. If the server
     *         hasn't been started yet, GetVersion() returns an empty string.
     */
    const std::string& GetVersion();

    /**
     * Get the server's log file path. If this path is empty, the server
     * will use it's built-in log file path.
     *
     * @return The log file path this server will use, is using or has used.
     */
    const std::string& GetLogFilePath();

    /**
     * Get the server's config file path. If this path is empty, the server
     * will use it's built-in config file path.
     *
     * @return The config file path this server will use, is using or has used.
     */
    const std::string& GetConfigPath();

    /**
     * Set startup options for the server.
     *
     * For arguments that do not take/need a value, use the empty string as
     * value.
     *
     * @param [in] key Commandline option
     * @param [in] value Option value (if any)
     */
    void SetOption(const std::string &key, const std::string &value);

    /**
     * Wait for a specific device to be added to the server.
     *
     * @param [in] display The X display connection
     * @param [in] name    The name of the device to wait for
     * @param [in] timeout The timeout in milliseconds
     *
     * @return Whether the device was added
     */
    static bool WaitForDevice(::Display *display, const std::string &name, time_t timeout = 1000);

    /**
     * Wait for an event on the X connection.
     *
     * @param [in] display The X display connection
     * @param [in] timeout The timeout in milliseconds
     *
     * @return Whether an event is available
     */
    static bool WaitForEvent(::Display *display, time_t timeout = 1000);

    /**
     * Wait for an event of a specific type on the X connection.
     *
     * All events preceding the matching event are discarded. If no event was found
     * before the timeout expires, all events in the queue will have been discarded.
     *
     * @param [in] display   The X display connection
     * @param [in] type      The X core protocol event type
     * @param [in] extension The X extension opcode of a generic event, or -1 for
     *                       any generic event
     * @param [in] evtype    The X extension event type of a generic event, or -1
     *                       for any event of the given extension
     * @param [in] timeout   The timeout in milliseconds
     *
     * @return Whether an event is available
     */
    static bool WaitForEventOfType(::Display *display, int type, int extension, int evtype, time_t timeout = 1000);

  private:
    struct Private;
    std::auto_ptr<Private> d_;

    /* Disable copy constructor, assignment operator */
    XServer(const XServer&);
    XServer& operator=(const XServer&);

    void TestStartup(void);

};
} // namespace testing
} // namespace xorg

#endif /* XORG_GTEST_XSERVER_H */

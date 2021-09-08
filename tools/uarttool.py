#!/usr/bin/env python
# SPDX-License-Identifier: Apache-2.0
#
# Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
# Copyright 2021 Jeff Kent <jeff@jkent.net>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Contains elements taken from miniterm "Very simple serial terminal" which
# is part of pySerial. https://github.com/pyserial/pyserial
# (C)2002-2015 Chris Liechti <cliechti@gmx.net>
#
# Originally released under BSD-3-Clause license.

from __future__ import division, print_function, unicode_literals

import argparse
import codecs
import os
import subprocess
from builtins import object

try:
    import queue
except ImportError:
    import Queue as queue

import shlex
import sys
import textwrap
import threading
import time
from io import open

import serial
import serial.tools.list_ports
import serial.tools.miniterm as miniterm

key_description = miniterm.key_description

# Control-key characters
CTRL_E = '\x05'
CTRL_H = '\x08'
CTRL_T = '\x14'
CTRL_Y = '\x19'
CTRL_X = '\x18'
CTRL_RBRACKET = '\x1d'  # Ctrl+]

# Command parsed from console inputs
CMD_STOP = 1
CMD_OUTPUT_TOGGLE = 2
CMD_EXECUTE = 3

# ANSI terminal codes (if changed, regular expressions in LineMatcher need to
# be udpated)
ANSI_RED = '\033[1;31m'
ANSI_YELLOW = '\033[0;33m'
ANSI_NORMAL = '\033[0m'


def color_print(message, color, newline='\n'):
    """ Print a message to stderr with colored highlighting """
    sys.stderr.write('%s%s%s%s' % (color, message,  ANSI_NORMAL, newline))


def yellow_print(message, newline='\n'):
    color_print(message, ANSI_YELLOW, newline)


def red_print(message, newline='\n'):
    color_print(message, ANSI_RED, newline)


# Tags for tuples in queues
TAG_KEY = 0
TAG_SERIAL = 1
TAG_SERIAL_FLUSH = 2
TAG_CMD = 3

# loader related messages
LOADER_UART_READY = b'BL1 UART Loader READY'
LOADER_CHECKSUM_FAIL = b'BL1 CHECKSUM FAIL'
LOADER_CHECKSUM_OK = b'BL1 CHECKSUM OK'

class StoppableThread(object):
    """
    Provide a Thread-like class which can be 'cancelled' via a subclass-
    provided cancellation method.

    Can be started and stopped multiple times.

    Isn't an instance of type Thread because Python Thread objects can only be
    run once
    """
    def __init__(self):
        self._thread = None

    @property
    def alive(self):
        """
        Is 'alive' whenever the internal thread object exists
        """
        return self._thread is not None

    def start(self):
        if self._thread is None:
            self._thread = threading.Thread(target=self._run_outer)
            self._thread.start()

    def _cancel(self):
        pass  # override to provide cancellation functionality

    def run(self):
        pass  # override for the main thread behaviour

    def _run_outer(self):
        try:
            self.run()
        finally:
            self._thread = None

    def stop(self):
        if self._thread is not None:
            old_thread = self._thread
            self._thread = None
            self._cancel()
            old_thread.join()


class ConsoleReader(StoppableThread):
    """ Read input keys from the console and push them to the queue,
    until stopped.
    """
    def __init__(self, console, event_queue, cmd_queue, parser):
        super(ConsoleReader, self).__init__()
        self.console = console
        self.event_queue = event_queue
        self.cmd_queue = cmd_queue
        self.parser = parser

    def run(self):
        self.console.setup()
        try:
            while self.alive:
                try:
                    c = self.console.getkey()
                except KeyboardInterrupt:
                    c = '\x03'
                if c is not None:
                    ret = self.parser.parse(c)
                    if ret is not None:
                        (tag, cmd) = ret
                        # stop command should be executed last
                        if tag == TAG_CMD and cmd != CMD_STOP:
                            self.cmd_queue.put(ret)
                        else:
                            self.event_queue.put(ret)

        finally:
            self.console.cleanup()

    def _cancel(self):
        # this is the way cancel() is implemented in pyserial 3.3 or newer,
        # older pyserial (3.1+) has cancellation implemented via 'select',
        # which does not work when console sends an escape sequence response
        #
        # even older pyserial (<3.1) does not have this method
        #
        # on Windows there is a different (also hacky) fix, applied above.
        #
        # note that TIOCSTI is not implemented in WSL / bash-on-Windows.
        # TODO: introduce some workaround to make it work there.
        #
        # Note: This would throw exception in testing mode when the stdin is
        # connected to PTY.
        import fcntl
        import termios
        fcntl.ioctl(self.console.fd, termios.TIOCSTI, b'\0')


class ConsoleParser(object):

    def __init__(self, eol='CRLF'):
        self.translate_eol = {
            'CRLF': lambda c: c.replace('\n', '\r\n'),
            'CR': lambda c: c.replace('\n', '\r'),
            'LF': lambda c: c.replace('\r', '\n'),
        }[eol]
        self.menu_key = CTRL_T
        self.exec_key = CTRL_E
        self.exit_key = CTRL_RBRACKET
        self._pressed_menu_key = False

    def parse(self, key):
        ret = None
        if self._pressed_menu_key:
            ret = self._handle_menu_key(key)
        elif key == self.menu_key:
            self._pressed_menu_key = True
        elif key == self.exec_key:
            ret = (TAG_CMD, CMD_EXECUTE)
        elif key == self.exit_key:
            ret = (TAG_CMD, CMD_STOP)
        else:
            key = self.translate_eol(key)
            ret = (TAG_KEY, key)
        return ret

    def _handle_menu_key(self, c):
        ret = None
        if c == self.exit_key or c == self.menu_key:  # send verbatim
            ret = (TAG_KEY, c)
        elif c in [CTRL_E, 'e', 'E']:
            ret = (TAG_CMD, CMD_EXECUTE)
        elif c in [CTRL_H, 'h', 'H', '?']:
            red_print(self.get_help_text())
        elif c == CTRL_Y:  # Toggle output display
            ret = (TAG_CMD, CMD_OUTPUT_TOGGLE)
        elif c in [CTRL_X, 'x', 'X']:  # Exiting from within the menu
            ret = (TAG_CMD, CMD_STOP)
        else:
            red_print('--- unknown menu character {} ---'
                    .format(key_description(c)))

        self._pressed_menu_key = False
        return ret

    def get_help_text(self):
        text = """\
            --- loader.py - BL2 uart loading tool
            --- based on idf_monitor from esp-idf
            ---
            --- {exit:8} Exit program
            --- {menu:8} Menu escape key, followed by:
            --- Menu keys:
            ---    {menu:14} Send the menu character itself to remote
            ---    {exit:14} Send the exit character itself to remote
            ---    {output:14} Toggle output display
            ---    {menuexit:14} Exit program
        """.format(exit=key_description(self.exit_key),
                   menu=key_description(self.menu_key),
                   output=key_description(CTRL_Y),
                   menuexit=key_description(CTRL_X) + ' (or X)')
        return textwrap.dedent(text)

class SerialReader(StoppableThread):
    """ Read serial data from the serial port and push to the
    event queue, until stopped.
    """
    def __init__(self, serial, event_queue):
        super(SerialReader, self).__init__()
        self.baud = serial.baudrate
        self.serial = serial
        self.event_queue = event_queue
        if not hasattr(self.serial, 'cancel_read'):
            # timeout for checking alive flag,
            # if cancel_read not available
            self.serial.timeout = 0.1

    def run(self):
        if not self.serial.is_open:
            self.serial.baudrate = self.baud
            self.serial.open()
        try:
            while self.alive:
                try:
                    data = self.serial.read(self.serial.in_waiting or 1)
                except (serial.serialutil.SerialException, IOError) as e:
                    data = b''
                    # self.serial.open() was successful before, therefore,
                    # this is an issue related to the disappearance of the
                    # device
                    red_print(e)
                    yellow_print('Waiting for the device to reconnect',
                            newline='')
                    self.serial.close()
                    while self.alive:  # so that exiting monitor works while
                                       # waiting
                        try:
                            time.sleep(0.5)
                            self.serial.open()
                            break  # device connected
                        except serial.serialutil.SerialException:
                            yellow_print('.', newline='')
                            sys.stderr.flush()
                    yellow_print('')  # go to new line
                if len(data):
                    self.event_queue.put((TAG_SERIAL, data), False)
        finally:
            self.serial.close()

    def _cancel(self):
        if hasattr(self.serial, 'cancel_read'):
            try:
                self.serial.cancel_read()
            except Exception:
                pass


class SerialStopException(Exception):
    """
    This exception is used for stopping the IDF monitor in testing mode.
    """
    pass


class Monitor(object):
    """
    Monitor application main class.

    This was originally derived from miniterm.Miniterm, but it turned out to
    be easier to write from scratch for this purpose.

    Main difference is that all event processing happens in the main thread,
    not the worker threads.
    """
    def __init__(self, serial_instance, binary, load_addr, no_load, exec_addr,
            no_exec, exec_baud, eol='CRLF'):
        super(Monitor, self).__init__()
        self.event_queue = queue.Queue()
        self.cmd_queue = queue.Queue()
        self.console = miniterm.Console()

        self.serial = serial_instance
        self.binary = binary
        self.load_addr = load_addr
        self.no_load = no_load
        self.exec_addr = exec_addr
        self.no_exec = no_exec
        self.exec_baud = exec_baud
        self.console_parser = ConsoleParser(eol)
        self.console_reader = ConsoleReader(self.console, self.event_queue,
                self.cmd_queue, self.console_parser)
        self.serial_reader = SerialReader(self.serial, self.event_queue)

        # internal state
        self._last_line_part = b''
        self._invoke_processing_last_line_timer = None
        self._force_line_print = False
        self._output_enabled = True
        self._hide_bl1_text = False

    def invoke_processing_last_line(self):
        self.event_queue.put((TAG_SERIAL_FLUSH, b''), False)

    def main_loop(self):
        self.console_reader.start()
        self.serial_reader.start()
        try:
            while self.console_reader.alive and self.serial_reader.alive:
                try:
                    item = self.cmd_queue.get_nowait()
                except queue.Empty:
                    try:
                        item = self.event_queue.get(True, 0.03)
                    except queue.Empty:
                        continue

                (event_tag, data) = item
                if event_tag == TAG_CMD:
                    self.handle_commands(data)
                elif event_tag == TAG_KEY:
                    try:
                        self.serial.write(codecs.encode(data))
                    except serial.SerialException:
                        pass  # this shouldn't happen, but sometimes port has
                              # closed in serial thread
                    except UnicodeEncodeError:
                        pass  # this can happen if a non-ascii character was
                              # passed, ignoring
                elif event_tag == TAG_SERIAL:
                    self.handle_serial_input(data)
                    if self._invoke_processing_last_line_timer is not None:
                        self._invoke_processing_last_line_timer.cancel()
                    self._invoke_processing_last_line_timer = \
                            threading.Timer(0.05,
                            self.invoke_processing_last_line)
                    self._invoke_processing_last_line_timer.start()
                    # If no further data is received in the next short period
                    # of time then the _invoke_processing_last_line_timer
                    # generates an event which will result in the finishing of
                    # the last line. This is fix for handling lines sent
                    # without EOL.
                elif event_tag == TAG_SERIAL_FLUSH:
                    self.handle_serial_input(data, finalize_line=True)
                else:
                    raise RuntimeError('Bad event data %r' %
                            ((event_tag,data),))
        except SerialStopException:
            sys.stderr.write(ANSI_NORMAL + 'Stopping condition has been '
                    'received\n')
        finally:
            try:
                self.console_reader.stop()
                self.serial_reader.stop()
                # Cancelling _invoke_processing_last_line_timer is not
                # important here because receiving empty data doesn't matter.
                self._invoke_processing_last_line_timer = None
            except Exception:
                pass
            sys.stderr.write(ANSI_NORMAL + '\n')

    def handle_serial_input(self, data, finalize_line=False):
        data = self._last_line_part + data
        while b'\n' in data:
            line, data = data.split(b'\n', 1)
            self.check_loader_trigger_before_print(line)
            self.check_checksum_before_print(line)
            if not self._hide_bl1_text:
                self._print(line + b'\n')
            self._hide_bl1_text = False

        if data != b'' and finalize_line:
            self._print(data)
            data = b''

        self._last_line_part = data

    def __enter__(self):
        """ Use 'with self' to temporarily disable monitoring behaviour """
        self.serial_reader.stop()
        self.console_reader.stop()

    def __exit__(self, *args, **kwargs):
        """ Use 'with self' to temporarily disable monitoring behaviour """
        self.console_reader.start()
        self.serial_reader.start()

    def load(self, binary, addr=0x20000000):
        if not os.path.exists(binary):
            yellow_print("binary not found")
            return
        time.sleep(0.05)

        checksum = 0
        with open(binary, 'rb') as f:
            f.seek(0, os.SEEK_END)
            length = f.tell()
            f.seek(0, os.SEEK_SET)

            yellow_print('loading %s (0x%08x) @ 0x%08x' % (binary, length,
                    addr))
            self.serial.write(b'load 0x%08x @ 0x%08x\n' % (length, addr))

            time.sleep(0.05)

            chunk = f.read(1024)
            while chunk:
                for byte in chunk:
                    checksum += byte
                self.serial.write(chunk)
                chunk = f.read(1024)

        checksum &= 0xFFFFFFFF
        self.serial.write(b'check 0x%08x\n' % (checksum,))

    def execute(self, addr):
        yellow_print('execute 0x%08x' % (addr,))
        self.serial.write(b'exec 0x%08x\n' % (addr,))
        self.serial.flush()
        if self.exec_baud is not None:
            self.serial.baudrate = self.exec_baud

    def check_loader_trigger_before_print(self, line):
        if self.binary and LOADER_UART_READY in line:
            self.output_enable(True)
            if self.binary and not self.no_load:
                self.load(self.binary, self.load_addr)
            self._hide_bl1_text = True

    def check_checksum_before_print(self, line):
        if LOADER_CHECKSUM_OK in line:
            yellow_print('load complete, checksum OK')
            if self.exec_addr is not None and not self.no_exec:
                self.execute(self.exec_addr)
            self._hide_bl1_text = True
        elif LOADER_CHECKSUM_FAIL in line:
            red_print('load complete, checksum FAIL')
            self.output_enable(True)
            self._hide_bl1_text = True

    def output_enable(self, enable):
        self._output_enabled = enable

    def output_toggle(self):
        self._output_enabled = not self._output_enabled
        yellow_print('\nToggle output display: {}, Type Ctrl-T Ctrl-Y to '
                'show/disable output again.'.format(self._output_enabled))

    def _print(self, string, console_printer=None):
        if console_printer is None:
            console_printer = self.console.write_bytes
        if self._output_enabled:
            console_printer(string)

    def handle_commands(self, cmd):
        if cmd == CMD_STOP:
            self.console_reader.stop()
            self.serial_reader.stop()
        elif cmd == CMD_OUTPUT_TOGGLE:
            self.output_toggle()
        elif cmd == CMD_EXECUTE:
            self.execute(self.exec_addr)
        else:
            raise RuntimeError('Bad command data %d' % (cmd))


def main():
    parser = argparse.ArgumentParser('loader - a uart serial loader')

    parser.add_argument(
        '--port', '-p',
        help='Serial port device',
        default=os.environ.get('MONITORPORT', '/dev/ttyUSB0')
    )

    parser.add_argument(
        '--baud', '-b',
        help='Serial port baud rate',
        type=int,
        default=os.getenv('MONITORBAUD', 460800))

    parser.add_argument(
        '--make', '-m',
        help='Command to run make',
        type=str, default='make')

    parser.add_argument(
        '--eol',
        choices=['CR', 'LF', 'CRLF'],
        type=lambda c: c.upper(),
        help='End of line to use when sending to the serial port',
        default='CR')

    parser.add_argument(
        '--load-addr', help='Load address, defaults to 0x20000000',
        type=lambda x: int(x, 0), default=0x20000000)

    parser.add_argument(
        '--no-load',
        action='store_true',
        help='Don\'t automatically load binary',
        default=False)

    parser.add_argument(
        '--exec-addr', help='Exec address, defaults to --load-addr',
        type=lambda x: int(x, 0), default=None)

    parser.add_argument(
        '--no-exec',
        action='store_true',
        help='Don\'t automaticaly execute binary',
        default=False)

    parser.add_argument(
        '--exec-baud', help='Baudrate to switch to after exec',
        type=int, default=None)

    parser.add_argument(
        'BINARY', nargs='?', help='BL2 binary, exec only if not provided',
        type=str, default=None)

    args = parser.parse_args()

    if args.exec_addr is None:
        args.exec_addr = args.load_addr

    serial_instance = serial.serial_for_url(args.port, args.baud,
                                            do_not_open=True)
    monitor = Monitor(serial_instance, args.BINARY, args.load_addr,
            args.no_load, args.exec_addr, args.no_exec, args.exec_baud,
            args.eol)

    yellow_print('--- idf_monitor on {p.name} {p.baudrate} ---'.format(
        p=serial_instance))
    yellow_print('--- Quit: {} | Menu: {} | Help: {} followed by {} '
            '---'.format(
        key_description(monitor.console_parser.exit_key),
        key_description(monitor.console_parser.menu_key),
        key_description(monitor.console_parser.menu_key),
        key_description(CTRL_H)))

    monitor.main_loop()

if __name__ == '__main__':
    main()

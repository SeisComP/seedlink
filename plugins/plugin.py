#!/usr/bin/env python3
# -*- coding: utf-8 -*-
############################################################################
# Copyright (C) GFZ Potsdam                                                #
# All rights reserved.                                                     #
#                                                                          #
# GNU Affero General Public License Usage                                  #
# This file may be used under the terms of the GNU Affero                  #
# Public License version 3.0 as published by the Free Software Foundation  #
# and appearing in the file LICENSE included in the packaging of this      #
# file. Please review the following information to ensure the GNU Affero   #
# Public License version 3.0 requirements will be met:                     #
# https://www.gnu.org/licenses/agpl-3.0.html.                              #
############################################################################

import sys
import os
import struct


_doy = (0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365)


def _is_leap(y):
    """True if y is a leap year."""
    return (y % 400 == 0) or (y % 4 == 0 and y % 100 != 0)


def _ldoy(y, m):
    """The day of the year of the first day of month m, in year y.

    Note: for January, m=1; for December, m=12.
    Examples:
    _ldoy(1900, 4) = 90
    _ldoy(1900, 1) = 0
    _ldoy(1999, 4) = 90
    _ldoy(2004, 4) = 91
    _ldoy(2000, 4) = 91

    """
    return _doy[m - 1] + (_is_leap(y) and m >= 3)


def _mdy2dy(month, day, year):
    return _ldoy(year, month) + day


class Seedlink(object):
    def __init__(self):
        self.__fd = os.fdopen(63, "wb")

    def send_raw3(self, sta, cha, t, usec_corr, tqual, data):
        packet = struct.pack(
            "@i10s10s9i%di" % len(data),
            8,
            sta.encode(),
            cha.encode(),
            t.year,
            _mdy2dy(t.month, t.day, t.year),
            t.hour,
            t.minute,
            t.second,
            t.microsecond,
            usec_corr,
            tqual,
            len(data),
            *data
        )

        self.__fd.write(packet)
        self.__fd.flush()


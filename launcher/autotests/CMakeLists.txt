# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

ecm_add_test(patchlisttest.cpp
    TEST_NAME patchlisttest
    LINK_LIBRARIES astra_static Qt::Test
    NAME_PREFIX "astra-"
)
# SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
# SPDX-License-Identifier: CC0-1.0

ecm_add_test(accountmanagertest.cpp
        TEST_NAME accountmanagertest
        LINK_LIBRARIES astra_static Qt::Test
        NAME_PREFIX "astra-"
)

ecm_add_test(processwatchertest.cpp
        TEST_NAME processwatchertest
        LINK_LIBRARIES astra_static Qt::Test
        NAME_PREFIX "astra-"
)

ecm_add_test(profilemanagertest.cpp
        TEST_NAME profilemanagertest
        LINK_LIBRARIES astra_static Qt::Test
        NAME_PREFIX "astra-"
)

ecm_add_test(utilitytest.cpp
        TEST_NAME utilitytest
        LINK_LIBRARIES astra_static Qt::Test
        NAME_PREFIX "astra-"
)

ecm_add_test(crtrandtest.cpp
        TEST_NAME crtrandtest
        LINK_LIBRARIES astra_static Qt::Test
        NAME_PREFIX "astra-"
)

ecm_add_test(encryptedargtest.cpp
        TEST_NAME encryptedargtest
        LINK_LIBRARIES astra_static Qt::Test
        NAME_PREFIX "astra-"
)

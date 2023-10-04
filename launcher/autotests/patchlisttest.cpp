// SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>

#include "patchlist.h"

class PatchListTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testBootPatchList()
    {
        const QString testCase{
            QStringLiteral("--477D80B1_38BC_41d4_8B48_5273ADB89CAC\r\nContent-Type: application/octet-stream\r\nContent-Location: "
                           "ffxivpatch/2b5cbc63/metainfo/D2023.04.28.0000.0001.http\r\nX-Patch-Length: "
                           "22221335\r\n\r\n22221335\t69674819\t19\t18\t2023.09.14.0000.0001\thttp://patch-dl.ffxiv.com/boot/2b5cbc63/"
                           "D2023.09.14.0000.0001.patch\r\n--477D80B1_38BC_41d4_8B48_5273ADB89CAC--\r\n")};

        const PatchList patchList{testCase};

        QCOMPARE(patchList.patches().size(), 1);
        QCOMPARE(patchList.patches()[0].version, QStringLiteral("2023.09.14.0000.0001"));
        QCOMPARE(patchList.patches()[0].url, QStringLiteral("http://patch-dl.ffxiv.com/boot/2b5cbc63/D2023.09.14.0000.0001.patch"));
    }

    void testGamePatchList()
    {
        const QString testCase{QStringLiteral(
            "--477D80B1_38BC_41d4_8B48_5273ADB89CAC\r\nContent-Type: application/octet-stream\r\nContent-Location: "
            "ffxivpatch/4e9a232b/metainfo/2023.07.26.0000.0000.http\r\nX-Patch-Length: "
            "1664916486\r\n\r\n1479062470\t44145529682\t71\t11\t2023.09.15.0000.0000\tsha1\t50000000\t1c66becde2a8cf26a99d0fc7c06f15f8bab2d87c,"
            "950725418366c965d824228bf20f0496f81e0b9a,cabef48f7bf00fbf18b72843bdae2f61582ad264,53608de567b52f5fdb43fdb8b623156317e26704,"
            "f0bc06cabf9ff6490f36114b25f62619d594dbe8,3c5e4b962cd8445bd9ee29011ecdb331d108abd8,88e1a2a322f09de3dc28173d4130a2829950d4e0,"
            "1040667917dc99b9215dfccff0e458c2e8a724a8,149c7e20e9e3e376377a130e0526b35fd7f43df2,1bb4e33807355cdf46af93ce828b6e145a9a8795,"
            "a79daff43db488f087da8e22bb4c21fd3a390f3c,6b04fadb656d467fb8318eba1c7f5ee8f030d967,a6641e1c894db961a49b70fda2b0d6d87be487a7,"
            "edf419de49f42ef19bd6814f8184b35a25e9e977,c1525c4df6001b66b575e2891db0284dc3a16566,01b7628095b07fa3c9c1aed2d66d32d118020321,"
            "991b137ea0ebb11bd668f82149bc2392a4cbcf52,ad3f74d4fca143a6cf507fc859544a4bcd501d85,936a0f1711e273519cae6b2da0d8b435fe6aa020,"
            "023f19d8d8b3ecaaf865e3170e8243dd437a384c,2d9e934de152956961a849e81912ca8d848265ca,8e32f9aa76c95c60a9dbe0967aee5792b812d5ec,"
            "dee052b9aa1cc8863efd61afc63ac3c2d56f9acc,fa81225aea53fa13a9bae1e8e02dea07de6d7052,59b24693b1b62ea1660bc6f96a61f7d41b3f7878,"
            "349b691db1853f6c0120a8e66093c763ba6e3671,4561eb6f954d80cdb1ece3cc4d58cbd864bf2b50,de94175c4db39a11d5334aefc7a99434eea8e4f9,"
            "55dd7215f24441d6e47d1f9b32cebdb041f2157f,2ca09db645cfeefa41a04251dfcb13587418347a\thttp://patch-dl.ffxiv.com/game/4e9a232b/"
            "D2023.09.15.0000.0000.patch\r\n61259063\t44145955874\t71\t11\t2023.09.21.0000.0001\tsha1\t50000000\t88c9bbfe2af4eea7b56384baeeafd59afb47ddeb,"
            "095c26e87b4d25505845515c389dd22dd429ea7e\thttp://patch-dl.ffxiv.com/game/4e9a232b/"
            "D2023.09.21.0000.0001.patch\r\n63776300\t44146911186\t71\t11\t2023.09.23.0000.0001\tsha1\t50000000\tc8fc6910be12d10b39e4e6ae980d4c219cfe56a1,"
            "5c0199b7147a47f620a2b50654a87c9b0cbcf43b\thttp://patch-dl.ffxiv.com/game/4e9a232b/"
            "D2023.09.23.0000.0001.patch\r\n32384649\t44146977234\t71\t11\t2023.09.26.0000."
            "0000\tsha1\t50000000\t519a5e46edb67ba6edb9871df5eb3991276da254\thttp://patch-dl.ffxiv.com/game/4e9a232b/"
            "D2023.09.26.0000.0000.patch\r\n28434004\t44147154898\t71\t11\t2023.09.28.0000."
            "0000\tsha1\t50000000\ta08e8a071b615b0babc45a09979ab6bc70affe14\thttp://patch-dl.ffxiv.com/game/4e9a232b/"
            "D2023.09.28.0000.0000.patch\r\n82378953\t5854598228\t30\t4\t2023.07.26.0000.0001\tsha1\t50000000\t07d9fecb3975028fdf81166aa5a4cca48bc5a4b0,"
            "c10677985f809df93a739ed7b244d90d37456353\thttp://patch-dl.ffxiv.com/game/ex1/6b936f08/"
            "D2023.07.26.0000.0001.patch\r\n29384945\t5855426508\t30\t4\t2023.09.23.0000.0001\tsha1\t50000000\tcf4970957e846e6cacfdad521252de18afc7e29b\thttp:/"
            "/patch-dl.ffxiv.com/game/ex1/6b936f08/"
            "D2023.09.23.0000.0001.patch\r\n136864\t5855426508\t30\t4\t2023.09.26.0000.0000\tsha1\t50000000\t3ca5e0160e1cedb7a2801048408b247095f432ea\thttp://"
            "patch-dl.ffxiv.com/game/ex1/6b936f08/"
            "D2023.09.26.0000.0000.patch\r\n126288\t5855426508\t30\t4\t2023.09.28.0000.0000\tsha1\t50000000\t88d201defb32366004c88b236d03278f95d9b442\thttp://"
            "patch-dl.ffxiv.com/game/ex1/6b936f08/"
            "D2023.09.28.0000.0000.patch\r\n49352444\t7620831756\t24\t4\t2023.09.23.0000.0001\tsha1\t50000000\t2a05a452281d119241f222f4eae43266a22560fe\thttp:/"
            "/patch-dl.ffxiv.com/game/ex2/f29a3eb2/"
            "D2023.09.23.0000.0001.patch\r\n301600\t7620831756\t24\t4\t2023.09.26.0000.0000\tsha1\t50000000\t215de572fe51bca45f83e19d719f52220818bc39\thttp://"
            "patch-dl.ffxiv.com/game/ex2/f29a3eb2/"
            "D2023.09.26.0000.0000.patch\r\n385096\t7620831756\t24\t4\t2023.09.28.0000.0000\tsha1\t50000000\t427c3fd61f2ca79698ecd6dc34e3457f6d8c01cd\thttp://"
            "patch-dl.ffxiv.com/game/ex2/f29a3eb2/"
            "D2023.09.28.0000.0000.patch\r\n60799419\t9737248724\t26\t4\t2023.09.23.0000.0001\tsha1\t50000000\tab064df7aec0526e8306a78e51adbbba8b129c3f,"
            "4e1bb58a987b3157c16f59821bc67b1464a301e5\thttp://patch-dl.ffxiv.com/game/ex3/859d0e24/"
            "D2023.09.23.0000.0001.patch\r\n555712\t9737248724\t26\t4\t2023.09.26.0000.0000\tsha1\t50000000\tdb6b1f34b0b58ca0d0621ff6cebcc63e7eb692c5\thttp://"
            "patch-dl.ffxiv.com/game/ex3/859d0e24/"
            "D2023.09.26.0000.0000.patch\r\n579560\t9737248724\t26\t4\t2023.09.28.0000.0000\tsha1\t50000000\t67b5d62ee8202fe045c763deea38c136c5324195\thttp://"
            "patch-dl.ffxiv.com/game/ex3/859d0e24/"
            "D2023.09.28.0000.0000.patch\r\n867821466\t12469514712\t40\t4\t2023.09.15.0000.0001\tsha1\t50000000\t08f6164685b4363d719d09a8d0ef0910b48ec4a1,"
            "05819ea5182885b59f0dfb981ecab159f46d7343,6ab6106ce4153cac5edb26ad0aabc6ba718ee500,3c552f54cc3c101d9f1786156c1cbd9880a7c02f,"
            "e0d425f6032da1ceb60ff9ca14a10e5e89f23218,245402087bf6d535cb08bbc189647e8f56301722,a3a0630bb4ddd36b532be0e0a8982dbce1bb9053,"
            "eca8a1394db1e84b9ec11a99bd970e6335326da5,40546e6d37cc5ea21d26c2e00a11f46a13d99a77,f41f312ad72ee65dc97645c8f7426c35970187ca,"
            "e5a9966528ecab5a51059de3d5cd83a921c73a1c,c03855127d135d22c65e34e615eddbe6f38769e9,befab30a77c14743f53b20910b564bb6a97dfe86,"
            "dcce8ea707f03606b583d51d947a4cf99b52635e,c4a33a8c51a047706b65887bed804ec6c2c29016,a17bc8bd8709c2a0c5725c134101132d3536e67d,"
            "d2b277de55a65697d80cfe8ee46199a8d7482c30,6b97cc2862c6f8f5d279be5f28cc13ed011763e5\thttp://patch-dl.ffxiv.com/game/ex4/1bf99b87/"
            "D2023.09.15.0000.0001.patch\r\n17717567\t12471821656\t40\t4\t2023.09.23.0000.0001\tsha1\t50000000\tda144d5c1c173ef1d98e4e7b558414ae53bcd392\thttp:"
            "//patch-dl.ffxiv.com/game/ex4/1bf99b87/"
            "D2023.09.23.0000.0001.patch\r\n3117253\t12471859944\t40\t4\t2023.09.26.0000.0000\tsha1\t50000000\t7acdab61e99d69ffa53e9136f65a1a1d3b33732b\thttp:/"
            "/patch-dl.ffxiv.com/game/ex4/1bf99b87/"
            "D2023.09.26.0000.0000.patch\r\n4676853\t12471859944\t40\t4\t2023.09.28.0000.0000\tsha1\t50000000\tce26ccb2115af612ccd4c42c1a27ef2ec925c81e\thttp:/"
            "/patch-dl.ffxiv.com/game/ex4/1bf99b87/D2023.09.28.0000.0000.patch\r\n--477D80B1_38BC_41d4_8B48_5273ADB89CAC--\r\n")};

        const PatchList patchList{testCase};
        QCOMPARE(patchList.patches().size(), 19);
        QCOMPARE(patchList.patches()[5].version, QStringLiteral("2023.07.26.0000.0001"));
        QCOMPARE(patchList.patches()[5].url, QStringLiteral("http://patch-dl.ffxiv.com/game/ex1/6b936f08/D2023.07.26.0000.0001.patch"));
    }
};

QTEST_MAIN(PatchListTest)
#include "patchlisttest.moc"

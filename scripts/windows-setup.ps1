# SPDX-FileCopyrightText: 2024 NotNite <hi@notnite.com>
# SPDX-License-Identifier: CC0-1.0

$ErrorActionPreference = "Stop"
$PSNativeCommandUseErrorActionPreference = $true

$LocalDir = "./local"
$BuildDir = "$LocalDir/build"
$PrefixDir = (Get-Location).Path + "/prefix"

$NumCores = [Environment]::ProcessorCount

function Configure($Name, $ExtraArgs = "") {
    $Command = "cmake -B $BuildDir-$Name -DCMAKE_PREFIX_PATH=$PrefixDir -DCMAKE_CXX_COMPILER=cl -DCMAKE_C_COMPILER=cl -DCMAKE_BUILD_TYPE=Debug -S $LocalDir/$Name -DCMAKE_INSTALL_PREFIX=$PrefixDir $ExtraArgs"
    Write-Output "Running $Command"
    Invoke-Expression $Command
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to configure $Name"
    }
}

function Clone($Name, $Url) {
    if (Test-Path "$LocalDir/$Name") {
        Write-Information "Skipping clone of $Name because it's source directory already exists"
    } else {
        git clone --depth=1 $Url "$LocalDir/$Name"

        if ($LASTEXITCODE -ne 0) {
            throw "Failed to clone $Name from $Url"
        }
    }
}

function CheckCompileResult($Name) {
    if ($LASTEXITCODE -ne 0) {
        throw "Failed to build $Name!"
    }
}

if (!(Test-Path $LocalDir)) {
    New-Item -ItemType Directory -Path $LocalDir
}

# Setup Windows dependencies
Invoke-WebRequest https://xiv.zone/distrib/dependencies/gettext.zip -OutFile "$LocalDir/gettext.zip"
Expand-Archive -Path "$LocalDir/gettext.zip" -DestinationPath $PrefixDir -Force

Invoke-WebRequest https://xiv.zone/distrib/dependencies/iconv.zip -OutFile "$LocalDir/iconv.zip"
Expand-Archive -Path "$LocalDir/iconv.zip" -DestinationPath $PrefixDir -Force

Invoke-WebRequest https://xiv.zone/distrib/dependencies/icoutils.zip -OutFile "$LocalDir/icoutils.zip"
Expand-Archive -Path "$LocalDir/icoutils.zip" -DestinationPath $PrefixDir -Force

# Build zlib
Clone "zlib" "https://github.com/madler/zlib.git"
Configure "zlib" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-zlib" --config Debug --target install
CheckCompileResult "zlib"

# Build Extra CMake Modules
Clone "extra-cmake-modules" "https://invent.kde.org/frameworks/extra-cmake-modules.git"
Configure "extra-cmake-modules" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-extra-cmake-modules" --config Debug --target install --parallel $NumCores
cmake --install "$BuildDir-extra-cmake-modules" --config Debug
CheckCompileResult "extra-cmake-modules"

# Build KI18n
Clone "ki18n" "https://invent.kde.org/frameworks/ki18n.git"
# Workaround for Windows
Configure "ki18n" "-DBUILD_TESTING=OFF"

(Get-Content -ReadCount 0 "$BuildDir-ki18n/cmake/build-pofiles.cmake") -replace 'FATAL_ERROR', 'WARNING' | Set-Content "$BuildDir-ki18n/cmake/build-pofiles.cmake"
cmake --build "$BuildDir-ki18n" --config Debug --target install --parallel $NumCores
CheckCompileResult "ki18n"

# Build KCoreAddons
Clone "kcoreaddons" "https://invent.kde.org/frameworks/kcoreaddons.git"
Configure "kcoreaddons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kcoreaddons" --config Debug --target install --parallel $NumCores
CheckCompileResult "kcoreaddons"

# Build KConfig
Clone "kconfig" "https://invent.kde.org/frameworks/kconfig.git"
Configure "kconfig" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kconfig" --config Debug --target install --parallel $NumCores
CheckCompileResult "kconfig"

# Build KArchive
Clone "karchive" "https://invent.kde.org/frameworks/karchive.git"
Configure "karchive" "-DBUILD_TESTING=OFF -DWITH_BZIP2=OFF -DWITH_LIBLZMA=OFF -DWITH_LIBZSTD=OFF"
cmake --build "$BuildDir-karchive" --config Debug --target install --parallel $NumCores
CheckCompileResult "karchive"

# Build Kirigami
Clone "kirigami" "https://invent.kde.org/frameworks/kirigami.git"
Configure "kirigami" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kirigami" --config Debug --target install --parallel $NumCores
CheckCompileResult "kirigami"

# Build Kirigami Add-ons
Clone "kirigami-addons" "https://invent.kde.org/libraries/kirigami-addons.git"
Configure "kirigami-addons" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-kirigami-addons" --config Debug --target install --parallel $NumCores
CheckCompileResult "kirigami-addons"

# Build Corrosion
Clone "corrosion" "https://github.com/corrosion-rs/corrosion.git"
Configure "corrosion" "-DCORROSION_BUILD_TESTS=OFF"
cmake --build "$BuildDir-corrosion" --config Debug --target install --parallel $NumCores
CheckCompileResult "corrosion"

# Build QCoro
Clone "qcoro" "https://github.com/danvratil/qcoro.git"
Configure "qcoro" "-DBUILD_TESTING=OFF"
cmake --build "$BuildDir-qcoro" --config Debug --target install --parallel $NumCores
CheckCompileResult "qcoro"

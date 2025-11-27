There are only two kinds of releases currently:

* **Major releases** include major features and of course, bug fixes.
* **Minor releases** should only include bug fixes. Important features can be backported when nessecary, but don't try to rock the boat.

It's important that the major number in the version string should not be changed for minor releases. For example:

* 0.4.0 -> 0.5.0 is a major release.
* 0.4.0 -> 0.4.1 is a minor release.

# Support policy

Only one major release should be supported at a time, and only the latest minor release for it. If there is 0.4.0, 0.5.0, 0.5.1 then only 0.5.1 is supported.

# Who can create releases

I'm the only one who can create releases, due to having SFTP access to the web host and my GPG key. In the future this can be expanded to more people by trusting another GPG key, however.

# Branches

Major releases should have their own branch such as `release/0.5`. These branches are meant to be stable, and minor releases built on top of them linearly. All references to these branches shall henceforth be called "release branches".

# Cherry-picking

All bugfixes should ideally be committed to `main`, unless it's not nessecary (like fixing a bug only present on that release branch.) Then cherry-pick it to the relevant release branches with `git cherry-pick -x`. It's important to use `-x` so we have a reference to the original commit.

# GitHub Releases?

GitHub Releases should be avoided as they are an ecosystem lock-in. We have our own web host that we can host files on outside of GitHub. Similarly, users should not be required to download release artifacts from Actions as that also requires a GitHub account.

# Process

Once you're ready to make a release, there are several steps to do. These can really be freely done in any order, this is more of a checklist.

## Make sure the version is bumped

You need to bump the version in two places: CMakeLists.txt and the AppStream data. Modify the version text for the `project`, which is this line:

```cmake
project(Astra VERSION 0.5.1 LANGUAGES CXX)
```

And for AppData, create a new `<release>` tag:

```xml
<releases>
    <release version="0.5.1"/>
    <release version="0.5.0"/>
</releases>
```

{{< note "TODO: We should have a way to put the changelogs in the AppStream data." >}}

Remember to commit to the release branch before continuing.

## Create a git tag

Next is to create a Git tag for the release. Ensure you're on the correct branch and hash, and use `git tag -a <version number>`. For example, `git tag -a 0.5.2`. Make sure the first line is a short blurb for the update. This is what I entered for 0.5.1:

```
Minor update for recent game updates
* Made Dalamud asset downloading more reliable.
* Fix logging in when the 32-bit client disappeared.
* Made the source tarballs slightly smaller.
* Bypass the official FFXIV website browser check.
* Changed the default Dalamud inject method to "Entrypoint".
* More boot patch reliability fixes.
```

Then push the tag using `git push origin tag <version number>`.

## Create & upload the tarball

A source tarball can be created by `scripts/tarball.sh`. The final tarball will be created in the parent directory. The SHA256 should be recorded, and this should then be uploaded to the web host.

{{< note "TODO: I should GPG sign this tarball." >}}

## Update xiv.zone

Currently the website has to be kept up to date manually:

1. Update the current stable version text under /install.
2. Create a new version page & update the changelog link.
3. Update the source tarball link, including the size (if changed) and the SHA256.

## Update the Flatpak

Make sure to run the process described in [Updating the Flatpak](astra/updating-flatpak).

## Update the distro packaging

There are three packages to update and test:

* Fedora
* Gentoo
* Arch Linux

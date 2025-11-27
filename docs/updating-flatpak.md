When the Flatpak repository needs updating, first run `scripts/build-flatpak-for-repo.sh`. The repository state path is hardcoded to `/home/josh/sources/flatpak-distrib` and to my personal GPG key for now.

Once that's done, run `scripts/copy-repo-to-server.sh` and then the Flatpak repository should be updated.

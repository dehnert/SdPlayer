# https://nix.dev/tutorials/packaging-existing-software#building-with-nix-build
# default.nix
let
  pkgs = import <nixpkgs> { config = {}; overlays = []; };
in
{
  sdplayer = pkgs.callPackage ./sdplayer.nix { };
}

# Running `nix-build default.nix` will run the build and spit out a path
# Running "`nix-build default.nix`/bin/SdPlayer" will actually run SdPlayer...
# at least in my limited experimentation.

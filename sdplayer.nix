{ stdenv, lib, libsForQt5, qtcreator, libvlc }:

# See https://discourse.nixos.org/t/wrapqtappshook-out-of-tree/5619/2
let
  pkgs = import <nixpkgs> {};
  fs = lib.fileset;
  mkDerivation = pkgs.libsForQt5.callPackage ({ mkDerivation }: mkDerivation) {};
in mkDerivation rec {
  pname = "SdPlayer";
  version = "2.0";

  meta = with lib; {
    platforms = platforms.linux;
    mainProgram = "SdPlayer";
  };

  buildInputs = [ libsForQt5.qt5.qtbase qtcreator libvlc ];
  #nativeBuildInputs = [ wrapQtAppsHook ];

  src = fs.toSource {
    root = ./.;
    fileset = fs.gitTracked ./.;
  };

  preConfigure = ''qmake -config release'';
  postInstall = ''
  mkdir $out/bin
  cp SdPlayer $out/bin/
  mkdir -p $out/etc/xdg/krubow
  cp assets/sdplayer.ini $out/etc/xdg/krubow/
  '';
}

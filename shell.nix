{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell {
  buildInputs = [ pkgs.simgrid pkgs.cmake ];
  shellHook = ''
    export CMAKE_PREFIX_PATH="${pkgs.simgrid}/"
  '';
}


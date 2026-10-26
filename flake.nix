{
  description = "Replacement firmware for the Toptron EL658 LED dimmer";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in {
      devShells.${system}.default = pkgs.mkShell {
        packages = [
          pkgs.pkgsCross.avr.buildPackages.gcc
          pkgs.avrdude
          pkgs.gnumake
          pkgs.gcc
          pkgs.python3
        ];
        buildInputs = [ pkgs.pkgsCross.avr.avrlibc ];
      };
    };
}

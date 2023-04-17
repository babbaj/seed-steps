{
  description = "A very basic flake";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";
    #flake-utils.url = "github:numtide/flake-utils/master";
  };

  outputs = { self, nixpkgs }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs { inherit system; };
  in {

    devShells.${system}.default = pkgs.mkShell {
      packages = with pkgs; [
        cmake
        pkg-config
        gtest
        abseil-cpp
        fmt
      ];

      shellHook = ''
        export NIX_CFLAGS_COMPILE="-march=native"
        #export CC=${pkgs.clang_15}/bin/clang
        #export CXX=${pkgs.clang_15}/bin/clang++
      '';
    };

    packages.${system}.seed-steps = pkgs.stdenv.mkDerivation {
        name = "seed-steps";
        version = "1.0";

        nativeBuildInputs = with pkgs; [
            cmake
            pkg-config
        ];
        buildInputs = with pkgs; [
            fmt
        ];

        src = ./.;

        preConfigure = ''
            ls
        '';

        installPhase = ''
            mkdir -p $out/bin
            mv mansion_seeds $out/bin
        '';
    };
  };
}

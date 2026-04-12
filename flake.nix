{
  description = "ESP32 infrared MQTT vs HTTP local demo";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { nixpkgs, ... }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      python = pkgs.python313.withPackages (ps: with ps; [
        flask
        paho-mqtt
      ]);
    in {
      devShells.${system}.default = pkgs.mkShell {
        packages = [
          python
          pkgs.mosquitto
          pkgs.curl
          pkgs.jq
          pkgs.platformio
          pkgs.esptool
        ];
      };
    };
}

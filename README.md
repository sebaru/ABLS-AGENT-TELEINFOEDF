# abls-agent-teleinfoedf

Standalone teleinfoedf runtime for Abls-Habitat.

## Current implementation status

- Runtime based on `abls-agent-libs`
- Keeps SRC `Watchdogd/TeleinfoEDF` business logic:
  - serial Teleinfo EDF line handling with reconnect and retry loop
  - historical mode and standard mode frame parsing
  - EDF mnemonics creation and AI publication through MQTT API
  - communication status reporting to master (`IO_COMM`)

Expected API config fields:

- `port` (serial device, for example `/dev/ttyUSB0`)
- `standard` (`true` for TIC standard mode, `false` for historical mode)

## Build

```sh
./install_deps.sh
./build.sh
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces runtime RPM package in `build/`.

The runtime package also installs a templated systemd unit:

- `abls-agent-teleinfoedf@.service`

Start one instance per agent tech id:

```sh
sudo systemctl enable --now abls-agent-teleinfoedf@<agent-tech-id>.service
```

## Packaging DEB

```sh
./build_apt.sh --dist bookworm
./build_apt.sh --dist trixie
```

Default target suite is detected from host OS codename (`/etc/os-release`), with `bookworm` fallback.

Useful options:

- `--version-suffix <s>`: override Debian version suffix (example `~trixie`)
- `--no-dist-suffix`: disable automatic `~<suite>` suffix

Produces runtime DEB package and copies normalized artifacts to:

- `build/deb/<suite>/<arch>/`

`build_apt.sh` builds only the native host architecture.

Package signatures are centralized in ABLS-PKGS (both DEB repository metadata and RPM package/repository signatures).

The DEB package installs the same templated systemd unit:

```sh
sudo systemctl enable --now abls-agent-teleinfoedf@<agent-tech-id>.service
```

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- copies RPM to `../ABLS-PKGS/public/rpms/<arch>/`
- copies DEB to `../ABLS-PKGS/deb-packages/<suite>/<arch>/`

## Container build

```sh
podman build -t abls-agent-teleinfoedf:dev \
  --build-arg ABLS_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_LIBS_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_RPM_URL=<url> \
  -f Containerfile .
```

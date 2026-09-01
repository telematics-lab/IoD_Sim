# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

IoD_Sim is a system-level simulator for the Internet of Drones, built as an **ns-3 contrib module** (module name `iodsim`). It layers drones, ZSPs (Zone Service Providers), remotes, and LEO/GEO satellites on top of ns-3, with standard-compliant WiFi / LTE / 5G-NR stacks. Simulations are driven entirely by **JSON scenario files** — there is essentially no per-experiment C++; you describe a scenario in JSON and the C++ engine builds and runs it.

Current stack: ns-3.48 + 5G-LENA (nr) `5g-lena-v5.1.y`. See the compatibility table in `README.md` before changing versions.

## Repository layout

- `src/` — the `iodsim` ns-3 module source (models, helpers, configuration, reporting). This is where nearly all C++ work happens.
- `leo/` — the `leo` ns-3 contrib module for LEO/GEO satellites (symlinked into `ns3/contrib/leo` by the setup scripts).
- `ns3/` — a **clone of ns-3-dev** created by the tooling; not tracked here. IoD_Sim's `src/` is wired in as a contrib module. Build and run commands are executed from inside this directory.
- `scenario/` — JSON scenario files (the actual "programs"). Also the CTest suite (`scenario/CMakeLists.txt` lists which run as tests).
- `analysis/` — post-processing Python/shell scripts (throughput, SINR, PDR, REM plotting, trajectory extraction). See `docs/analysis.md`.
- `tools/` — setup and utility scripts (see below).
- `results/` — simulation output; each run creates `<scenario_name>-<date>.<time>/`.
- `docs/leo-nr-json-guide.md` — the authoritative reference for JSON scenario keys (especially LEO/NR). Consult it when editing scenarios.

## Setup (one-time)

The build lives in a generated `ns3/` clone, so the environment must be assembled first. In order:

```bash
./tools/install-dependencies.sh   # system deps (Fedora/Arch/Debian; manual on macOS)
./tools/prepare-ns3.sh            # clone ns-3.48, symlink leo, clone nr, apply tools/patches/**
./tools/configure-iodsim.sh       # cmake configure + create .venv + ns3 configure
```

`configure-iodsim.sh` options: `--mode default` (vs default `debug`; release unsupported), `--no-cli` (disables `ENABLE_CLI_COMMANDS`). It enables only modules `iodsim,nr,leo,point-to-point-layout`, disables MPI/python, and creates a `.venv` with the analysis + scenario-design requirements.

`prepare-ns3.sh` clones every external checkout it owns (`ns3/`, `ns3/contrib/nr`) at a pinned ref and applies the patches from `tools/patches/ns3/` and `tools/patches/nr/` via `git am`. It is **idempotent**: re-running it on an already-prepared tree does nothing. Work is redone only when a pinned ref or a patch set changes, and then it restarts from the pinned ref. Uncommitted changes inside those checkouts abort the re-preparation rather than being clobbered — pass `--force` to discard them. If you need to modify ns-3 or nr itself, do it as a patch there — those trees are disposable.

VSCode tasks mirror this flow (`1-Install dependencies` … `4-Build IoD Sim`, plus `Run IoD Sim`, `Generate REMs`, `Test all IoD Sim scenarios`).

## Build, run, test

All build/run commands run from inside `ns3/`. The executable is versioned as `ns3.48-scenario-<mode>`.

```bash
cd ns3
./ns3 build                                       # build
./ns3 run "scenario --config=../scenario/simple_wifi.json"   # run a scenario
```

Radio Environment Maps: add `--radioMaps` to the run args (or the `Generate REMs` task).

Direct binary (after build), e.g. `ns3/build/ns3.48-scenario-debug --config=/path/to/scenario.json`.

Tests are the scenarios themselves, run via CTest:

```bash
# from ns3/
./ns3 build && CTEST_OUTPUT_ON_FAILURE=1 CTEST_PARALLEL_LEVEL=$(nproc) make test -C ../build
# run a single scenario/test directly:
./ns3 run "scenario --config=../scenario/<name>.json"
```

Which scenarios are registered as tests is controlled by the `Tests` list in `scenario/CMakeLists.txt` — add a scenario there to include it in the suite.

Static/portable binary via Docker: `cd tools/compile && docker compose up --build && docker compose rm -f -s -v` → binary in `tools/compile/bin`.

Most analysis scripts and some VSCode run tasks expect the `.venv` active: `source .venv/bin/activate`.

## Architecture

**Entry point & flow.** `src/main.cc` constructs `ns3::Scenario` and calls `operator()`. `Scenario` (declared in `src/scenario.h`, implemented across `src/scenario-*.cc` — split by concern: `-core`, `-entities`, `-link`, `-mac`, `-mobility`, `-network`, `-phy`) drives the whole run: parse config → `ConfigureWorld/Phy/Mac/Network` → configure entities (mobility, IPv4, apps, peripherals, battery) → optional REM generation → `Simulator::Run()` → write reports.

**Configuration pipeline (the heart of the project).** JSON → C++ objects → ns-3 setup:
- `ScenarioConfigurationHelper` (a `Singleton`, accessed via the `CONFIGURATOR` macro, `src/configuration/helper/scenario-configuration-helper.h`) parses the top-level JSON and exposes typed getters.
- `src/configuration/*-configuration.{cc,h}` are typed holders for each layer/device (phy, mac, netdevice, network-layer, mobility, lte/nr variants, remote, bearer, qos-flow…).
- `src/configuration/helper/*.cc` are the builder helpers that translate those configuration objects into concrete ns-3 objects (`entity-configuration-helper`, `phy-layer-configuration-helper`, `network-layer-configuration-helper`, `mobility-factory-helper`, `wifi-*`, `lte-setup-helper`, `nr-phy-simulation-helper`, …).

When adding a new configurable feature, the pattern is: add a `*-configuration` holder + its `*-configuration-helper`, register the source in `src/CMakeLists.txt`, and document the JSON keys in `docs/leo-nr-json-guide.md`.

**JSON parsing** uses [rapidyyjson](https://github.com/domysh/rapidyyjson) — a RapidJSON-compatible API on top of yyjson. CMake fetches it itself with `FetchContent` in `src/CMakeLists.txt`, pinned to a commit SHA, exactly as `leo/CMakeLists.txt` does for sgp4; bump `GIT_TAG` there to update it. It is header-only, and linking the `rapidyyjson::rapidyyjson` INTERFACE target brings both its include directory and `libyyjson`, so no `include_directories()` or `find_library()` is involved. Because `build_lib()` puts `iodsim` in ns-3's export set, the target is added to that same export set — without it, CMake fails at generate time (the same reason leo exports `sgp4`). Fixes to the library belong upstream in that repo, not here.

**Entities & registries.** `src/entity/` defines drones, plus global lists (`drone-list`, `zsp-list`, `remote-list`, `leo-sat-list`) following ns-3's `*List` singleton pattern. Drones have an energy model and peripherals (`src/peripheral/`: storage, input, generic — with power/state modeling).

**Applications** (`src/application/`) are ns-3 apps installed onto nodes from JSON (drone client/server, TCP/UDP variants, NAT, storage client). `app-statistics-helper` collects per-app stats.

**Reporting** (`src/report/`) builds a structured XML report of a run (validated against `src/report/schema.xsd`): per-entity location/trajectory, protocol stacks, transfers, data stats, simulation duration. The `analysis/` scripts consume this XML plus ns-3 traces (pcap, LTE/NR stats files).

**IRS** (`src/irs/`) implements Intelligent Reflecting Surface support (patch model, serving/patch configurators, an IRS-assisted spectrum channel) used by the `irs_*` scenarios.

**Mobility** (`src/mobility/`) adds flight-plan/curve-based and trace-based mobility models beyond ns-3's built-ins.

## Conventions

- **C++ style is enforced**: `.clang-format` (Microsoft base, 100-col, ns-3 include grouping) and `.clang-tidy`. Match the surrounding ns-3 idioms (`Ptr<>`, `NS_LOG_*`, attribute system, `GetTypeId`).
- `ENABLE_CLI_COMMANDS` (default ON) lets the simulation shell out (e.g. curl downloads, GUI previews). Guarded by the macro in `src/scenario.h` / `src/CMakeLists.txt`; disable with `configure-iodsim.sh --no-cli` for untrusted/sandboxed builds.
- New scenarios go in `scenario/`; use existing ones (`simple_wifi.json`, `leo-nr-*.json`, `irs_paper_*.json`) as templates and validate keys against `docs/leo-nr-json-guide.md`.
- `resultsPath` (auto-timestamped subfolder) vs `resultPath` (exact dir) — pick one per scenario.

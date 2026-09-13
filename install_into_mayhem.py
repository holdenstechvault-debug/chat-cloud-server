#!/usr/bin/env python3
"""
Install the Garage external app into a checkout of
portapack-mayhem/mayhem-firmware (current `next` layout).

Usage:
    python install_into_mayhem.py /path/to/mayhem-firmware

The script is intentionally conservative:
- it refuses to overwrite an existing garage app,
- it checks the expected current external-app address range,
- it aborts rather than guessing if Mayhem's layout has changed.
"""

from pathlib import Path
import re
import shutil
import sys

HERE = Path(__file__).resolve().parent
SOURCE_APP = HERE / "garage"

GARAGE_ADDR = 0xAE110000
GARAGE_END = 0xAE118000


def fail(msg):
    raise SystemExit("ERROR: " + msg)


def main():
    if len(sys.argv) != 2:
        fail("Usage: python install_into_mayhem.py /path/to/mayhem-firmware")

    repo = Path(sys.argv[1]).expanduser().resolve()
    cmake = repo / "firmware/application/external/external.cmake"
    linker = repo / "firmware/application/external/external.ld"
    info = repo / "firmware/tools/external_app_info.py"
    dest = repo / "firmware/application/external/garage"

    for p in (cmake, linker, info):
        if not p.exists():
            fail(f"Expected Mayhem file not found: {p}")

    if dest.exists():
        fail(f"{dest} already exists. Remove it first if you intend to reinstall.")

    ctext = cmake.read_text(encoding="utf-8")
    ltext = linker.read_text(encoding="utf-8")
    itext = info.read_text(encoding="utf-8")

    if "external/garage/" in ctext or "ram_external_app_garage" in ltext:
        fail("Garage app registration already exists.")

    if "org = 0xAE110000" in ltext:
        fail("0xAE110000 is already in use in external.ld; choose a new app slot.")

    end_match = re.search(r"external_apps_address_end\s*=\s*(0x[0-9A-Fa-f]+)", itext)
    if not end_match:
        fail("Could not find external_apps_address_end.")

    current_end = int(end_match.group(1), 16)
    if current_end > 0xAE108000:
        fail("Mayhem has added newer external-app slots since this package was made. Do not guess an address; update this package against the new tree.")

    shutil.copytree(SOURCE_APP, dest)

    cmake_marker = ("\t#keyfob 216 byte\n" "\texternal/keyfob/main.cpp\n" "\texternal/keyfob/ui_keyfob.cpp\n" "\texternal/keyfob/ui_keyfob.hpp\n")
    cmake_insert = cmake_marker + ("\n\t# Linear MegaCode garage transmitter\n" "\texternal/garage/main.cpp\n" "\texternal/garage/ui_garage.cpp\n" "\texternal/garage/ui_garage.hpp\n")
    if cmake_marker not in ctext:
        fail("Could not find keyfob block in external.cmake; Mayhem layout changed.")
    ctext = ctext.replace(cmake_marker, cmake_insert, 1)

    list_marker = "\tkeyfob\n\ttetris\n"
    list_insert = "\tkeyfob\n\tgarage\n\ttetris\n"
    if list_marker not in ctext:
        fail("Could not find keyfob entry in EXTAPPLIST; Mayhem layout changed.")
    ctext = ctext.replace(list_marker, list_insert, 1)

    mem_marker = "    ram_external_app_aprs_tx               (rwx) : org = 0xAE100000, len = 32k\n"
    mem_insert = mem_marker + "    ram_external_app_garage                (rwx) : org = 0xAE110000, len = 32k\n"
    if mem_marker not in ltext:
        fail("Could not find APRS TX memory marker in external.ld.")
    ltext = ltext.replace(mem_marker, mem_insert, 1)

    section_marker = ("    .external_app_aprs_tx : ALIGN(4) SUBALIGN(4)\n" "    {\n" "        KEEP(*(.external_app.app_aprs_tx.application_information));\n" "        *(*ui*external_app*aprs_tx*);\n" "    } > ram_external_app_aprs_tx")
    section_insert = section_marker + ("\n\n" "    .external_app_garage : ALIGN(4) SUBALIGN(4)\n" "    {\n" "        KEEP(*(.external_app.app_garage.application_information));\n" "        */external/garage/*(*ui*external_app*garage*);\n" "    } > ram_external_app_garage")
    if section_marker not in ltext:
        fail("Could not find APRS TX section in external.ld.")
    ltext = ltext.replace(section_marker, section_insert, 1)

    itext = re.sub(r"external_apps_address_end\s*=\s*0x[0-9A-Fa-f]+", "external_apps_address_end = 0xAE118000", itext, count=1)

    cmake.write_text(ctext, encoding="utf-8")
    linker.write_text(ltext, encoding="utf-8")
    info.write_text(itext, encoding="utf-8")

    print("Garage app installed into Mayhem source tree.")
    print(f"  App: {dest}")
    print("  Frequency: 318.000 MHz")
    print("  Facility: 0")
    print("  Transmitter: 44734")
    print("  Button: 2")
    print("  MegaCode key: 8575F2")
    print("Now build Mayhem. The app output will be: build/firmware/application/garage.ppma")

if __name__ == "__main__":
    main()

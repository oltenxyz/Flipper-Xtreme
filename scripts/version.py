#!/usb/bin/env python3
VERSION = "XFW-0047"

import json
import os
import subprocess
from datetime import date, datetime

from flipper.app import App


class GitVersion:
    REVISION_SUFFIX_LENGTH = 8

    def __init__(self, source_dir):
        self.source_dir = source_dir

    def get_version_info(self):
        commit = (
            self._exec_git(f"rev-parse --short={self.REVISION_SUFFIX_LENGTH} HEAD")
            or "unknown"
        )

        dirty = False
        try:
            self._exec_git("diff --quiet")
        except subprocess.CalledProcessError as e:
            if e.returncode == 1:
                dirty = True

        # If WORKFLOW_BRANCH_OR_TAG is set in environment, is has precedence
        # (set by CI)
        branch = (
            os.environ.get("WORKFLOW_BRANCH_OR_TAG", None)
            or VERSION
            or self._exec_git("rev-parse --abbrev-ref HEAD")
            or "unknown"
        )

        branch_num = self._exec_git("rev-list --count HEAD") or "n/a"

        version = os.environ.get("DIST_SUFFIX", None) or VERSION or "unknown"

        force_no_dirty = os.environ.get("FORCE_NO_DIRTY", None) or ""
        if force_no_dirty != "":
            dirty = False

        return {
            "GIT_COMMIT": commit,
            "GIT_BRANCH": branch,
            "VERSION": version,
            "BUILD_DIRTY": dirty and 1 or 0,
        }

    def _exec_git(self, args):
        cmd = ["git"]
        cmd.extend(args.split(" "))
        return (
            subprocess.check_output(cmd, cwd=self.source_dir, stderr=subprocess.STDOUT)
            .strip()
            .decode()
        )


class Main(App):
    def init(self):
        self.subparsers = self.parser.add_subparsers(help="sub-command help")

        # generate
        self.parser_generate = self.subparsers.add_parser(
            "generate", help="Generate version header"
        )

        self.parser_generate.add_argument("-o", dest="outdir", required=True)
        self.parser_generate.add_argument(
            "-t",
            dest="target",
            type=int,
            help="hardware target",
            required=True,
        )
        self.parser_generate.add_argument("--dir", dest="sourcedir", required=True)
        self.parser_generate.set_defaults(func=self.generate)

    def generate(self):
        current_info = GitVersion(self.args.sourcedir).get_version_info()

        if "SOURCE_DATE_EPOCH" in os.environ:
            build_date = datetime.utcfromtimestamp(int(os.environ["SOURCE_DATE_EPOCH"]))
        else:
            build_date = date.today()

        current_info.update(
            {
                "BUILD_DATE": build_date.strftime("%d-%m-%Y"),
                "TARGET": self.args.target,
            }
        )

        version_values = []
        for key in current_info:
            val = current_info[key]
            if isinstance(val, str):
                val = f'"{val}"'
            version_values.append(f"#define {key} {val}")

        new_version_info_fmt = "\n".join(version_values) + "\n"

        current_version_info = None
        version_header_name = os.path.join(self.args.outdir, "version.inc.h")
        version_json_name = os.path.join(self.args.outdir, "version.json")

        try:
            with open(version_header_name, "r") as file:
                current_version_info = file.read()
        except EnvironmentError as e:
            if self.args.debug:
                print(e)

        if current_version_info != new_version_info_fmt:
            if self.args.debug:
                print("old: ", current_version_info)
                print("new: ", new_version_info_fmt)
            with open(version_header_name, "w", newline="\n") as file:
                file.write(new_version_info_fmt)
            # os.utime("../lib/toolbox/version.c", None)
            print("Version information updated")
        else:
            if self.args.debug:
                print("Version information hasn't changed")

        version_json = {
            "firmware_build_date": current_info["BUILD_DATE"],
            "firmware_commit": current_info["GIT_COMMIT"],
            "firmware_branch": current_info["GIT_BRANCH"],
            "firmware_target": current_info["TARGET"],
            "firmware_version": current_info["VERSION"],
        }
        with open(version_json_name, "w", newline="\n") as file:
            json.dump(version_json, file, indent=4)
        return 0


if __name__ == "__main__":
    Main()()

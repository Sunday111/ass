from pathlib import Path
import subprocess
import shutil
import os


def main():
    script_dir = Path(__file__).parent.resolve()
    build_dir = script_dir / "build"
    llvm_root = Path("/home/sunday/llvm")
    llvm_install = llvm_root / "llvm-install"

    subprocess.check_call(["cmake", *("--build", build_dir)])
    executable = build_dir / "tests/ass_tests"
    profraw = build_dir / "profile.profraw"
    subprocess.check_call(
        [
            executable,
        ],
        env={**os.environ, "LLVM_PROFILE_FILE": profraw},
    )

    profdata = build_dir / "profile.profdata"
    subprocess.check_call(
        [
            llvm_install / "bin/llvm-profdata",
            "merge",
            "-sparse",
            profraw,
            *("-o", profdata),
        ]
    )

    subprocess.check_call(
        [
            llvm_install / "bin/llvm-cov",
            *("show", executable),
            f"-instr-profile={profdata}",
            *("-output-dir", build_dir / "report"),
            "-format=html",
            "--show-branch-summary",
            "--show-instantiation-summary",
            "--show-mcdc-summary",
            "--show-region-summary",
        ]
    )


if __name__ == "__main__":
    main()

from pathlib import Path
import subprocess
import shutil
import os

def main():
    script_dir = Path(__file__).parent.resolve()
    src_dir = script_dir
    build_dir = script_dir / 'build'
    # gtest_install = script_dir / 'googletest-install/lib/cmake/GTest'
    gtest_install = script_dir.parent / 'googletest-install'

    llvm = Path('/home/sunday/coverage/llvm-install')

    shutil.rmtree(build_dir, ignore_errors=True)
    subprocess.check_call([
        'cmake',
        *('-G', 'Ninja'),
        *('-S', src_dir),
        *('-B', build_dir),
        '-DCMAKE_BUILD_TYPE:STRING=Debug',
        '-DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON',
        f'-DCMAKE_C_COMPILER:FILEPATH={llvm/"bin/clang"}',
        f'-DCMAKE_CXX_COMPILER:FILEPATH={llvm/"bin/clang++"}',
        f'-DCMAKE_CXX_FLAGS=-std=c++20 -fprofile-instr-generate -fcoverage-mapping -fcoverage-mcdc',
        f'-DGTest_ROOT:PATH={gtest_install.as_posix()}',
        '-DASS_ENABLE_TESTING:BOOL=ON',
        '-DASS_FETCH_GOOGLE_TESTS:BOOL=OFF',
        '-DASS_FIND_GTEST_PACKAGE:BOOL=ON',
    ])
    subprocess.check_call([
        'cmake',
        *('--build', build_dir)
    ])
    executable = build_dir / 'tests/ass_tests'
    profraw =  build_dir / 'profile.profraw'
    subprocess.check_call([
        executable,
        ], env={**os.environ, 'LLVM_PROFILE_FILE': profraw
    })

    profdata = build_dir / 'profile.profdata'
    subprocess.check_call([
        llvm / 'bin/llvm-profdata',
        'merge',
        '-sparse',
        profraw,
        *('-o', profdata),
    ])

    subprocess.check_call([
        llvm / 'bin/llvm-cov',
        *('show', executable),
        f'-instr-profile={profdata}',
        *('-output-dir', build_dir / 'report'),
        '-format=html',
        '--show-branch-summary',
        '--show-instantiation-summary',
        '--show-mcdc-summary',
        '--show-region-summary',
    ])

if __name__ == '__main__':
    main()

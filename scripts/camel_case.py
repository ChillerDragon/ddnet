#!/usr/bin/env python3

# pylint: skip-file

import argparse
import sys
import os
import glob
import subprocess
import contextlib
import shutil
import multiprocessing
import functools

if sys.version_info[1] < 11:
	print('you need python 3.11 or newer to run this script')
	sys.exit(1)

class CamelCaseNamespace(argparse.Namespace):
	skip: int
	jobs: int
	dry_run: bool
	build_dir: str

class color:
	@classmethod
	def no_color(cls) -> bool:
		if os.environ.get('NO_COLOR'):
			return True
		if sys.stdout.isatty() is False:
			return True
		return False

	@classmethod
	def green(cls) -> str:
		if cls.no_color():
			return ''
		return '\033[92m'

	@classmethod
	def red(cls) -> str:
		if cls.no_color():
			return ''
		return '\033[91m'

	@classmethod
	def bold(cls) -> str:
		if cls.no_color():
			return ''
		return '\033[1m'

	@classmethod
	def end(cls) -> str:
		if cls.no_color():
			return ''
		return '\033[0m'


def get_code_files() -> list[str]:
	files: list[str] = glob.glob('./src/**/*.cpp', recursive=True)
	files += glob.glob('./src/**/*.h', recursive=True)
	files_filtered: list[str] = []
	skip_files = (
		'./src/game/mapitems_ex_types.h',
		'./src/engine/shared/protocol_ex_msgs.h',
		'./src/engine/shared/teehistorian_ex_chunks.h',
		'./src/game/mapbugs_list.h',
		'./src/game/tuning.h',
		'./src/engine/shared/config_variables.h',
		'./src/engine/shared/websockets.h',
		'./src/engine/client/keynames.h',
	)
	skip_dirs = (
		'./src/rust-bridge/',
		'./src/game/generated/',
		'./src/test/',
		'./src/base/',
		'./src/engine/external/',
		'./src/tools/',
	)
	for file in files:
		if file in skip_files:
			continue
		skip = False
		for skip_dir in skip_dirs:
			if file.startswith(skip_dir):
				skip = True
				continue
		if skip:
			continue
		files_filtered.append(file)
	return files_filtered

def command_failed(args: list[str]) -> None:
	print(f"{color.red()}command failed:{color.end()}", file=sys.stderr)
	print('', file=sys.stderr)
	print(f"{color.bold()}  {' '.join(args)}{color.end()}", file=sys.stderr)
	print('', file=sys.stderr)

def check_file(file_path: str, current: int, total: int, skip: int, clang_tidy_bin: str, build_dir: str, dry_run: bool) -> bool:
	if current < skip:
		return True

	print(f"[{current}/{total}] {color.green()}{file_path}{color.end()}")
	clang_config: str = os.path.join(
			os.path.dirname(os.path.realpath(__file__)),
			'clang-tidy-camel-case.yml')
	clang_tidy_cmd: list[str] = [
		clang_tidy_bin,
		f"--config-file={clang_config}",
		file_path,
		'-p',
		build_dir]
	if not dry_run:
		clang_tidy_cmd.append('--fix')
	proc: subprocess.CompletedProcess = subprocess.run(
		clang_tidy_cmd,
		capture_output=True,
		text=True)

	# always print the warnigs clang finds
	# camel case style offenses are printed to stdout
	# by clang tidy
	if proc.stdout != '':
		print(proc.stdout)

	if proc.returncode == 0:
		return True

	# applying fixes returns exit code 1
	if not dry_run and proc.returncode == 1:
		return True

	# clang tidy prints suppression info to stderr
	# so we only want to see stderr when clang tidy failed
	print(proc.stderr, file=sys.stderr)
	command_failed(clang_tidy_cmd)
	return False

def compile_ddnet(build_dir: str, jobs: int) -> None:
	if os.path.exists(build_dir):
		return
	os.mkdir(build_dir)
	with contextlib.chdir(build_dir):
		exit_code: int = subprocess.call([
			'cmake',
			'-DCMAKE_BUILD_TYPE=Debug',
			'-DDOWNLOAD_GTEST=ON',
			'-DCMAKE_EXPORT_COMPILE_COMMANDS=1',
			'..'
		])
		if exit_code != 0:
			print('failed to build ddnet', file=sys.stderr)
			sys.exit(1)
		exit_code = subprocess.call([
			'make',
			f"-j{jobs}"
		])
		if exit_code != 0:
			print('failed to build ddnet', file=sys.stderr)
			sys.exit(1)

def generated_code_hack(build_dir: str) -> None:
	os.makedirs('src/game/generated', exist_ok=True)
	for gen_file in ('client_data.h', 'client_data7.h'):
		shutil.copy(os.path.join(build_dir, 'src/game/generated', gen_file), 'src/game/generated')

def main() -> int:
	p = argparse.ArgumentParser(description="")
	p.add_argument('--skip', help="", default=0, type=int)
	p.add_argument('-j', '--jobs', help="", default=multiprocessing.cpu_count(), type=int)
	p.add_argument('-n', '--dry-run', action='store_true', help="Don't fix, only warn", default=False)
	p.add_argument('build_dir', help="", default='build-camel', nargs='?', type=str)
	args = p.parse_args(namespace=CamelCaseNamespace())

	compile_ddnet(args.build_dir, args.jobs)
	generated_code_hack(args.build_dir)

	files: list[str] = get_code_files()
	total: int = len(files)

	with multiprocessing.Pool(args.jobs) as pool:
		check_file_prefilled = functools.partial(check_file, skip=args.skip, total=total, clang_tidy_bin='clang-tidy', build_dir=args.build_dir, dry_run=args.dry_run)
		results = pool.starmap(check_file_prefilled, zip(files, range(len(files))))

	if all(results):
		return 0
	return 1

if __name__ == "__main__":
	sys.exit(main())


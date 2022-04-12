#!/bin/python

import argparse
import json
import os
import multiprocessing
import subprocess
import sys

TGREEN =  '\033[32m' # Green Text	
TBLUE =  '\033[34m' # Green Text	
TRED =  '\033[31m' # Red Text	
TWHITE = '\033[39m' #Withe text

description_help='''Build all adicup projects
Examples:\n
    >python build_projects.py ..\.. aducm3029
	>python build_projects.py /home/user/noos xilinx
'''

def parse_input():
    parser = argparse.ArgumentParser(description=description_help,\
			    formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument('adicup_location', help="Path to adicup repo location")
    parser.add_argument('export_dir', help="Path where to save files")
    args = parser.parse_args()

    return (args.adicup_location, args.export_dir)

VERBOSE = 0
ERR = 0

def shell_source(script):
	"""
	Sometime you want to emulate the action of "source" in bash,
	settings some environment variables. Here is a way to do it.
	"""

	pipe = subprocess.Popen(". %s && env -0" % script, stdout=subprocess.PIPE, shell=True, executable="/bin/bash")
	output = pipe.communicate()[0].decode('utf-8')
	output = output[:-1] # fix for index out for range in 'env[ line[0] ] = line[1]'

	env = {}
	# split using null char
	for line in output.split('\x00'):
		line = line.split( '=', 1)
		#print(line)
		env[ line[0] ] = line[1]

	os.environ.update(env)

def run_cmd(cmd):
	log_file = 'log.txt'
	print(cmd)
	sys.stdout.flush()
	if (VERBOSE):
		os.system(cmd)
	else:
		err = os.system(cmd + ' > %s 2>&1' % log_file)
		if (err != 0):
			global ERR
			print(TRED + "Error" + TWHITE)
			print("See log:")
			os.system("cat %s" % log_file)
			ERR = err

def to_blue(str):
	return TBLUE + str + TWHITE

def print_build(build_name, project):
	print("Runing build %s on project %s" % (to_blue(build_name),
						to_blue(project)))

def build_cces_project(adicup_location, project, project_dir, export_dir):
	DEFAULT_WORKSPACE = os.path.join(adicup_location, 'workspace')
	CCES_TEMPLATE = "cces -nosplash \
-application com.analog.crosscore.headlesstools -data %s -project %s "
	print_build("cces", project)

	cces_cmd = CCES_TEMPLATE % (DEFAULT_WORKSPACE, project_dir)
	run_cmd(cces_cmd + '-build Release')

	binary = os.path.join(project_dir, 'Release', project)
	hex = os.path.join(export_dir, project + '.hex')
	run_cmd("arm-none-eabi-objcopy -O ihex %s %s" % (binary, hex))

	print(TGREEN + "DONE" + TWHITE)

def build_noos_project(build_file, project, project_dir, export_dir, build_dir):
	CMD_TEMPLATE = 'make -C %s %s BUILD_DIR=%s BINARY=%s LOCAL_BUILD=n LINK_SRCS=n VERBOSE=y '
	fp = open(build_file)
	builds = json.loads(fp.read())
	for (build_name, flags) in builds.items():
		print_build(build_name, project)
		binary = os.path.join(build_dir, "%s_%s.elf" % (project, build_name))
		export_file = os.path.join(build_dir, binary)
		cmd = CMD_TEMPLATE % (project_dir, flags, build_dir, binary)
		run_cmd(cmd + 'update_srcs')
		run_cmd(cmd + '-j%d re' % (multiprocessing.cpu_count() - 1))
		export_file = export_file.replace('.elf', '.hex')
		run_cmd("cp %s %s" % (export_file, export_dir))
		print(TGREEN + "DONE" + TWHITE)
	fp.close()

def main():
	shell_source(("~/.aducm3029_environment.sh"))
	(adicup_location, export_dir) = parse_input()
	projets = os.path.join(adicup_location,'projects')
	run_cmd("test -d {0} || mkdir -p {0}".format(export_dir))
	dirs = os.listdir(projets)
	for project in dirs:
		project_dir = os.path.join(projets, project)
		build_file = os.path.join(project_dir, 'builds.json')
		if os.path.isfile(build_file):
			build_dir = os.path.join(adicup_location, 'build')
			build_noos_project(build_file, project, project_dir, export_dir, build_dir)
		else:
			build_cces_project(adicup_location, project, project_dir, export_dir)
main()

if (ERR):
	exit("Error occured: %d" % ERR)

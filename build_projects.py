#!/bin/python

import argparse
import json
import os
import multiprocessing
import sys

TGREEN =  '\033[32m' # Green Text	
TBLUE =  '\033[34m' # Green Text	
TRED =  '\033[31m' # Red Text	
TWHITE = '\033[39m' #Withe text

description_help='''Build all noos projects
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
DEBUG = 0

def run_cmd(cmd):
	log_file = 'log.txt'
	print(cmd)
	sys.stdout.flush()
        if (VERBOSE):
                os.system(cmd)
        else:
	        err = os.system(cmd + ' > %s 2>&1' % log_file)
	        if (err != 0):
		        print(TGREEN + "Error" + TWHITE)
		        print("See log:")
		        os.system("cat %s" % log_file)

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

        if DEBUG == 1:
                # This will not update the project in the projects folder
                # only in workspace anyway some project may not work because
                # they don't have the same name as the folder:
                # cn0357, adt7420, adxl362, asset_health, cn0397, ad5592r
                cces_cmd = CCES_TEMPLATE % (DEFAULT_WORKSPACE, project_dir)
                run_cmd(cces_cmd + '-copy')

                cces_cmd = CCES_TEMPLATE % (DEFAULT_WORKSPACE, project)
                run_cmd(cces_cmd + '-build Debug')
                binary = os.path.join(DEFAULT_WORKSPACE, project, 'Debug', project)
        else:
                cces_cmd = CCES_TEMPLATE % (DEFAULT_WORKSPACE, project_dir)
                run_cmd(cces_cmd + '-build Debug')
                binary = os.path.join(project_dir, 'Debug', project)

        hex = os.path.join(export_dir, project + '.hex')
        run_cmd("arm-none-eabi-objcopy -O ihex %s %s" % (binary, hex))
        print(TGREEN + "DONE" + TWHITE)

def build_noos_project(build_file, project, projet_dir, export_dir):
        CMD_TEMPLATE = 'make -C %s %s BUILD_DIR_NAME=%s BINARY=%s -j%d LOCAL_BUILD=n LINK_SRCS=n VERBOSE=y '
        fp = open(build_file)
        builds = json.loads(fp.read())
        for (build_name, flags) in builds.items():
                print_build(build_name, project)
                build_dir_name = 'build_%s' % build_name
                binary = os.path.join(build_dir_name, "%s_%s.elf" % (project, build_name))
                export_file = os.path.join(projet_dir, binary)
                cmd = CMD_TEMPLATE % (projet_dir, flags, build_dir_name, binary, multiprocessing.cpu_count() - 1)
                run_cmd(cmd + 'ra')
                run_cmd(cmd + 'hex')
                export_file = export_file.replace('.elf', '.hex')
                run_cmd("cp %s %s" % (export_file, export_dir))
                print(TGREEN + "DONE" + TWHITE)
        fp.close()

def main():
	(adicup_location, export_dir) = parse_input()
	projets = os.path.join(adicup_location,'projects')
	run_cmd("test -d {0} || mkdir -p {0}".format(export_dir))
        dirs = os.listdir(projets)
	for project in dirs:
		project_dir = os.path.join(projets, project)
		build_file = os.path.join(project_dir, 'builds.json')
		if os.path.isfile(build_file):
                        pass
                        build_noos_project(build_file, project, project_dir, export_dir)
                else:
                        build_cces_project(adicup_location, project, project_dir, export_dir)
	run_cmd("zip -r {0}.zip {0}".format(export_dir))
main()
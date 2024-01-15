#!/bin/bash

# Copyright 2024(c) Analog Devices, Inc.
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#     - Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     - Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in
#       the documentation and/or other materials provided with the
#       distribution.
#     - Neither the name of Analog Devices, Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#     - The use of this software may or may not infringe the patent rights
#       of one or more patent holders.  This license does not release you
#       from the requirement that you obtain separate licenses from these
#       patent holders to use this software.
#     - Use of the software either in source or binary form, must be run
#       on or directly connected to an Analog Devices Inc. component.
#
# THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED.
#
# IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, INTELLECTUAL PROPERTY
# RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
# THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# The Adicup Release procedure consists of 3 tasks in the Releases section of Azure Pipelines:
# 1. Delete GitHub Release - deletes the old release in order to create a new one with same release
#                            tag.
# 2. Prepare Release - the release_projects.sh script is passed as input file from the Adicup
#                      sources affilated to the Release.
# 3. Create GitHub Release - generate new release using the same release tag and new sources and

# This is a script for the intermediate stage in the remote release pipeline following the new CI
# infrastructure for building the projects. The script gets all projects specific binaries and
# publish them as github releases.

RELEASE_TAG="Latest"

#Update Release Tag in preparation for new release
update_release_tag() {
	if [[ $(git show-ref --tags | grep "$RELEASE_TAG") ]]; then
		# delete Latest tag locally
		git tag -d $RELEASE_TAG
		# delete Latest tag remotely
		git push origin :refs/tags/$RELEASE_TAG
	fi
	# add Latest tag locally
	git tag -f $RELEASE_TAG HEAD
	# add Latest tag remotely
	git push -f --tags
}

update_release_tag

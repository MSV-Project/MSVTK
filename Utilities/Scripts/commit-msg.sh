#!/usr/bin/env bash
#=============================================================================
#
# Library: MSVTK
# 
# Copyright (c) Kitware, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================


MSG="$1"

die_advice='
Valid issue tokens are (in regex): 
    1. [Ff]ix(es)? #[0-9]+
    2. [Cc]lose(s)? #[0-9]+
    3. [Ii]ssue #[0-9]+
Look at: 
  https://github.com/blog/831-issues-2-0-the-next-generation 
for more information.

To continue editing, run the command
  git commit -e -F '"$MSG"'
(assuming your working directory is at the top).'

die() {
        echo 'github/commit-msg hook failure' 1>&2
        echo '-----------------------' 1>&2
        echo '' 1>&2
        echo "$@" 1>&2
        test -n "$die_advice" && echo "$die_advice" 1>&2
        exit 1
}

msg_github() {
        cat "$MSG" | grep -q -E '([Ff]ix(es)? #[0-9]+)|([Cc]lose(s)? #[0-9]+)|([Ii]ssue #[0-9]+)' && return
        die 'Github issue reference not found.'
}

msg_github 


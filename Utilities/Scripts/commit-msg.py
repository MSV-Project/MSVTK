#!/usr/bin/python
#=============================================================================
# Copyright 2010-2012 Kitware, Inc.
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

import logging
import sys
import re

issue_tokens = '(fix(ed|es)?\s#\d+)|(close(d|s)?\s#\d+)'

def check_message_format():
  if len(sys.argv) < 2 :
    logging.error("No commit message filename specified") 
    return 1
  
  commit_msg_filename = sys.argv[1]
  try:
    commit_msg_text = open(commit_msg_filename).read()
    
  except Exception, e:
    logging.error("Failed to open file '%s'", commit_msg_filename)
    logging.debug(e)
    return 1
  
  token_id = re.compile(issue_tokens, re.IGNORECASE)
  
  matches = token_id.search(commit_msg_text.decode('utf-8'))
  
  #print matches.group(0)
  if matches is None:
    print '[Policy] Your commit message is not formated correctly.'
    return 1
  return 0
  
  
#----------------------------------------------------------------------------
# python script entry point. Dispatches check_message_format()

if __name__ == "__main__":
  exit (check_message_format())
  
#!/usr/bin/python3

import os
import re

root = os.path.dirname(os.path.realpath(__file__))

test_pattern = r'tst_[a-zA-Z0-9]+(\.exe)?'
exit_code = 0

for root, directories, filenames in os.walk(root):
    for filename in filenames:
    
        if re.match(test_pattern, filename) is not None:
            test_exec = os.path.join(root,filename)
            
            if not os.path.exists(test_exec):
                continue

            if os.system(test_exec) != 0:
                exit_code = 1
            

exit(exit_code)


            

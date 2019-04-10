#!/usr/bin/python3

import os
import subprocess

# define all tests and expected return value
tests = [["noarea.config", 1],
         ["syntax.config", 1],
         ["threecorners.config", 0]
]


FNULL = open(os.devnull, 'w')

failed = 0
for test in tests:
    retval = subprocess.call(["../build/padring", "--svg", "padring.svg", "--lef", "iocells.lef", "-o","padring.gds", test[0]], stdout=FNULL)
    if (retval == test[1]):
        spaces = 30 - len(test[0])
        print(test[0] + (' '*spaces) + "OK!")
    else:
        failed = failed + 1
        print(test[0] + (' '*spaces) + "*** FAIL ***")

print("\nFailed tests: " + str(failed))


#!/usr/bin/python3

import os
import subprocess

# define all tests, the LEF library used and expected return value (1 = fail)
tests = [["noarea.config", "iocells.lef", 1],
         ["syntax.config", "iocells.lef", 1],
         ["threecorners.config", "iocells.lef", 0],
         ["fillerexit.config", "iocells_nofiller1.lef", 1],
         ["nonsquarecorners.config", "nonsquarecorners.lef", 0]
]


FNULL = open(os.devnull, 'w')

failed = 0
for test in tests:
    retval = subprocess.call(["../build/padring", "--svg", "padring.svg", "--def", "padring.def", "--lef", test[1], "-o","padring.gds", test[0]], stdout=FNULL)
    if (retval == test[2]):
        spaces = 30 - len(test[0])
        print(test[0] + (' '*spaces) + "OK!")
    else:
        failed = failed + 1
        print(test[0] + (' '*spaces) + "*** FAIL ***")

print("\nFailed tests: " + str(failed))


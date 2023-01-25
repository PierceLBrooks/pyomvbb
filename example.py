
# Author: Pierce Brooks

import sys
import json
import omvbb

"""
pbrooks@pbrooks:~/pyomvbb$ python3 ./example.py "[[10,15,20],[-70,-80,-90], [1,2,-3], [-1,-2,3]]"
p1: -70 -80 -90
p2: 10 15 20
 l: 165.907
estimated 3d diameter:  80  95 110 eps: 0.001
p1: -3.47451  1.34053
p2:  3.47451 -1.34053
 l: 7.44828
gridSearch: new volume: 2999.43
for dir: -2.02777  6.36819 -2.51681
gridSearch: new volume: 2997.89
for dir: -1.69263   5.2291 -3.28504
gridSearch: new volume: 2997.89
for dir:  3.02332  4.68105 -1.71678
gridSearch: new volume: 2997.89
for dir: -0.817338  0.566472  0.105202
(-70.0, -80.0, -90.0)
(9.642629623413086, 14.330904960632324, 20.826322555541992)
(-68.01571655273438, -81.375244140625, -90.25540161132812)
(11.626912117004395, 12.955659866333008, 20.570919036865234)
(-67.62860870361328, -75.56011199951172, -95.48319244384766)
(12.014019966125488, 18.770790100097656, 15.34312915802002)
(-65.64432525634766, -76.93536376953125, -95.73859405517578)
(13.998302459716797, 17.395544052124023, 15.087726593017578)
"""

if (__name__ == "__main__"):
    if (len(sys.argv) < 2):
        sys.exit()
    points = json.loads(sys.argv[1])
    for i in range(len(points)):
        points[i] = tuple(points[i])
    box = omvbb.OMVBB(0.0)
    corners = box.compute(points)
    for i in range(len(corners)):
        print(str(corners[i]))


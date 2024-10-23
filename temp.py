vals = ('x', 'y', 'z')
for first in vals:
    for second in vals:
        for third in vals:
            print("#define {}{}{}(vec) (vec3(vec.{}, vec.{}, vec.{}))".format(first, second, third, first, second, third))



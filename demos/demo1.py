from ngsolve import *
import ngsmetal

v = ngsmetal.MetalVector(5)
v2 = ngsmetal.MetalVector(5)

v *= 3
v *= 2
print (v)

print ("v2=",v2)
v2 += v
print ("now, v2=",v2)

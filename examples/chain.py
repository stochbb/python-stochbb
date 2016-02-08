#!/usr/bin/env python
# -*- coding: utf-8 -*-
from numpy import *
import stochbb;
from matplotlib import pylab

X1 = stochbb.gamma(10, 10);
X2 = stochbb.gamma(10, 20);
Y = X1+X2
Z = 20 + X1 + X2

Tmin, Tmax, N = 100, 400, 10000;
t = linspace(Tmin, Tmax, N)
pX1 = empty(N,); X1.density().eval(Tmin, Tmax, pX1)
pX2 = empty(N,); X2.density().eval(Tmin, Tmax, pX2)
pY = empty(N,); Y.density().eval(Tmin, Tmax, pY)
pZ = empty(N,); Z.density().eval(Tmin, Tmax, pZ)
#pZ2 = empty(N,); Z2.density().eval(Tmin, Tmax, pZ2)

#a = empty(N,); (X1+20).density().eval(Tmin, Tmax, a)
#b = empty(N,); (X2+100).density().eval(Tmin, Tmax, b)
#res = convolve(a,b, mode="same")

pylab.plot(t, pX1, label="X1~Gamma(10,10)")
pylab.plot(t, pX2, label="X2~Gamma(10,20)")
pylab.plot(t, pY, label="Y=X1+X2")
pylab.plot(t, pZ, label="Z=20+X1+X2")
#pylab.plot(t, pZ2, label="(X1+X2)+120")
pylab.legend()

pylab.show()
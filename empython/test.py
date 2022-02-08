# include the newmodule functions
import newmodule
from newmodule import *

# see what it supplies:
help (newmodule)

#define two functions to register
def x():
	print "This is function X"

def y():
	print "This is function Y"

# Register the functions
RegisterFunction(1, x)
RegisterFunction(2, y)

# Do something else
for i in xrange(5):
	print i

# Call the functions
CallFunctionByID(2)
CallFunctionByID(1)

# Constants from module:
print newmodule.INT1
print newmodule.STR1
# Import all instruction modules so they register themselves

# Base RV64I Operations
from . import add
from . import sub
from . import addi
from . import lw
from . import sw
from . import beq
from . import jal
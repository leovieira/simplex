# Simplex
Solver for linear programming problems

## How to create a model?
### 1º line:
- Specify whether to minimize "min" or maximize "max";
- Enter the objective function. Passing "\<var>=<coefficient>" and separating the monomials with a comma;
- E.g.: maximize z = 2\*a - 4\*b + 5\*c + 3.6\*d
  - Must be: max, a=2, b=-4, c=5, d=3.6

### nº line:
- Enter one constraint per row;
- Each constraint must be in the following model:
  - "\<var>=\<coefficient>", ..., "\<relational-operator>", "\<value>";
  - Since "\<relational-operator>" is an element of {>, <, =, >=, <=}.
- E.g.: -0.3\*a + 7\*b - 0.5\*c + 0.4\*d <= 40
  - Must be: a=-0.3, b=7, c=-0.5, d=0.4, <=, 40

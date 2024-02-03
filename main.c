#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

#define VAR_LENGTH 6
#define EPSILON 1e-9

typedef enum {
  MIN,
  MAX
} action_t;

typedef struct {
  char name[VAR_LENGTH + 1];
  double coef;
} variable_t;

typedef struct {
  variable_t vars[10]; // inequality variables
  int num_vars;
  char op[3]; // =, >, <, >= and <=
  double value;
} inequality_t;

typedef struct {
  action_t action;
  inequality_t obj_func;
  inequality_t consts[10]; // constraints
  int num_consts;
} problem_t;

typedef struct {
  int rows;
  int cols;
  char **rows_h;
  char **cols_h;
  double **data;
  action_t action;
  int art_vars;
} tableau_t;

void trim(char *str) {
  int start, end, i;

  for (start = 0; isspace((unsigned char)str[start]); ++start);

  if (str[start] == '\0') {
    str[0] = '\0';
    return;
  }

  end = strlen(str) - 1;
  while (end > start && isspace((unsigned char)str[end])) --end;

  for (i = start; i <= end; ++i) {
    str[i - start] = str[i];
  }

  str[i - start] = '\0';
}

int isEqual(double a, double b) {
  return fabs(a - b) < EPSILON;
}

int initializeTableau(tableau_t *tableau) {
  tableau->rows_h = (char **) malloc(tableau->rows * sizeof(char *));
  if (tableau->rows_h == NULL) return 1;
  for (int i = 0; i < tableau->rows; i++) {
    tableau->rows_h[i] = (char *) malloc((VAR_LENGTH + 1) * sizeof(char));
    if (tableau->rows_h[i] == NULL) return 1;
  }

  tableau->cols_h = (char **) malloc(tableau->cols * sizeof(char *));
  if (tableau->cols_h == NULL) return 1;
  for (int i = 0; i < tableau->cols; i++) {
    tableau->cols_h[i] = (char *) malloc((VAR_LENGTH + 1) * sizeof(char));
    if (tableau->cols_h[i] == NULL) return 1;
  }

  tableau->data = (double **) malloc(tableau->rows * sizeof(double *));
  if (tableau->data == NULL) return 1;
  for (int i = 0; i < tableau->rows; i++) {
    tableau->data[i] = (double *) calloc(tableau->cols, sizeof(double));
    if (tableau->data[i] == NULL) return 1;
  }
}

int findRowIdx(tableau_t tab, char *str) {
  for (int i = 0 ; i < tab.rows; i++)
    if (strcmp(tab.rows_h[i], str) == 0)
      return i;
  return -1;
}

int findColIdx(tableau_t tab, char *str) {
  for (int i = 0 ; i < tab.cols; i++)
    if (strcmp(tab.cols_h[i], str) == 0)
      return i;
  return -1;
}

void printTableau(tableau_t tableau) {
  printf("base    ");
  for (int i = 0; i < tableau.cols; i++) {
    printf(" | %-*s", 8, tableau.cols_h[i]);
  }

  /*
  printf("\n");
  for (int i = 0; i < 9 + tableau.cols * 11; i++) {
    printf("-");
  }
  */

  for (int i = 0; i < tableau.rows; i++) {
    printf("\n");
    for (int i = 0; i < tableau.cols + 1; i++) {
      for (int j = 0; (i == 0 && j < 9) || (i != 0 && j < 10); j++) {
        printf("-");
      }
      if (i < tableau.cols) printf("+");
    }

    printf("\n%-*s", 8, tableau.rows_h[i]);
    for (int j = 0; j < tableau.cols; j++) {
      printf(" | %-*.*f", 8, 2,
        (!isEqual(tableau.data[i][j], 0)) ? tableau.data[i][j] : fabs(tableau.data[i][j]));
    }
  }

  printf("\n");
}

int simplex(tableau_t *tableau, int two_phases) {
  int row_s, col_s;
  double value;

  if (two_phases) {
    printf("\n\n# Início da primeira fase:\n\n");
  } else {
    printf("\n\n# Início da segunda fase:\n\n");
  }
  printTableau(*tableau);

  do {
    col_s = -1;
    value = DBL_MAX;

    for (int j = 0; j < tableau->cols - 1; j++) {
      if (tableau->data[0][j] < 0 && tableau->data[0][j] < value) {
        col_s = j;
        value = tableau->data[0][j];
      }
    }

    if (col_s == -1) {
      if (two_phases && !isEqual(tableau->data[0][tableau->cols-1], 0)) {
        return 1;
      } else {
        break;
      }
    }

    row_s = -1;
    value = DBL_MAX;

    for (int i = (two_phases) ? 2 : 1; i < tableau->rows; i++) {
      double calc = tableau->data[i][tableau->cols-1] / tableau->data[i][col_s];
      if (calc > 0 && calc < value) {
        row_s = i;
        value = calc;
      }
    }

    if (row_s == -1) return 1;

    strcpy(tableau->rows_h[row_s], tableau->cols_h[col_s]);

    double pivot = tableau->data[row_s][col_s];

    for (int j = 0; j < tableau->cols; j++) {
      tableau->data[row_s][j] = tableau->data[row_s][j] / pivot;
    }

    for (int i = 0; i < tableau->rows; i++) {
      if (i == row_s) continue;
      pivot = tableau->data[i][col_s];

      for (int j = 0; j < tableau->cols; j++) {
        tableau->data[i][j] = tableau->data[i][j] - pivot * tableau->data[row_s][j];
      }
    }

    printf("\n\n");
    printTableau(*tableau);
  } while (1);

  if (!two_phases) return 0;

  tableau_t new_tableau;

  new_tableau.rows = tableau->rows - 1;
  new_tableau.cols = tableau->cols - tableau->art_vars;
  new_tableau.action = tableau->action;
  new_tableau.art_vars = 0;

  initializeTableau(&new_tableau);

  int r = -1, c = -1;

  for (int i = 1; i < tableau->rows; i++) {
    strcpy(new_tableau.rows_h[++r], tableau->rows_h[i]);
  }

  for (int i = 0; i < new_tableau.cols - 1; i++) {
    strcpy(new_tableau.cols_h[++c], tableau->cols_h[i]);
  }
  strcpy(new_tableau.cols_h[++c], tableau->cols_h[tableau->cols-1]);

  r = -1;
  c = -1;

  for (int i = 1; i < tableau->rows; i++) {
    r++;
    c = -1;
    for (int j = 0; j < new_tableau.cols - 1; j++) {
      new_tableau.data[r][++c] = tableau->data[i][j];
    }
    new_tableau.data[r][++c] = tableau->data[i][tableau->cols-1];
  }

  free(tableau->rows_h);
  free(tableau->cols_h);
  free(tableau->data);

  tableau->rows = new_tableau.rows;
  tableau->cols = new_tableau.cols;
  tableau->rows_h = new_tableau.rows_h;
  tableau->cols_h = new_tableau.cols_h;
  tableau->data = new_tableau.data;
  tableau->art_vars = new_tableau.art_vars;

  return simplex(tableau, 0);
}

int main(int argc, char *argv[]) {
  FILE *file;
  char line[500], *token;
  problem_t problem;
  tableau_t tableau;

  if (argc != 2) {
    if (argc == 1) {
      printf("Espera-se o nome do arquivo como argumento.\n");
    } else {
      printf("Espera-se apenas um argumento.\n");
    }
    return 1;
  }

  file = fopen(argv[1], "r");

  if (file == NULL) {
    printf("Não foi possível ler o arquivo: %s\n", argv[1]);
    return 1;
  }

  fgets(line, sizeof(line), file);
  token = strtok(line, ",");
  trim(token);

  if (strcmp(token, "min") == 0) {
    problem.action = MIN;
  } else if (strcmp(token, "max") == 0) {
    problem.action = MAX;
  } else {
    printf("Erro de sintaxe.\n");
    return 1;
  }

  token = strtok(NULL, ",");
  problem.obj_func.num_vars = 0;

  while (token != NULL) {
    trim(token);

    if (sscanf(token, "%[^=]=%lf",
      problem.obj_func.vars[problem.obj_func.num_vars].name,
      &problem.obj_func.vars[problem.obj_func.num_vars].coef
    ) == 2) {
      trim(problem.obj_func.vars[problem.obj_func.num_vars].name);
    } else {
      printf("Erro de sintaxe.\n");
      return 1;
    }

    problem.obj_func.num_vars++;
    token = strtok(NULL, ",");
  }

  for (int i = 0; i < problem.obj_func.num_vars; i++) {
    problem.obj_func.vars[i].coef *= -1;
  }
  strcpy(problem.obj_func.op, "=");
  problem.obj_func.value = 0;
  problem.num_consts = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    token = strtok(line, ",");
    problem.consts[problem.num_consts].num_vars = 0;

    while (token != NULL) {
      trim(token);

      if (sscanf(token, "%[^=]=%lf",
        problem.consts[problem.num_consts].vars[problem.consts[problem.num_consts].num_vars].name,
        &problem.consts[problem.num_consts].vars[problem.consts[problem.num_consts].num_vars].coef
      ) == 2) {
        trim(problem.consts[problem.num_consts].vars[problem.consts[problem.num_consts].num_vars].name),
        problem.consts[problem.num_consts].num_vars++;
      } else {
        if (sscanf(token, "%lf",
          &problem.consts[problem.num_consts].value
        ) != 1) {
          if (sscanf(token, "%s",
            problem.consts[problem.num_consts].op
          ) == 1) {
            trim(problem.consts[problem.num_consts].op);
          } else {
            printf("Erro de sintaxe.\n");
            return 1;
          }
        }
      }

      token = strtok(NULL, ",");
    }

    problem.num_consts++;
  }

  fclose(file);

  // begin print the objective function and constraints
  if (problem.action == MAX) {
    printf("Maximize:\nz = ");
  } else {
    printf("Minimize:\nz = ");
  }

  for (int i = 0; i < problem.obj_func.num_vars; i++) {
    if (problem.obj_func.vars[i].coef * -1 > 0) {
      if (i > 0) printf(" + ");
    } else {
      printf(" - ");
    }
    printf("%.*f*%s", 2, fabs(problem.obj_func.vars[i].coef * -1), problem.obj_func.vars[i].name);
  }
  printf("\n\nSujeito a:\n");

  for (int i = 0; i < problem.num_consts; i++) {
    for (int j = 0; j < problem.consts[i].num_vars; j++) {
      if (problem.consts[i].vars[j].coef > 0) {
        if (j > 0) printf(" + ");
      } else {
        printf(" - ");
      }
      printf("%.*f*%s", 2, fabs(problem.consts[i].vars[j].coef), problem.consts[i].vars[j].name);
    }
    printf(" %s %.*f\n", problem.consts[i].op, 2, problem.consts[i].value);
  }
  // end print the objective function and constraints

  tableau.rows = problem.num_consts; // consts
  tableau.cols = problem.obj_func.num_vars + 1; // vars + sol
  tableau.action = problem.action;
  tableau.art_vars = 0;

  for (int i = 0; i < problem.num_consts; i++) {
    if (strcmp(problem.consts[i].op, "=") == 0) {
      tableau.cols += 1; // artificial variable (a)
      tableau.art_vars++;
    } else {
      tableau.cols += 1; // slack and excess variables (s)
      if (strcmp(problem.consts[i].op, ">") == 0 || strcmp(problem.consts[i].op, ">=") == 0) {
        tableau.cols += 1; // artificial variable (a)
        tableau.art_vars++;
      }
    }
  }

  tableau.rows += (tableau.art_vars > 0) ? 2 : 1; // (z or (r + z))
  initializeTableau(&tableau);

  int r = -1, c = -1, s_count = 0, a_count = 0;

  // adds 'r' to the header rows if there are artificial variables
  if (tableau.art_vars > 0) {
    strcpy(tableau.rows_h[++r], "r");
  }
  strcpy(tableau.rows_h[++r], "z"); // adds 'z' to the header rows

  // adds problem variables to the header columns
  for (int i = 0; i < problem.obj_func.num_vars; i++) {
    strcpy(tableau.cols_h[++c], problem.obj_func.vars[i].name);
  }

  for (int i = 0; i < problem.num_consts; i++) {
    // adds the slack and excess variables to the header columns
    if (strcmp(problem.consts[i].op, "=") != 0) {
      sprintf(tableau.cols_h[++c], "_s%d", ++s_count);
    }

    // adds the rest of the base variables
    if (strcmp(problem.consts[i].op, "<") == 0 || strcmp(problem.consts[i].op, "<=") == 0) {
      sprintf(tableau.rows_h[++r], "_s%d", s_count);
    } else {
      sprintf(tableau.rows_h[++r], "_a%d", ++a_count);
    }
  }

  a_count = 0;

  // adds artificial variables to the header columns
  for (int i = 0; tableau.art_vars > 0 && i < problem.num_consts; i++) {
    if (strcmp(problem.consts[i].op, "<") != 0 && strcmp(problem.consts[i].op, "<=") != 0) {
      sprintf(tableau.cols_h[++c], "_a%d", ++a_count);
    }
  }

  strcpy(tableau.cols_h[++c], "sol"); // adds 'sol' to the header columns

  char var_name[VAR_LENGTH];

  // adds r coefficients
  for (int i = 0; i < tableau.art_vars; i++) {
    sprintf(var_name, "_a%d", i + 1);
    c = findColIdx(tableau, var_name);
    tableau.data[0][c] = -1;
  }

  // adds z coefficients
  for (int i = 0; i < problem.obj_func.num_vars; i++) {
    c = findColIdx(tableau, problem.obj_func.vars[i].name);
    tableau.data[(tableau.art_vars > 0) ? 1 : 0][c] = problem.obj_func.vars[i].coef;
  }

  s_count = 0; a_count = 0;

  for (int i = 0; i < problem.num_consts; i++) {
    r = i + ((tableau.art_vars > 0) ? 2 : 1);
    // adds the coefficients
    for (int j = 0; j < problem.consts[i].num_vars; j++) {
      c = findColIdx(tableau, problem.consts[i].vars[j].name);
      if (c == -1) {
        printf("Variável '%s' não está na função objetivo.\n", problem.consts[i].vars[j].name);
        return 1;
      }
      tableau.data[r][c] = problem.consts[i].vars[j].coef;
    }

    sprintf(var_name, "_s%d", ++s_count);
    c = findColIdx(tableau, var_name);

    // adds the slack, excess and artificial coefficients
    if (strcmp(problem.consts[i].op, "<") == 0 || strcmp(problem.consts[i].op, "<=") == 0) {
      tableau.data[r][c] = 1;
    } else {
      tableau.data[r][c] = -1;
      sprintf(var_name, "_a%d", ++a_count);
      c = findColIdx(tableau, var_name);
      tableau.data[r][c] = 1;
    }

    // adds the value of coefficient
    tableau.data[r][tableau.cols-1] = problem.consts[i].value;
  }

  printf("\n\n# Preparação do tableau:\n\n");
  printTableau(tableau);

  double sum;

  // fix the r row
  if (tableau.art_vars > 0) {
    strcpy(tableau.rows_h[0], "-r");

    for (int j = 0; j < tableau.cols; j++) {
      sum = 0;

      for (int i = 2; i < tableau.rows; i++) {
        // check if has artificial var
        if (strncmp(tableau.rows_h[i], "_a", 2) == 0) {
          sum += tableau.data[i][j];
        }
      }

      sum += tableau.data[0][j];
      tableau.data[0][j] = sum * (-1);
    }
  }

  // invert the z row if it is minimization
  if (tableau.action == MIN) {
    r = (tableau.art_vars > 0) ? 1 : 0;
    strcpy(tableau.rows_h[r], "-z");
    for (int j = 0; j < tableau.cols; j++) {
      tableau.data[r][j] *= -1;
    }
  }

  if (tableau.art_vars > 0 || tableau.action == MIN) {
    printf("\n\n# ");
    if (tableau.art_vars && tableau.action == MIN) {
      printf("Corrige a linha 'r' e inverte a linha 'z':\n\n");
    } else if (tableau.art_vars) {
      printf("Corrige a linha 'r':\n\n");
    } else {
      printf("Inverte a linha 'z':\n\n");
    }
    printTableau(tableau);
  }

  int rs = simplex(&tableau, (tableau.art_vars > 0) ? 1 : 0);

  if (rs != 0) {
    printf("\n\nNão foi possível encontrar a solução ótima!\n");
    return 1;
  }

  printf("\n\nSolução ótima:\n");
  printf("z = ");

  double sol = tableau.data[0][tableau.cols-1];
  if (tableau.action == MIN) sol *= -1;
  printf("%.2f\n\n", sol);

  for (int i = 1; i < tableau.rows; i++) {
    printf("%s = %.2f\n", tableau.rows_h[i], tableau.data[i][tableau.cols-1]);
  }

  /* need print all vars of problem?
  for (int i = 0; i < problem.obj_func.num_vars; i++) {
    r = findRowIdx(tableau, problem.obj_func.vars[i].name);
    printf("%s = %.2f\n", problem.obj_func.vars[i].name, tableau.data[r][tableau.cols-1]);
  }
  */

  return 0;
}

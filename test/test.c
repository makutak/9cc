int assert(int expected, int actual, char *code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected, but got %d\n", code, expected, actual);
    exit(1);
  }
}

int main() {
  assert(0, 0, "0");
  return 0;
}

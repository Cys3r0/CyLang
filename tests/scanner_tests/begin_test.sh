echo "Recompiling..."
clang scanner_tests.c ../test_utils.c ../../scanner.c -o scanner_tests.out
echo "Done recompiling..."

for file in *.in; do
    echo "Redirecting file..."
    (./scanner_tests.out < "$file") > "${file%.in}.out"
done
echo "Recompiling..."
clang parser_tests.c ../test_utils.c ../../scanner.c ../../parser.c -o parser_tests.out
echo "Done recompiling..."

for file in *.in; do
    echo "Redirecting file..."
    (./parser_tests.out < "$file") > "${file%.in}.out"
done
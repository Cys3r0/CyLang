for f in *; do
    base="${f%.in}"
    ext="${f##*.}"
    if [ $ext = "in" ]; then
        "../a.out" < "$f" > "${base}.out"
    fi
done


for i in {0..100}
do
    echo ""
    echo ""
    echo "Starting test $i"

  ./multi-lookup 6 0 seviced.txt resolved.txt input/names1*.txt

  sleep .5
done